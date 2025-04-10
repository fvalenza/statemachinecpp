#include "transitionHandler.hpp"
#include "states/activeState.hpp"
#include "states/terminateState.hpp"


template<>
void TransitionHandlerTemplated<IdleState, ActiveState>::handle(
    IdleState* from,
    ActiveState* to)
{
    std::cout << "[Specialized Transition] Idle → Active" << std::endl;
}

// Specialized transition: ErrorState -> IdleState
template<>
void TransitionHandlerTemplated<ActiveState, terminateState>::handle(
    ActiveState* from,
    terminateState* to)
{
    std::cout << "[Specialized Transition] Active -> Terminate" << std::endl;
}



