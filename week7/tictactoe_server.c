#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <errno.h>

#define PORT 8080
#define MAX_CLIENT 2
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
int cur_player = 1;
int move_count = 0;
char buffer[BUFFER_SIZE];

void notify_turn(int client_sock);
int check_valid_move(int row, int col);
int check_winner();
void print_board();

int main() {
    int listen_fd, client1_sock, client2_sock;
    struct sockaddr_in serv_addr;
    socklen_t addrlen = sizeof(serv_addr);
    int row, col;

    // Create socket
    if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Define server address
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    // Bind the socket
    if (bind(listen_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connection
    if (listen(listen_fd, MAX_CLIENT) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Listening on port %d...\n", PORT);

    // Prepare for player registation message
    memset(buffer, 0, BUFFER_SIZE);
    buffer[0] = GAME_REGISTER; 

    // Client 1 connect
    if ((client1_sock = accept(listen_fd, (struct sockaddr *)&serv_addr, &addrlen)) < 0) {
        perror("accept failed");
        exit(EXIT_FAILURE);
    }
    buffer[1] = GAME_PLAYER_1; // Assign Player 1
    send(client1_sock, buffer, BUFFER_SIZE, 0);
    printf("Player 1 connected, waiting for player 2...\n");

    // Client 2 connect
    if ((client2_sock = accept(listen_fd, (struct sockaddr *)&serv_addr, &addrlen)) < 0) {
        perror("accept failed");
        exit(EXIT_FAILURE);
    }
    buffer[1] = GAME_PLAYER_2;  // Assign Player 2
    send(client2_sock, buffer, BUFFER_SIZE, 0);
    printf("Player 2 connected, start game!\n");

    memset(buffer, 0, BUFFER_SIZE);
    while (1) {
        // Turn notification
        if (cur_player == 1) {
            notify_turn(client1_sock);
            if (recv(client1_sock, buffer, BUFFER_SIZE, 0) <= 0) {
                printf("Player 1 disconnected!\n");
                break;
            };
        } else if (cur_player == 2) {
            notify_turn(client2_sock);
            if (recv(client2_sock, buffer, BUFFER_SIZE, 0) <= 0) {
                printf("Player 2 disconnected!\n");
                break;
            };
        }

        // Check player move
        if (buffer[0] == MOVE) {
            row = buffer[1];
            col = buffer[2];
            int is_valid_move = check_valid_move(row, col);

            // If it is a valid move, accept player move
            if (is_valid_move) {
                game_board[row][col] = cur_player;
                move_count++;

                // Send STATE_UPDATE message to two players
                buffer[0] = STATE_UPDATE;
                buffer[3] = cur_player;
                send(client1_sock, buffer, BUFFER_SIZE, 0);
                send(client2_sock, buffer, BUFFER_SIZE, 0);
                printf("Player %d move:\n", cur_player);
                print_board();
            } else {
                // If it is not a valid move, send MOVE_INVALID message
                buffer[0] = MOVE_INVALID;
                if (cur_player == 1) {
                    send(client1_sock, buffer, BUFFER_SIZE, 0);
                } else if (cur_player == 2) {
                    send(client2_sock, buffer, BUFFER_SIZE, 0);
                }
                continue;
            }
        } else {
            continue;
        }

        // Check board state
        int winner = check_winner();
        if (winner > 0 || move_count == 9) {
            buffer[0] = RESULT;
            if (winner == 1) {
                buffer[1] = GAME_PLAYER_1;
                printf("Player 1 Win!\n");
            }
            else if (winner == 2) {
                buffer[1] = GAME_PLAYER_2;
                printf("Player 2 Win!\n");
            }
            else {
                buffer[1] = GAME_DRAW;
                printf("Draw!\n");
            }
            send(client1_sock, buffer, BUFFER_SIZE, 0);
            send(client2_sock, buffer, BUFFER_SIZE, 0);
            break;
        }

        // Switch player
        if (cur_player == 1) cur_player = 2;
        else cur_player = 1;
    }

    close(client1_sock);
    close(client2_sock);
    close(listen_fd);
    return 0;
}

void notify_turn(int client_sock) {
    buffer[0] = TURN_NOTIFICATION;
    send(client_sock, buffer, BUFFER_SIZE, 0);
}

int check_valid_move(int row, int col) {
    if (row < 0 || row > 2) return 0;
    if (col < 0 || col > 2) return 0;
    if (game_board[row][col] != 0) return 0;
    return 1;
}

int check_winner() {
    for (int i = 0; i < 3; i++) {
        if (game_board[i][0] != 0 && game_board[i][0] == game_board[i][1] && game_board[i][0] == game_board[i][2]) return game_board[i][0];
        if (game_board[0][i] != 0 && game_board[0][i] == game_board[1][i] && game_board[0][i] == game_board[2][i]) return game_board[0][i];
    }

    if (game_board[0][0] != 0 && game_board[0][0] == game_board[1][1] && game_board[0][0] == game_board[2][2]) return game_board[0][0];
    if (game_board[2][0] != 0 && game_board[2][0] == game_board[1][1] && game_board[2][0] == game_board[0][2]) return game_board[2][0];

    return -1;
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