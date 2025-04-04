#include "states.hpp"
#include "processor.hpp"

void IdleState::CommandHandler(Processor& processor, std::shared_ptr<MyMessage> msg) {
    std::cout << "IdleState CommandHandler called." << std::endl;
    if (msg->payload == "special") {
        std::cout << "IdleState CommandHandler: special message." << std::endl;
    } else if (msg->payload == "stop") {
        // stop the execution of the currentstate execute() method and set processor.currentState to nullptr
        stop();
        processor.changeState(std::make_shared<terminateState>());
    } else {
        processor.postMessage(msg); // Need split hpp/cpp files
    }
}
void IdleState::execute(Processor& processor) {
                                std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "Executing state: " << name() << std::endl;
    //performing a bunch a computations here
    if (shouldStop) {
        std::cout << "[IdleState] Interrupted.\n";
        return;
    }
    processor.changeState(std::make_shared<ActiveState>()); // Need split hpp/cpp files

}

void ActiveState::CommandHandler(Processor& processor, std::shared_ptr<MyMessage> msg) {
    std::cout << "ActiveState CommandHandler called with msg: ." << msg->payload << std::endl;
    if (msg->payload == "special") {
        std::cout << "ActiveState CommandHandler: special message." << std::endl;
    } else if (msg->payload == "stop") {
        // stop the execution of the currentstate execute() method and set processor.currentState to nullptr
        stop();
        processor.changeState(std::make_shared<terminateState>());
    } else {
        processor.postMessage(msg); // Need split hpp/cpp files
    }
}
void ActiveState::execute(Processor& processor) {
    // Do some stuff
    for (int i = 0; i < 5; ++i) {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        std::cout << "Executing state: " << name() << "for the " << i << "-th times" << std::endl;
        //performing a bunch a computations here
        if (shouldStop) {
            std::cout << "[ActiveState] Interrupted.\n";
            return;
        }
    }
    // Wait for a message in order to continue execution
    std::cout << "[ActiveState] Waiting for message..." << std::endl;
    auto msg = processor.waitForMessage(); // Need split hpp/cpp files
    if (!msg) {
        std::cout << "[ActiveState] Interrupted during waitForMessage" << std::endl;
        return;
    }
    // Continue execution processing the message
    processor.changeState(std::make_shared<terminateState>()); // Need split hpp/cpp files

}
