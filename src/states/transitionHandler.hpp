
#pragma once

#include <memory>
#include <iostream>

#include "states.hpp"  // Your IdleState, ErrorState, etc.

template<typename FromState, typename ToState>
struct TransitionHandlerTemplated {
    static void handle(FromState* from, ToState* to) {
        std::cout << "[Generic Templated Transition] From " << from->name() << " to " << to->name() << std::endl;

    }
};


// Rmq: can't make TransitionHandlerTemplated to work, so by default we may need this raw solution....
// Rmq2: seems kind of logical that will need something at runtime as long as templates can't be resolved at compiletime due to polymorphism
struct TransitionHandler {
    // TODO a remplacer par shared_ptr<IState> et dynamic_ptr_cast si on fait cette solution
    static void handle(IState* from, IState* to) {
        std::cout << "[Dynamic cast Transition] From " << from->name() << " to " << to->name() << std::endl;
        // Here put a big "switch" sur la paire ??? DEGUEU......
        // Or use a map to store the transitions
        // Or use a function pointer to call the right transition

        // example with dynamic casts
        // if (dynamic_cast<IdleState*>(from) && dynamic_cast<ActiveState*>(to)) transitionIdleToActive(from, to);
        // else if (dynamic_cast<ActiveState*>(from) && dynamic_cast<IdleState*>(to)) transitionActiveToIdle(from, to);
        // etc....
    }
};
