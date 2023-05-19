#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER "127.0.0.1"
#define BUFFER_SIZE 1024
#define PORT 8888

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in server;
    char message[BUFFER_SIZE], server_reply[BUFFER_SIZE];

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

    while (1) {
        printf("Enter coordinates (x, y): ");
        scanf("%s", message);

        if (send(sock, message, strlen(message), 0) < 0) {
            printf("Send failed\n");
            return 1;
        }

        if (recv(sock, server_reply, BUFFER_SIZE, 0) < 0) {
            printf("recv failed\n");
            return 1;
        }

        printf("Server reply : %s\n", server_reply);
    }

    close(sock);
    return 0;
}
