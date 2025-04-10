#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>      // for close()
#include <arpa/inet.h>   // for sockets
#include <poll.h>

#include "states/states.hpp"
#include "states/transitionHandler.hpp"

#define PORT 9090
#define BUFFER_SIZE 1024

using MessageFilter = std::function<bool(const std::shared_ptr<MyMessage>&)>;

// https://rollbar.com/blog/cpp-custom-exceptions/
class waitInterupted : public std::exception {
    private:
    std::string message;

    public:
    waitInterupted(std::string msg = "") : message(msg) {}
    const char * what () const noexcept override {
        return message.c_str();
    }
};

class Processor {
    MessageQueue<std::shared_ptr<MyMessage>> msgQueue;
    std::atomic<bool> running{true};
    std::mutex stateMutex;
    std::shared_ptr<IState> currentState_;
    std::shared_ptr<IState> newState_;
    std::function<bool(std::shared_ptr<MyMessage>)> currentMessageFilter = nullptr;

public:
    // Processor() : currentState_(std::make_shared<IdleState>()) {}
    Processor() {
        // currentState_ = std::make_shared<IdleState>(); // initial state
        currentState_ = nullptr;
    }


    bool isGlobalMessage(std::shared_ptr<MyMessage> msg) {
        return false;
    }

    void handleGlobal(std::shared_ptr<MyMessage> msg) {
        std::cout << "[Global] " << msg->payload << std::endl;
    }

    void handleUnexpected(std::shared_ptr<MyMessage> msg) {
        std::cout << "[Unexpected] " << msg->payload << std::endl;
    }

    void commandHandler(std::shared_ptr<MyMessage> msg) {
        if (msg->payload == "stop") {
            std::cout << "[CommandHandler] Stop command received." << std::endl;
            stop();
        } else {
            postMessage(msg);
        }
    }

    void setMessageFilter(std::function<bool(std::shared_ptr<MyMessage>)> filter) {
        currentMessageFilter = std::move(filter);
    }

    void clearMessageFilter() {
        currentMessageFilter = nullptr;
    }


    void receiveMessage() {
        int server_fd, new_socket;
        struct sockaddr_in address;
        socklen_t addrlen = sizeof(address);
        char buffer[BUFFER_SIZE] = {0};

        // Create socket file descriptor
        if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
            perror("Socket failed");
            exit(EXIT_FAILURE);
        }

        // Bind socket to IP/PORT
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(PORT);

