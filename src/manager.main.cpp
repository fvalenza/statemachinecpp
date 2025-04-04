#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <memory>



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
        cv.wait(lock, [&] { return !queue.empty(); });
        T value = std::move(queue.front());
        queue.pop();
        return value;
    }
};

class Processor; // forward declaration

class IState {
public:
    virtual void handle(Processor& context, std::shared_ptr<MyMessage> msg) = 0;
    virtual std::string name() const = 0;
    virtual ~IState() = default;
};


class IdleState : public IState {
public:
    void handle(Processor& context, std::shared_ptr<MyMessage> msg) override;
    std::string name() const override { return "Idle"; }
};

class ActiveState : public IState {
public:
    void handle(Processor& context, std::shared_ptr<MyMessage> msg) override;
    std::string name() const override { return "Active"; }
};

class Processor {
    MessageQueue<std::shared_ptr<MyMessage>> msgQueue;
    std::atomic<bool> running{true};
    std::mutex stateMutex;
    std::shared_ptr<IState> currentState;

public:
    Processor() {
        currentState = std::make_shared<IdleState>(); // initial state
    }

    void start() {
        std::thread([this] {
            while (running) {
                auto msg = msgQueue.wait_and_pop();
                if (!msg) continue;

                std::lock_guard<std::mutex> lock(stateMutex);
                currentState->handle(*this, msg);
            }
        }).detach();
    }

    void postMessage(std::shared_ptr<MyMessage> msg) {
        msgQueue.push(std::move(msg));
    }

    void handleSpecial(std::shared_ptr<MyMessage> msg) {
        std::cout << "[SPECIAL Immediate] " << msg->payload << std::endl;
    }

    void changeState(std::shared_ptr<IState> newState) {
        std::cout << "Changing state from " << currentState->name() << " to " << newState->name() << std::endl;
        std::lock_guard<std::mutex> lock(stateMutex);
        std::cout << "State change: " << currentState->name() << " -> " << newState->name() << std::endl;
        currentState = std::move(newState);
    }

    std::string currentStateName() {
        std::lock_guard<std::mutex> lock(stateMutex);
        return currentState->name();
    }

    void stop() {
        running = false;
        msgQueue.push(nullptr);
    }
};



void IdleState::handle(Processor& context, std::shared_ptr<MyMessage> msg) {
    std::cout << "[Idle] Received: " << msg->payload << std::endl;

    if (msg->payload == "1") {
        std::cout << "Already in Idle state." << std::endl;
        context.changeState(std::make_shared<ActiveState>());
    } else if (msg->payload == "2") {
        std::cout << "Transitioning to Active..." << std::endl;
        context.changeState(std::make_shared<ActiveState>());
    } else {
        std::cout << "Unknown message in Idle state: " << msg->payload << std::endl;
    }
    
}

void ActiveState::handle(Processor& context, std::shared_ptr<MyMessage> msg) {
    std::cout << "[Active] Processing: " << msg->payload << std::endl;

    if (msg->payload == "go_idle") {
        context.changeState(std::make_shared<IdleState>());
    }
}

#include <cstring>
#include <unistd.h>      // for close()
#include <arpa/inet.h>   // for sockets

#define PORT 9090
#define BUFFER_SIZE 1024

void tcpListener(Processor& processor) {
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
                processor.handleSpecial(msg);
            } else {
                processor.postMessage(msg);
            }
        }

        close(new_socket);  // Close client connection
    }
}

int main() {
    Processor processor;
    processor.start();

    std::thread listenerThread(tcpListener, std::ref(processor));

    // Keep running
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        std::cout << "[Main] Current State: " << processor.currentStateName() << std::endl;
    }

    processor.stop();
    listenerThread.join();
    return 0;
}
