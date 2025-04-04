
#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>         // close()
#include <arpa/inet.h>      // socket functions

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 9090

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <message>\n";
        return 1;
    }

    std::string message = argv[1];  // Read message from CLI argument

    int sock = 0;
    struct sockaddr_in serv_addr;

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Socket creation error\n";
        return 1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT);

    // Convert IPv4 address from text to binary
    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address / Address not supported\n";
        return 1;
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection Failed\n";
        return 1;
    }

    // Send message
    send(sock, message.c_str(), message.length(), 0);
    std::cout << "Message sent: " << message << "\n";

    close(sock);
    return 0;
}
