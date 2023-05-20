#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <server_ip> <port>\n", argv[0]);
        return 1;
    }
    
    char *SERVER = argv[1];
    int PORT = atoi(argv[2]);
    
    srand(time(NULL));
    int sock;
    struct sockaddr_in server;
    char message[BUFFER_SIZE], server_reply[BUFFER_SIZE];
    int field_size, mortars_count;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        printf("Could not create socket\n");
    }

    server.sin_addr.s_addr = inet_addr(SERVER);
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("connect failed. Error\n");
        return 1;
    }

    printf("Connected\n");

    // Read the field size and mortar count from the server
    if (recv(sock, server_reply, BUFFER_SIZE, 0) < 0) {
        printf("recv failed\n");
        return 1;
    }

    // Parse the field size and mortar count
    char *token = strtok(server_reply, ";");
    if (token != NULL) {
        field_size = atoi(token);
        token = strtok(NULL, ";");
        if (token != NULL) {
            mortars_count = atoi(token);
        }
    }

    while (1) {
        int x = rand() % field_size;
        int y = rand() % field_size;
        sprintf(message, "%d;%d", x, y);
        printf("Generated coordinates (x, y): %s\n", message);

        if (send(sock, message, strlen(message), 0) < 0) {
            printf("Send failed\n");
            return 1;
        }

        if (recv(sock, server_reply, BUFFER_SIZE, 0) < 0) {
            printf("recv failed\n");
            return 1;
        }
        char *message_start = server_reply;
        char *newline_position;
        while ((newline_position = strchr(message_start, '\n')) != NULL) {
            *newline_position = '\0';
            printf("Server reply : %s\n", message_start);
            message_start = newline_position + 1;
        }
        sleep(x + y);
    }

    close(sock);
    return 0;
}
