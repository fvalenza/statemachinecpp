#include "idleState.hpp"
#include "states.hpp"
#include "processor.hpp"


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
