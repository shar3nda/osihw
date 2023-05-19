#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_CLIENTS 2
#define BUFFER_SIZE 1024
#define PORT 8888

int client_sockets[MAX_CLIENTS];
int client_count = 0;

struct mortar {
    int x;
    int y;
    int destroyed;
};

struct field {
    int n;
    struct mortar *mortars_a;
    struct mortar *mortars_b;
};

int main(int argc, char *argv[]) {
    int server_sock, client_sock, client_len, read_size, i;
    struct sockaddr_in server, client;
    char client_message[BUFFER_SIZE];

    if (argc < 3) {
        printf("Usage: %s <field_size> <mortars_count>\n", argv[0]);
        return 1;
    }
    int field_size = atoi(argv[1]);
    int mortars_count = atoi(argv[2]);
    if (field_size < 1 || mortars_count < 1) {
        printf("Field size and mortars count must be positive integers\n");
        return 1;
    }

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1) {
        printf("Could not create socket\n");
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    if (bind(server_sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("bind failed. Error\n");
        return 1;
    }

    listen(server_sock, MAX_CLIENTS);

    printf("Generating field...\n");
    struct field battlefield;
    battlefield.n = field_size;
    battlefield.mortars_a = malloc(mortars_count * sizeof(struct mortar));
    battlefield.mortars_b = malloc(mortars_count * sizeof(struct mortar));
    for (i = 0; i < mortars_count; i++) {
        battlefield.mortars_a[i].x = rand() % field_size;
        battlefield.mortars_a[i].y = rand() % field_size;
        battlefield.mortars_a[i].destroyed = 0;
        battlefield.mortars_b[i].x = rand() % field_size;
        battlefield.mortars_b[i].y = rand() % field_size;
        battlefield.mortars_b[i].destroyed = 0;
    }
    printf("Field generated\n");

    printf("Waiting for incoming connections...\n");
    client_len = sizeof(struct sockaddr_in);

    while (1) {
        client_sock = accept(server_sock, (struct sockaddr *)&client, (socklen_t *)&client_len);

        if (client_sock < 0) {
            perror("accept failed\n");
            continue;
        }

        if (client_count >= MAX_CLIENTS) {
            printf("Maximum clients connected. Connection rejected\n");
            close(client_sock);
            continue;
        }

        client_sockets[client_count++] = client_sock;
        printf("Connection accepted\n");

        while ((read_size = recv(client_sock, client_message, BUFFER_SIZE, 0)) > 0) {
            printf("Received coordinates: %s\n", client_message);

            // Send the message to other clients
            for (i = 0; i < client_count; i++) {
                if (client_sockets[i] != client_sock) {
                    write(client_sockets[i], client_message, read_size);
                }
            }
        }

        if (read_size == 0) {
            printf("Client disconnected\n");
            fflush(stdout);
            // Remove the client from the client_sockets array
            for (i = 0; i < client_count; i++) {
                if (client_sockets[i] == client_sock) {
                    client_sockets[i] = client_sockets[client_count - 1];
                    client_count--;
                    break;
                }
            }
        } else if (read_size == -1) {
            perror("recv failed\n");
        }

        close(client_sock);
    }

    return 0;
}
