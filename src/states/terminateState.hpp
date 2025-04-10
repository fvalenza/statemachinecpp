#pragma once

#include <iostream>
#include <string>
#include "baseState.hpp"


class terminateState : public IState {
public:
    std::string name() const override { return "terminateState"; }
    void execute(Processor& processor) override;
};
;
