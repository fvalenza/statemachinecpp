#include <iostream>
#include <string>
#include <cstring>
#include <queue>
#include <mutex>
#include <condition_variable>
// #include <memory>
// #include <functional>
// #include <atomic>
// #include <thread>
// #include <utility>

#include "myMessage.hpp"

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
