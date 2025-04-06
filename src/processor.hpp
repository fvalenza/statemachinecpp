#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>      // for close()
#include <arpa/inet.h>   // for sockets
#include <poll.h>

#include "states/states.hpp"

#define PORT 9090
#define BUFFER_SIZE 1024

using MessageFilter = std::function<bool(const std::shared_ptr<MyMessage>&)>;

class Processor {
    MessageQueue<std::shared_ptr<MyMessage>> msgQueue;
    std::atomic<bool> running{true};
    std::atomic<bool> requestedStateChange{false};
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

    // void commandHandler(std::shared_ptr<MyMessage> msg) {
    //     if (currentState_) {
    //         currentState_->CommandHandler(*this, msg);
    //     }
    // }

    bool isErrorMessage(std::shared_ptr<MyMessage> msg) {
        return msg->payload == "error";
    }

    void handleError(std::shared_ptr<MyMessage> msg) {
        std::cout << "[Error] " << msg->payload << std::endl;
    }

    // void handleError(std::shared_ptr<MyMessage> msg) {
    //     std::cout << "[Processor] Error received: " << msg->payload << std::endl;
    //
    //     // Optional: force a transition
    //     currentState_->stop(); // forcefully terminate
    //     changeState(std::make_shared<ErrorState>()); // or terminateState
    // }
    //
    void handleUnexpected(std::shared_ptr<MyMessage> msg) {
        std::cout << "[Unexpected] " << msg->payload << std::endl;
    }

    void commandHandler(std::shared_ptr<MyMessage> msg) {
        if (isErrorMessage(msg)) {
            handleError(msg);
        } else if (currentMessageFilter && currentMessageFilter(msg)) {
            postMessage(msg);
        } else {
            handleUnexpected(msg);
        }
    }

    void setMessageFilter(std::function<bool(std::shared_ptr<MyMessage>)> filter) {
        currentMessageFilter = std::move(filter);
    }

    void clearMessageFilter() {
        currentMessageFilter = nullptr;
    }

    // WARN: This one accepts a single connection and then terminates as soon as the connection closes
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

                if (msg->payload == "special") {
                    handleSpecial(msg);
                } else {
                    commandHandler(msg);
                    // postMessage(msg);
                }
            }

            close(new_socket);  // Close client connection
        }

        close(server_fd);
    }

    void start() {
        std::cout << "[ASTSManager] Starting in Idle State...\n";
        currentState_ = std::make_shared<IdleState>();
        // currentState->execute(*this);
        run();
    }
    void execute() {
        if (currentState_) {
            currentState_->execute(*this);
        }
    }

    void run() {
        std::cout << "[ASTSManager] FSM loop started.\n";

        while (running) {
            // std::lock_guard<std::mutex> lock(stateMutex);
            std::cout << "[ASTSManager] Running loop" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            if (requestedStateChange) {
                std::cout << "[ASTSManager] Change of state requested" << std::endl;
                std::cout << " From " << (currentState_ ? currentState_->name() : "None")
                          << " to " << (newState_ ? newState_->name() : "None") << std::endl;
                currentState_ = std::move(newState_);
                newState_ = nullptr;
                requestedStateChange = false;

                if (currentState_)
                    std::cout << "[ASTSManager] Switched to state: " << currentState_->name() << std::endl;
                // else
                    // std::cout << "[ASTSManager] State change requested to nullptr, terminating FSM loop. Waiting for restart or kill" << std::endl;
                    // break; // nullptr indicates termination
            }

            if (currentState_) {
                if (currentState_->name() == "terminateState") {
                    running = false;
                }
                std::cout << "[ASTSManager] Executing state: " << currentState_->name() << std::endl;
                currentState_->execute(*this);
            }
        }

        std::cout << "[ASTSManager] FSM loop terminated.\n";
    }

    // void run() {
    //     std::cout << "[ASTSManager] Starting FSM loop in state: " << currentState->name() << "\n";
    //     while (running && currentState_) {
    //         if (currentState->name() == "terminateState") {
    //             running = false;
    //         }
    //         std::cout << "[ASTSManager] Starting new state execute : " << currentState->name() << "\n";
    //         currentState->execute(*this);
    //     }
    //     std::cout << "[ASTSManager] FSM loop finished.\n";
    // }

    // std::shared_ptr<MyMessage> waitForMessage() {
    //     return msgQueue.wait_and_pop();
    // }

    std::shared_ptr<MyMessage> waitForMessage() {
        auto msg = msgQueue.wait_and_pop();
        setMessageFilter(nullptr);  // Optional: clear after receipt
        return msg;
    }

    void postMessage(std::shared_ptr<MyMessage> msg) {
        msgQueue.push(std::move(msg));
    }

    void handleSpecial(std::shared_ptr<MyMessage> msg) {
        std::cout << "[SPECIAL Immediate] " << msg->payload << std::endl;
    }


    void changeState(std::shared_ptr<IState> newState) {
        // std::lock_guard<std::mutex> lock(stateMutex);
        if (currentState_) {
            std::cout << "Changing state from " << currentState_->name()
                      << " to " << newState->name() << std::endl;
            currentState_->stop();
        }

        requestedStateChange = true;
        newState_ = std::move(newState);
        std::cout << "State change requested to " << newState_->name() << std::endl;
    }

    std::string currentStateName() {
        std::lock_guard<std::mutex> lock(stateMutex);
        return currentState_ ? currentState_->name() : "None";
    }

    void stop() {
        running = false;
        if (currentState_) currentState_->stop();
        msgQueue.stop();
    }

    void msgQueueStop() {
        msgQueue.stop();
    }
};
