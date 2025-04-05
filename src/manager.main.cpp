#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <memory>
#include <functional>



#include "processor.hpp"


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
    std::thread listenerThread([&processor]() {
        processor.receiveMessage();
    });
    std::cout << "[Main] Calling processor.start()" << std::endl;
    processor.start();

    // std::thread listenerThread(tcpListener, std::ref(processor));

    // std::thread listenerThread(std::bind(&Processor::receiveMessage));

    // // Keep running
    // while (true) {
    //     std::this_thread::sleep_for(std::chrono::seconds(5));
    //     std::cout << "[Main] Current State: " << processor.currentStateName() << std::endl;
    // }

    std::cout << "[Main] Calling processor.stop()" << std::endl;
    processor.stop();
    listenerThread.join();
    return 0;
}