        if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
            perror("Bind failed");
            exit(EXIT_FAILURE);
        }

        // Start listening
        if (listen(server_fd, 3) < 0) {
            perror("Listen failed");
            exit(EXIT_FAILURE);
        }

        std::cout << "TCP Server listening on port " << PORT << "...\n";

        while (running) {
            // Accept a new connection
            new_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen);
            if (new_socket < 0) {
                perror("Accept failed");
                continue;
            }

            // Read message
            ssize_t valread = read(new_socket, buffer, BUFFER_SIZE);
            if (valread > 0) {
                buffer[valread] = '\0';
                std::string receivedMessage(buffer);

                // Convert to MyMessage
                // auto msg = std::make_shared<MyMessage>(parseMessage(receivedMessage));
                std::cout << "Received message: " << receivedMessage << std::endl;
                auto msg = std::make_shared<MyMessage>( receivedMessage);

                commandHandler(msg);
            }

            close(new_socket);  // Close client connection
        }

        close(server_fd);
    }

    void start() {
        std::cout << "[ASTSManager] Starting in Idle State...\n";
        currentState_ = std::make_shared<IdleState>();
        run();
    }

    template<typename FromState, typename ToState>
    void handleTransition(std::shared_ptr<FromState> from, std::shared_ptr<ToState> to) {
        std::cout << "[ASTSManager] Handling transition from " << from->name() << " to " << to->name() << std::endl;
        // The next line only calls the generic version of the transition handler because it resolves into shared_ptr<IState>
        TransitionHandler<FromState, ToState>::handle(from, to);
        // The next line has error: no viable conversion from shared_ptr<Istate> to shared_ptr<IdleState>
        TransitionHandler<IdleState, ActiveState>::handle(to, from);
    }

    void run() {
        std::cout << "[ASTSManager] FSM loop started.\n";

        while (running) {
            std::cout << "[ASTSManager] Running loop" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            if(newState_ != nullptr)
            {
                std::cout << "[ASTSManager] Change of state requested" << std::endl;
                std::cout << " From " << (currentState_ ? currentState_->name() : "None")
                            << " to " << newState_->name() << std::endl;
                // This next line is not working due to template argument deduction and polymorphism, it will resolves into handleTransition<shared_ptr<IState>, shared_ptr<IState>>
			    handleTransition(currentState_, newState_);
                // The next line does not compile, i was adding it to test the template specialization
                handleTransition<IdleState, ActiveState>(currentState_, newState_);
                currentState_ = std::move(newState_);
                newState_ = nullptr;
                std::cout << "[ASTSManager] Switched to state: " << currentState_->name() << std::endl;
            }


            if (currentState_) {
                std::cout << "[ASTSManager] Executing state: " << currentState_->name() << std::endl;
                try {
                    currentState_->execute(*this);
                } catch( const waitInterupted& e) {
                    std::cout << "[ASTSManager] Exception caught: " << e.what() << std::endl;
                    // Handle the exception, e.g., log it or change state
                } catch (const std::exception& e) {
                    std::cerr << "[ASTSManager] Exception: " << e.what() << std::endl;
                    // Handle other exceptions
                } catch (...) {
                    std::cerr << "[ASTSManager] Unknown exception occurred." << std::endl;

                }
            }
        }

        std::cout << "[ASTSManager] FSM loop terminated.\n";
        stop();
    }

    std::shared_ptr<MyMessage> waitForMessage() noexcept(false) {
        while(1)
        {
            auto msg = msgQueue.wait_and_pop();
            if(msg == nullptr)
            {
                throw waitInterupted("FVA exception");   // cas ou on debloque un state pour pouvoir switcher d'etat
            }
            if (isGlobalMessage(msg)) {
                handleGlobal(msg);
            } else if (currentMessageFilter && currentMessageFilter(msg)) {
                setMessageFilter(nullptr);
                return msg;
            } else {
                handleUnexpected(msg);
            }
        }
    }

    void postMessage(std::shared_ptr<MyMessage> msg) {
        msgQueue.push(std::move(msg));
    }


    void changeState(std::shared_ptr<IState> newState) {
        if (currentState_) {
            std::cout << "Changing state from " << currentState_->name()
                      << " to " << newState->name() << std::endl;
            currentState_->stop();
        }

        newState_ = std::move(newState);
        std::cout << "State change requested to " << newState_->name() << std::endl;
    }

    std::string currentStateName() {
        return currentState_ ? currentState_->name() : "None";
    }

    void stop() {
        std::cout << "[ASTSManager] Stopping statemachine" << std::endl;
        running = false;
        if (currentState_) currentState_->stop();
        msgQueue.stop();
    }

    void msgQueueStop() {
        msgQueue.stop();

    }
};


    // WARN: This one accepts a single connection and then terminates as soon as the connection closes but do not block when finishing like the other one
    // void receiveMessage() {
    //     int server_fd, client_fd;
    //     struct sockaddr_in address;
    //     socklen_t addrlen = sizeof(address);
    //     char buffer[BUFFER_SIZE] = {0};
    //
    //     // Create socket
    //     if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    //         perror("Socket failed");
    //         exit(EXIT_FAILURE);
    //     }
    //
    //     int opt = 1;
    //     setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
    //
    //     // Bind
    //     address.sin_family = AF_INET;
    //     address.sin_addr.s_addr = INADDR_ANY;
    //     address.sin_port = htons(PORT);
    //
    //     if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
    //         perror("Bind failed");
    //         exit(EXIT_FAILURE);
    //     }
    //
    //     // Listen
    //     if (listen(server_fd, 1) < 0) {
    //         perror("Listen failed");
    //         exit(EXIT_FAILURE);
    //     }
    //
    //     std::cout << "Waiting for client to connect on port " << PORT << "...\n";
    //
    //     // Accept a single client
    //     client_fd = accept(server_fd, (struct sockaddr*)&address, &addrlen);
    //     if (client_fd < 0) {
    //         perror("Accept failed");
    //         close(server_fd);
    //         return;
    //     }
    //
    //     std::cout << "Client connected.\n";
    //
    //     struct pollfd pfd;
    //     pfd.fd = client_fd;
    //     pfd.events = POLLIN;
    //
    //     while (running) {
    //         int ret = poll(&pfd, 1, 1000); // timeout = 1000ms
    //
    //         if (ret < 0) {
    //             perror("poll failed");
    //             break;
    //         }
    //
    //         if (ret == 0) {
    //             // Timeout: no data, loop again
    //             continue;
    //         }
    //
    //         if (pfd.revents & POLLIN) {
    //             ssize_t valread = read(client_fd, buffer, BUFFER_SIZE - 1);
    //             if (valread <= 0) {
    //                 std::cout << "Client disconnected or read error.\n";
    //                 break;
    //             }
    //
    //             buffer[valread] = '\0';
    //             std::string receivedMessage(buffer);
    //             std::cout << "Received message: " << receivedMessage << std::endl;
    //
    //             auto msg = std::make_shared<MyMessage>(receivedMessage);
    //             if (msg->payload == "special") {
    //                 handleSpecial(msg);
    //             } else {
    //                 commandHandler(msg);
    //             }
    //         }
    //     }
    //
    //     close(client_fd);
    //     close(server_fd);
    //     std::cout << "Receiver thread shutting down.\n";
    // }
