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
    std::shared_ptr<IState> currentState_;
    std::shared_ptr<IState> newState_;

public:
    Processor() {
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
        if (msg->payload == "stop") { // SI on veut mettre des messages particuliers pour debug etc
            std::cout << "[CommandHandler] Stop command received." << std::endl;
            stop();
        } else { // Sinon on les poste (classiquement) dans la queue pour depilement par un State consommateur
            postMessage(msg);
        }
        // Version "non debug" serait juste le postMessage(msg)
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
        std::cout << "[ASTSManager] Starting in Init State...\n";
        currentState_ = std::make_shared<ATEInitState>();
        run();
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
                }
            }
        }

        std::cout << "[ASTSManager] FSM loop terminated.\n";
        stop();
    }

    std::shared_ptr<MyMessage> waitForMessage() noexcept(false) {
        auto msgFilter = ([this](std::shared_ptr<MyMessage> msg) {
            return true;
        });

        while(1)
        {
            auto msg = msgQueue.wait_and_pop();
            if(msg == nullptr)
            {
                throw waitInterupted("FVA exception");   // cas ou on debloque un state pour pouvoir switcher d'etat
            }
            if (isGlobalMessage(msg)) {
                handleGlobal(msg);
            } else if ( msgFilter(msg)) {
                return msg;
            } else {
                handleUnexpected(msg);
            }
        }
    }
    std::shared_ptr<MyMessage> waitForMessage(std::unordered_set<int> acceptedIDs) noexcept(false) {
        auto msgFilter = ([this](std::unordered_set<int> acceptedIDs, std::shared_ptr<MyMessage> msg) {
            return acceptedIDs.count(std::stoi(msg->payload)) > 0;
        });

        while(1)
        {
            auto msg = msgQueue.wait_and_pop();
            if(msg == nullptr)
            {
                throw waitInterupted("FVA exception");   // cas ou on debloque un state pour pouvoir switcher d'etat
            }
            if (isGlobalMessage(msg)) {
                handleGlobal(msg);
            } else if ( msgFilter(acceptedIDs, msg)) {
                return msg;
            } else {
                handleUnexpected(msg);
            }
        }
    }
    std::shared_ptr<MyMessage> waitForMessage(MessageFilter fn) noexcept(false) {
        while(1)
        {
            auto msg = msgQueue.wait_and_pop();
            if(msg == nullptr)
            {
                throw waitInterupted("FVA exception");   // cas ou on debloque un state pour pouvoir switcher d'etat
            }
            if (isGlobalMessage(msg)) {
                handleGlobal(msg);
            } else if ( fn(msg)) {
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


