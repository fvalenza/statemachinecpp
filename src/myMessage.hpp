#include <string>

class MyMessage {
public:
    std::string payload;

    MyMessage(std::string p) : payload(std::move(p)) {}
};

