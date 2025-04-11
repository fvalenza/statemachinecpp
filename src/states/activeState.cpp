
#include "activeState.hpp"
#include "processor.hpp"

void ActiveState::handleTransitionT1(Processor& processor) {
    //actions

    //
    processor.changeState(std::make_shared<terminateState>()); // Need split hpp/cpp files
}

void ActiveState::execute(Processor& processor) {
    std::cout << "[ActiveState] Start of execute.\n";
    // Do some stuff
    for (int i = 0; i < 5; ++i) {
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
    // With new implementation of waitForMessage a message received too early is not bypassed by the reception thread as it is put in the message queue

    auto msgFilter = ([this](std::shared_ptr<MyMessage> msg) {
        std::unordered_set<int> acceptedIDs = {4, 42, 77, 99};
        return acceptedIDs.count(std::stoi(msg->payload)) > 0;
    });

    // auto msg = processor.waitForMessage(); // Accept everything
    auto msg = processor.waitForMessage({4,5}); // Accept ids from a set
    // auto msg = processor.waitForMessage(msgFilter); // Accept from a custom filter function

    std::cout << "[ActiveState] Received message : " << msg->payload << std::endl;
    // Continue execution processing the message
    handleTransitionT1(processor);
    return;
    // processor.changeState(std::make_shared<IdleState>()); // Need split hpp/cpp files
    std::cout << "[ActiveState] execute terminated." << std::endl;

}
