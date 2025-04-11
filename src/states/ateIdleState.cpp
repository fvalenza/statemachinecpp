#include "states.hpp"
#include "processor.hpp"


void ATEIdleState::execute(Processor& processor) {
    std::cout << "[ATEIdleState] Start of execute.\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));
    //performing a bunch a computations here
    if (shouldStop) {
        std::cout << "[ATEIdleState] Interrupted.\n";
        return;
    }
    std::cout << "[ATEIdleState] Changing State" << std::endl;
    processor.changeState(std::make_shared<ActiveState>()); // Need split hpp/cpp files
    std::cout << "[ATEIdleState] execute terminated." << std::endl;

}
