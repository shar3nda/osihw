#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#define BUF_SIZE 1024

int field_size;
int mortars_count;
int unique_id;

struct thread_data {
    int sockfd;
    struct sockaddr_in serverAddr;
};

void *send_thread(void *arg) {
    struct thread_data *data = (struct thread_data *) arg;
    char buffer[BUF_SIZE];
    while (1) {
        int x = rand() % field_size;
        int y = rand() % field_size;
        sprintf(buffer, "%d;%d;%d", unique_id, x, y);
        printf("Generated coordinates (x, y): %d, %d\n", x, y);
        sendto(data->sockfd, buffer, strlen(buffer), 0, (struct sockaddr *) &data->serverAddr,
               sizeof(data->serverAddr));
        sleep(x + y);
    }
    pthread_exit(NULL);
}

void *receive_thread(void *arg) {
    struct thread_data *data = (struct thread_data *) arg;
    char buffer[BUF_SIZE];
    while (1) {
        int addr_size = sizeof(data->serverAddr);
        int recvLen = recvfrom(data->sockfd, buffer, BUF_SIZE, 0, NULL, NULL);
        if (recvLen > 0) {
            buffer[recvLen] = 0;
            int client_id, x, y;
            sscanf(buffer, "%d;%d;%d", &client_id, &x, &y);
            if (client_id != unique_id) {
                printf("Received a hit at (x, y): %d, %d\n", x, y);
            }
        }
    }
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    struct sockaddr_in serverAddr;
    int sockfd;

    unique_id = getpid();

    if (argc < 5) {
        printf("Usage: %s <field_size> <mortars_count> <ip_address> <port>\n", argv[0]);
        return 1;
    }

    field_size = atoi(argv[1]);
    mortars_count = atoi(argv[2]);
    char *ip_address = argv[3];
    int port = atoi(argv[4]);

    sockfd = socket(PF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Error: socket creation failed\n");
        exit(EXIT_FAILURE);
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr(ip_address);

    pthread_t tid[2];
    struct thread_data data;
    data.sockfd = sockfd;
    data.serverAddr = serverAddr;

    pthread_create(&tid[0], NULL, send_thread, &data);
    pthread_create(&tid[1], NULL, receive_thread, &data);

    pthread_join(tid[0], NULL);
    pthread_join(tid[1], NULL);

    return 0;
}
