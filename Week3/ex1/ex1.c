#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>  // For sockaddr_in, inet_pton

int main() {
    struct sockaddr_in server_addr;

    // Initialize the sockaddr_in structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);  // Example port number

    // Convert IPv4 address from text to binary and store in sin_addr
    if (inet_pton(AF_INET, "192.168.1.1", &server_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        return -1;
    }

    // Print the initialized values
    printf("sin_family: %d\n", server_addr.sin_family);
    printf("sin_port: %d\n", ntohs(server_addr.sin_port));
    printf("sin_addr: %s\n", inet_ntoa(server_addr.sin_addr));

    return 0;
}