#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <errno.h>

#define PORT 8080
#define SERVER_ADDR "127.0.0.1"
#define GAME_REGISTER 0x01
#define MOVE 0x02
#define STATE_UPDATE 0x03
#define RESULT 0x04
#define TURN_NOTIFICATION 0x05
#define GAME_PLAYER_1 0x11
#define GAME_PLAYER_2 0x12
#define GAME_DRAW 0x13
#define MOVE_INVALID 0x21
#define BUFFER_SIZE 1024
#define BOARD_SIZE 3

int game_board[BOARD_SIZE][BOARD_SIZE] = {0};
char buffer[BUFFER_SIZE];

void print_board();

int main() {
    int sock_fd;
    struct sockaddr_in serv_addr;
    int row, col, player_move;
    char player[1];

    // Create a socket
    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set up the server address structure
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = inet_addr(SERVER_ADDR);

    // Connect to server
    if (connect(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect failed");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }
    printf("Join game successfully! Initializing game board...\n");
    print_board();

    while(1) {
        if (recv(sock_fd, buffer, BUFFER_SIZE, 0) <= 0) {
            printf("Server disconnected!\n");
            break;
        };

        // Register player
        if (buffer[0] == GAME_REGISTER) {
            player[0] = buffer[1];
        }

        // Make move
        else if (buffer[0] == TURN_NOTIFICATION) {
            int is_valid_move = 0;
            printf("Your turn! Enter your move:\n");
            printf("Row (0 - 2): ");
            scanf("%d", &row);
            printf("Column (0 - 2): ");
            scanf("%d", &col);

            buffer[0] = MOVE;
            buffer[1] = row;
            buffer[2] = col;
            send(sock_fd, buffer, BUFFER_SIZE, 0);
        }

        // Receive MOVE_INVALID message
        else if (buffer[0] == MOVE_INVALID) {
            printf("Invalid move, please try again!\n\n");
            continue;
        }

        // Update game board state
        else if (buffer[0] == STATE_UPDATE) {
            row = buffer[1];
            col = buffer[2];
            player_move = buffer[3];
            game_board[row][col] = player_move;
            print_board();
        }

        // Receive result
        else if (buffer[0] == RESULT) {
            if (buffer[1] == GAME_DRAW) printf("Draw!\n");
            else if (buffer[1] == player[0]) printf("You Win!\n");
            else if (buffer[1] != player[0]) printf("You Lose!\n");
            break;
        }
    }

    close(sock_fd);
    return 0;
}

void print_board() {
    printf("-----------------\n");
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (game_board[i][j] == 0) printf("- ");
            if (game_board[i][j] == 1) printf("O ");
            if (game_board[i][j] == 2) printf("X ");
        }
        printf("\n");
    }
    printf("-----------------\n\n");
}