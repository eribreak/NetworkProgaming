#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>  // For inet_pton, inet_ntop

int main() {
    char ip_str[INET_ADDRSTRLEN];
    struct in_addr ip_addr;
    char ip_str_converted[INET_ADDRSTRLEN];

    // Prompt the user to input an IP address
    printf("Enter an IP address: ");
    scanf("%s", ip_str);

    // Convert IP address from text to binary form
    if (inet_pton(AF_INET, ip_str, &ip_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        return -1;
    }

    // Convert binary IP address back to text form
    if (inet_ntop(AF_INET, &ip_addr, ip_str_converted, INET_ADDRSTRLEN) == NULL) {
        perror("inet_ntop error");
        return -1;
    }

    // Print the converted IP address
    printf("Converted IP address: %s\n", ip_str_converted);

    return 0;
}