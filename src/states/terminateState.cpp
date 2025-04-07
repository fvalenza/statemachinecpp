
#include "terminateState.hpp"
#include "processor.hpp"

void terminateState::execute(Processor& processor)  {
    std::cout << "[TerminateState] Start of execute" << std::endl;
    // Do nothing. and normally it should finish the execution in the main
    std::cout << "[TerminateState] processor value" << processor.currentStateName() << std::endl;
    // processor.changeState(nullptr); // NOTE I dont know why but this line created segfault, instead of doing it here in the processor::run i check if the newState to execute is terminate and stop the loop instead
    std::cout << "[TerminateState] execute terminated" << std::endl;
}
