#pragma once

#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <memory>
#include <functional>




#include <cstring>
#include <unistd.h>      // for close()
#include <arpa/inet.h>   // for sockets


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
        std::cout << "[WaitAndPop] Waiting for message..." << std::endl;

        cv.wait(lock, [&] { return !queue.empty() || stop_wait; });

        if(stop_wait){
            std::cout << "Stopping effective of wait_and_pop" << std::endl;
            // stop_wait = false;
            return nullptr;
        }

        T value = std::move(queue.front());
        queue.pop();
        return value;
    }

    void stop() {
        std::lock_guard<std::mutex> lock(mtx);
        std::cout << " Requesting to stop the wait on messageQueue" << std::endl;
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
    void CommandHandler(Processor& processor, std::shared_ptr<MyMessage> msg) override;
    void execute(Processor& processor) override;
};

class ActiveState : public IState {
public:
    // void handleMessage(Processor& context, std::shared_ptr<MyMessage> msg) override;
    std::string name() const override { return "Active"; }
    void CommandHandler(Processor& processor, std::shared_ptr<MyMessage> msg) override;
    void execute(Processor& processor) override;
};

class terminateState : public IState {
public:
    std::string name() const override { return "terminateState"; }
    void CommandHandler(Processor& processor, std::shared_ptr<MyMessage> msg) override {
        // Do nothing
    }
    void execute(Processor& processor) override;
};
