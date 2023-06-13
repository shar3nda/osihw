#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUF_SIZE 1024

struct client_node {
    struct sockaddr_in clientAddr;
    socklen_t addr_size;
    struct client_node *next;
};

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
    struct sockaddr_in server_addr, client_addr;
    int sockfd;
    char buffer[BUF_SIZE];
    socklen_t addr_size;

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

    // Creating a socket
    sockfd = socket(PF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Error: socket creation failed\n");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip_address);

    // Bind the address struct to the socket
    if (bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        perror("Error: bind failed\n");
        exit(EXIT_FAILURE);
    }

    addr_size = sizeof(client_addr);

    // client list head
    struct client_node *clients = NULL;

    printf("Generating field...\n");
    struct field battlefield;
    battlefield.n = field_size;
    battlefield.mortars_a = malloc(mortars_count * sizeof(struct mortar));
    battlefield.mortars_b = malloc(mortars_count * sizeof(struct mortar));
    for (size_t i = 0; i < mortars_count; i++) {
        battlefield.mortars_a[i].x = rand() % field_size;
        battlefield.mortars_a[i].y = rand() % field_size;
        battlefield.mortars_a[i].destroyed = 0;
        battlefield.mortars_b[i].x = rand() % field_size;
        battlefield.mortars_b[i].y = rand() % field_size;
        battlefield.mortars_b[i].destroyed = 0;
    }
    printf("Field generated\n");

    while (1) {
        int recv_len = recvfrom(sockfd, buffer, BUF_SIZE, 0, (struct sockaddr *) &client_addr, &addr_size);
        if (recv_len > 0) {
            buffer[recv_len] = 0;
            int client_pid, x, y;
            sscanf(buffer, "%d;%d;%d", &client_pid, &x, &y);

            printf("Client with pid %d shot at %d,%d\n", client_pid, x, y);

            struct client_node *node = clients;
            while (node != NULL) {
                if (node->clientAddr.sin_addr.s_addr == client_addr.sin_addr.s_addr &&
                    node->clientAddr.sin_port == client_addr.sin_port) {
                    break;
                }
                node = node->next;
            }
            if (node == NULL) {
                node = malloc(sizeof(struct client_node));
                node->clientAddr = client_addr;
                node->addr_size = addr_size;
                node->next = clients;
                clients = node;
            }

            struct client_node *tmp = clients;
            while (tmp != NULL) {
                sendto(sockfd, buffer, recv_len, 0, (struct sockaddr *) &(tmp->clientAddr), tmp->addr_size);
                tmp = tmp->next;
            }
        }
    }

    // free client list
    while (clients != NULL) {
        struct client_node *tmp = clients;
        clients = clients->next;
        free(tmp);
    }

    return 0;
}
