#include <iostream>
#include <thread>
#include "processor.hpp"


int main() {
    std::cout << "Starting Execution" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(5));
    Processor processor;

    std::thread listenerThread([&processor]() { // Equivalent to InstallRxCallback(LAMBDA
        processor.receiveMessage();
    });

    std::cout << "[Main] Calling processor.start()" << std::endl;
    processor.start();

    std::cout << "[Main] Calling processor.stop()" << std::endl;
    processor.stop();
    listenerThread.join();
    return 0;
}
