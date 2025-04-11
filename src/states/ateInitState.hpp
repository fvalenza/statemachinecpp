#pragma once

#include <iostream>
#include <string>
#include "baseState.hpp"

class ATEInitState : public IState {
public:
    // void handleMessage(Processor& context, std::shared_ptr<MyMessage> msg) override;
    std::string name() const override { return "Idle"; }
    void execute(Processor& processor) override;
    void transitionT1ToIdle(Processor& processor);
};
