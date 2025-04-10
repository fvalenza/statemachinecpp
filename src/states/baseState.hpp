#pragma once

#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <memory>
#include <functional>
#include <unordered_set>


#include "messageQueue.hpp"


#include <cstring>
#include <unistd.h>      // for close()
#include <arpa/inet.h>   // for sockets

class Processor; // forward declaration

class IState {
protected:
    std::atomic<bool> shouldStop{false};
    std::unordered_set<int> acceptedIDs_; // CGU pourquoi c'est ici ?
public:
    virtual std::string name() const = 0;
    virtual void execute(Processor& processor) noexcept(false) = 0;
    virtual void stop() { shouldStop = true; }
    virtual ~IState() = default;
};




