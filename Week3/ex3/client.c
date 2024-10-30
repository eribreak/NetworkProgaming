#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};
    char server_ip[INET_ADDRSTRLEN];

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return -1;
    }

    // Prompt user to input server IP address
    printf("Enter server IP address: ");
    scanf("%s", server_ip);

    // Convert IP address from text to binary form
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        return -1;
    }

    // Connect to server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        return -1;
    }

    // Send message to server
    char *message = "Hello, Server!";
    write(sock, message, strlen(message));

    // Read response from server
    read(sock, buffer, BUFFER_SIZE);
    printf("Response from server: %s\n", buffer);

    // Close the connection
    close(sock);

    return 0;
}