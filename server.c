#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <pthread.h>

#define MAX_CLIENTS 2
#define BUFFER_SIZE 1024

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

void *client_handler(void *socket_desc) {
    int sock = *(int *)socket_desc;
    int read_size;
    char client_message[BUFFER_SIZE];

    // Handle client communication
    while ((read_size = recv(sock, client_message, BUFFER_SIZE, 0)) > 0) {
        char *token = strtok(client_message, ";");
        int x = -1, y = -1;
        if (token != NULL) {
            x = atoi(token);
            token = strtok(NULL, ";");
            if (token != NULL) {
                y = atoi(token);
            }
        }

        // Send the message to the other client
        for (int i = 0; i < client_count; i++) {
            if (client_sockets[i] != sock) {
                char received_shot[BUFFER_SIZE];
                snprintf(received_shot, BUFFER_SIZE, "Shot received at: %d;%d\n", x, y);
                write(client_sockets[i], received_shot, strlen(received_shot));
                break;
            }
        }

        // Send acknowledgement with the result of the shot
        char ack_msg[BUFFER_SIZE];
        snprintf(ack_msg, BUFFER_SIZE, "Shot fired at: %d;%d\n", x, y);
        write(sock, ack_msg, strlen(ack_msg));
    }

    if (read_size == 0) {
        printf("Client disconnected\n");
        fflush(stdout);
        // Remove the client from the client_sockets array
        for (int i = 0; i < client_count; i++) {
            if (client_sockets[i] == sock) {
                client_sockets[i] = client_sockets[client_count - 1];
                client_count--;
                break;
            }
        }
    } else if (read_size == -1) {
        perror("recv failed\n");
    }

    close(sock);
    free(socket_desc);

    return 0;
}

int main(int argc, char *argv[]) {
    int server_sock, client_sock, client_len, read_size, i;
    struct sockaddr_in server, client;
    memset(&client, 0, sizeof(client));
    client_len = sizeof(client);
    char client_message[BUFFER_SIZE];

    if (argc < 5) {
        printf("Usage: %s <field_size> <mortars_count> <ip_address> <port>\n", argv[0]);
        return 1;
    }

    int field_size = atoi(argv[1]);
    int mortars_count = atoi(argv[2]);
    char *ip_address = argv[3];
    int port = atoi(argv[4]);

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
    server.sin_addr.s_addr = inet_addr(ip_address);
    server.sin_port = htons(port);

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

        char field_and_mortar_info[BUFFER_SIZE];
        sprintf(field_and_mortar_info, "%d;%d", field_size, mortars_count);

        // send information to the connected client
        write(client_sock, field_and_mortar_info, strlen(field_and_mortar_info));

        pthread_t thread_id;
        int *new_sock = malloc(sizeof(int));
        *new_sock = client_sock;
        if (pthread_create(&thread_id, NULL, client_handler, (void *)new_sock) < 0) {
            perror("Could not create thread\n");
            return 1;
        }

        printf("Handler assigned\n");
    }

    return 0;
}
