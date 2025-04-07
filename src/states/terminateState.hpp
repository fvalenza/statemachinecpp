#pragma once

#include <iostream>
#include <string>
#include "baseState.hpp"


class terminateState : public IState {
public:
    std::string name() const override { return "terminateState"; }
    void CommandHandler(Processor& processor, std::shared_ptr<MyMessage> msg) override {
        // Do nothing
    }
    void execute(Processor& processor) override;
};
;
