#include "states.hpp"
#include "processor.hpp"

void IdleState::CommandHandler(Processor& processor, std::shared_ptr<MyMessage> msg) {
    std::cout << "[IdleState] CommandHandler called." << std::endl;
    if (msg->payload == "special") {
        std::cout << "[IdleState] CommandHandler: special message." << std::endl;
    } else if (msg->payload == "stop") {
        // stop the execution of the currentstate execute() method and set processor.currentState to nullptr
        // stop();
        processor.changeState(std::make_shared<terminateState>());
    } else {
        processor.postMessage(msg); // Need split hpp/cpp files
    }
    std::cout << "[IdleState] CommandHandler terminated." << std::endl;
}
void IdleState::execute(Processor& processor) {
    std::cout << "[IdleState] Start of execute.\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));
    //performing a bunch a computations here
    if (shouldStop) {
        std::cout << "[IdleState] Interrupted.\n";
        return;
    }
    std::cout << "[IdleState] Changing State" << std::endl;
    processor.changeState(std::make_shared<ActiveState>()); // Need split hpp/cpp files
    std::cout << "[IdleState] execute terminated." << std::endl;

}

void ActiveState::CommandHandler(Processor& processor, std::shared_ptr<MyMessage> msg) {
    std::cout << "[ActiveState] CommandHandler called with msg: " << msg->payload << std::endl;
    if (msg->payload == "special") {
        std::cout << "[ActiveState] CommandHandler: special message." << std::endl;
    } else if (msg->payload == "stop") {
        // stop the execution of the currentstate execute() method and set processor.currentState to nullptr
        // stop();
        processor.msgQueueStop(); // Stop the message queue
        processor.changeState(std::make_shared<terminateState>());
    } else {
        processor.postMessage(msg); // Need split hpp/cpp files
    }
    std::cout << "[ActiveState] CommandHandler terminated." << std::endl;
}
void ActiveState::execute(Processor& processor) {
    std::cout << "[ActiveState] Start of execute.\n";
    // Do some stuff
    for (int i = 0; i < 1; ++i) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "Executing state: " << name() << "for the " << i << "-th times" << std::endl;
        //performing a bunch a computations here
        if (shouldStop) {
            std::cout << "[ActiveState] Interrupted.\n";
            return;
        }
    }
    // Wait for a message in order to continue execution
    std::cout << "[ActiveState] Waiting for message..." << std::endl;
    //TODO A message received too early still unlocks this wait

    // processor.setMessageFilter([](std::shared_ptr<MyMessage> msg) {
    //     return msg->payload == "4";
    // });

    acceptedIDs_ = {4, 42, 77, 99};
    processor.setMessageFilter([this](const std::shared_ptr<MyMessage>& msg) {
        return acceptedIDs_.count(std::stoi(msg->payload)) > 0;
    });

    auto msg = processor.waitForMessage(); // Need split hpp/cpp files
    if (!msg) {
        std::cout << "[ActiveState] Interrupted during waitForMessage" << std::endl;
        return;
    }
    std::cout << "[ActiveState] Received message : " << msg->payload << std::endl;
    // Continue execution processing the message
    processor.changeState(std::make_shared<terminateState>()); // Need split hpp/cpp files
    // processor.changeState(std::make_shared<IdleState>()); // Need split hpp/cpp files
    std::cout << "[ActiveState] execute terminated." << std::endl;

}

void terminateState::execute(Processor& processor)  {
    std::cout << "[TerminateState] Start of execute" << std::endl;
    // Do nothing. and normally it should finish the execution in the main
    std::cout << "[TerminateState] processor value" << processor.currentStateName() << std::endl;
    // processor.changeState(nullptr); // NOTE I dont know why but this line created segfault, instead of doing it here in the processor::run i check if the newState to execute is terminate and stop the loop instead
    std::cout << "[TerminateState] execute terminated" << std::endl;
}
