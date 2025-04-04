
#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>      // for close()
#include <arpa/inet.h>   // for sockets

#include "states/states.hpp"

#define PORT 9090
#define BUFFER_SIZE 1024

class Processor {
    MessageQueue<std::shared_ptr<MyMessage>> msgQueue;
    std::atomic<bool> running{true};
    std::mutex stateMutex;
    std::shared_ptr<IState> currentState;

public:
    Processor() {
        // currentState = std::make_shared<IdleState>(); // initial state
        currentState = nullptr;
    }

    void commandHandler(std::shared_ptr<MyMessage> msg) {
        if (currentState) {
            currentState->CommandHandler(*this, msg);
        }
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

        while (true) {
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
    }

    void start() {
        std::cout << "[ASTSManager] Starting in Idle State...\n";
        currentState = std::make_shared<IdleState>();
        currentState->execute(*this);
    }

    void execute() {
        if (currentState) {
            currentState->execute(*this);
        }
    }

    std::shared_ptr<MyMessage> waitForMessage() {
        return msgQueue.wait_and_pop();
    }
    void postMessage(std::shared_ptr<MyMessage> msg) {
        msgQueue.push(std::move(msg));
    }

    void handleSpecial(std::shared_ptr<MyMessage> msg) {
        std::cout << "[SPECIAL Immediate] " << msg->payload << std::endl;
    }

    void changeState(std::shared_ptr<IState> newState) {
        std::cout << "Changing state from " << currentState->name() << " to " << newState->name() << std::endl;
        if (currentState) {
            std::cout << "Stopping current state: " << currentState->name() << std::endl;
            currentState->stop();
        }

        currentState = std::move(newState);
        std::cout << "Switched to state: " << currentState->name() << std::endl;
        execute();
    }

    std::string currentStateName() {
        std::lock_guard<std::mutex> lock(stateMutex);
        return currentState->name();
    }

    void stop() {
        running = false;
        msgQueue.stop(); // Unblock any state waiting for messages
    }
};
