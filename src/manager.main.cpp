#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <memory>
#include <functional>



class MyMessage {
public:
    std::string payload;

    MyMessage(std::string p) : payload(std::move(p)) {}
};

template<typename T>
class MessageQueue {
    std::queue<T> queue;
    std::mutex mtx;
    std::condition_variable cv;
    bool stop_wait = false;

public:
    void push(T value) {
        {
            std::lock_guard<std::mutex> lock(mtx);
            queue.push(std::move(value));
        }
        cv.notify_one();
    }

    T wait_and_pop() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [&] { return !queue.empty() || stop_wait; });

        if(stop_wait){
            stop_wait = false;
            return nullptr;
        }

        T value = std::move(queue.front());
        queue.pop();
        return value;
    }

    void stop() {
        std::lock_guard<std::mutex> lock(mtx);
        stop_wait = true;
        cv.notify_all();
    }

    void reset() {
        std::lock_guard<std::mutex> lock(mtx);
        stop_wait = false;
    }

};

class Processor; // forward declaration

class IState {
protected:
    std::atomic<bool> shouldStop{false};
public:
    virtual std::string name() const = 0;
    virtual void CommandHandler(Processor& processor, std::shared_ptr<MyMessage> msg) = 0;
    virtual void execute(Processor& processor) = 0;
    virtual void stop() { shouldStop = true; }
    virtual ~IState() = default;
};


class IdleState : public IState {
public:
    // void handleMessage(Processor& context, std::shared_ptr<MyMessage> msg) override;
    std::string name() const override { return "Idle"; }
    void CommandHandler(Processor& processor, std::shared_ptr<MyMessage> msg) override {
        std::cout << "IdleState CommandHandler called." << std::endl;
        if (msg->payload == "special") {
            std::cout << "IdleState CommandHandler: special message." << std::endl;
        } else if (msg->payload == "stop") {
            // stop the execution of the currentstate execute() method and set processor.currentState to nullptr
            stop();
            // processor.changeState(std::make_shared<terminateState>());
        } else {
            // processor.postMessage(msg); // Need split hpp/cpp files
        }
    }
    void execute(Processor& processor) override {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        std::cout << "Executing state: " << name() << std::endl;
        //performing a bunch a computations here
        if (shouldStop) {
            std::cout << "[IdleState] Interrupted.\n";
            return;
        }
        // processor.changeState(std::make_shared<ActiveState>()); // Need split hpp/cpp files

    }
};

class ActiveState : public IState {
public:
    // void handleMessage(Processor& context, std::shared_ptr<MyMessage> msg) override;
    std::string name() const override { return "Active"; }
    void CommandHandler(Processor& processor, std::shared_ptr<MyMessage> msg) override {
        std::cout << "ActiveState CommandHandler called with msg: ." << msg->payload << std::endl;
        if (msg->payload == "special") {
            std::cout << "ActiveState CommandHandler: special message." << std::endl;
        } else {
            // processor.postMessage(msg); // Need split hpp/cpp files
        }
    }
    void execute(Processor& processor) override {
        // Do some stuff
        // Wait for a message in order to continue execution
        // auto msg = processor.waitForMessage(); // Need split hpp/cpp files
        // if (!msg) {
        //     std::cout << "[ActiveState] Interrupted during waitForMessage" << std::endl;
        //     return;
        // }
        // Continue execution processing the message
        // processor.changeState(std::make_shared<terminateState>()); // Need split hpp/cpp files

    }
};
class terminateState : public IState {
public:
    std::string name() const override { return "Active"; }
    void CommandHandler(Processor& processor, std::shared_ptr<MyMessage> msg) override {
        // Do nothing
    }
    void execute(Processor& processor) override {
        // Do nothing. and normally it should finish the execution in the main
    }
};

#include <cstring>
#include <unistd.h>      // for close()
#include <arpa/inet.h>   // for sockets

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



// void IdleState::handleMessage(Processor& context, std::shared_ptr<MyMessage> msg) {
//     std::cout << "[Idle] Received: " << msg->payload << std::endl;
//
//     if (msg->payload == "1") {
//         std::cout << "Already in Idle state." << std::endl;
//         context.changeState(std::make_shared<ActiveState>());
//     } else if (msg->payload == "2") {
//         std::cout << "Transitioning to Active..." << std::endl;
//         context.changeState(std::make_shared<ActiveState>());
//     } else {
//         std::cout << "Unknown message in Idle state: " << msg->payload << std::endl;
//     }
//
// }
//
// void ActiveState::handleMessage(Processor& context, std::shared_ptr<MyMessage> msg) {
//     std::cout << "[Active] Processing: " << msg->payload << std::endl;
//
//     if (msg->payload == "go_idle") {
//         context.changeState(std::make_shared<IdleState>());
//     }
// }



int main() {
    std::cout << "Starting Execution" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(5));
    Processor processor;
    processor.start();

    // std::thread listenerThread(tcpListener, std::ref(processor));

    // std::thread listenerThread(std::bind(&Processor::receiveMessage));
    std::thread listenerThread([&processor]() {
        processor.receiveMessage();
    });

    // // Keep running
    // while (true) {
    //     std::this_thread::sleep_for(std::chrono::seconds(5));
    //     std::cout << "[Main] Current State: " << processor.currentStateName() << std::endl;
    // }

    processor.stop();
    listenerThread.join();
    return 0;
}
