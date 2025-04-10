
#pragma once

#include <memory>
#include <iostream>
#include <typeindex>

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
    static void handle(IState* from, IState* to) {
        std::cout << "[Generic Transition] From " << from->name() << " to " << to->name() << std::endl;
        // Here put a big "switch" sur la paire ??? DEGUEU......
        // Or use a map to store the transitions
        // Or use a function pointer to call the right transition

        // example with dynamic casts
        // if (dynamic_cast<IdleState*>(from) && dynamic_cast<ActiveState*>(to)) transitionIdleToActive(from, to);
        // else if (dynamic_cast<ActiveState*>(from) && dynamic_cast<IdleState*>(to)) transitionActiveToIdle(from, to);
        // etc....
    }
};




void transitionIdleToActive(std::shared_ptr<IdleState> from, std::shared_ptr<ActiveState> to) {
    std::cout << "[Specialized] Idle → Active" << std::endl;
    // your logic here
}

void transitionActiveToTerminate(std::shared_ptr<ActiveState> from, std::shared_ptr<terminateState> to) {
    std::cout << "[Specialized] Active → Terminate" << std::endl;
    // your logic here
}


class TransitionHandlerMapFunctions {
    using Key = std::pair<std::type_index, std::type_index>;
    using HandlerFn = std::function<void(std::shared_ptr<IState>, std::shared_ptr<IState>)>;

    void handle(std::shared_ptr<IState> from, std::shared_ptr<IState> to) {
        Key key{typeid(*from), typeid(*to)};
        auto it = registry_.find(key);
        if (it != registry_.end()) {
            it->second(from, to); // Call the actuel mapped function
        } else {
            std::cout << "[Generic Transition] From " << from->name() << " to " << to->name() << std::endl;
        }
    }

    template<typename From, typename To>
    void registerTransition(std::function<void(std::shared_ptr<From>, std::shared_ptr<To>)> fn) {
        Key key{typeid(From), typeid(To)};
        registry_[key] = [fn](std::shared_ptr<IState> from, std::shared_ptr<IState> to) {
            fn(std::dynamic_pointer_cast<From>(from), std::dynamic_pointer_cast<To>(to));
        };
    }

    void registerAllTransitions() {
        registerTransition<IdleState, ActiveState>(transitionIdleToActive);
        registerTransition<ActiveState, terminateState>(transitionActiveToTerminate);
    }

private:
    std::unordered_map<Key, HandlerFn> registry_;
};

