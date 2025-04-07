#pragma once

#include <iostream>
#include <string>
#include "baseState.hpp"


class ActiveState : public IState {
public:
    // void handleMessage(Processor& context, std::shared_ptr<MyMessage> msg) override;
    std::string name() const override { return "Active"; }
    void CommandHandler(Processor& processor, std::shared_ptr<MyMessage> msg) override;
    void execute(Processor& processor) override;
};
