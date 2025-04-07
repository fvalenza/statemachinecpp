#include "idleState.hpp"
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
