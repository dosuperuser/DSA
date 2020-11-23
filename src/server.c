#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

enum operation_mode {
    THREAD = 0,
    PROC = 1,
};

struct thread_arguments {
    int client_socket_fd;
};

enum operation_mode OPERATION_MODE = PROC;

static void handler_internal(int client_fd)
{
    char buf[4096];
    // Read 1 byte that indicates size of file name.
    char file_name_size;
    int n = recv(client_fd, &file_name_size, 1, 0);

    // Always prepare yourself for worst
    if (n == 0 || n == -1) {
        close(client_fd);
        return;
    }

    // Read file_name_size bytes from socket to form a file name.
    n = recv(client_fd, buf, file_name_size, 0);

    // Always prepare yourself for worst
    if (n == 0 || n == -1) {
        close(client_fd);
        return;
    }

    printf("Creating file %s\n", buf);
    FILE* file = NULL;
    file = fopen(buf, "wb");

    if (file == NULL) {
        close(client_fd);
        perror("Error: ");
        return;
    }

    while ((n = recv(client_fd, buf, 4096, 0)) && n != 0) {
        fwrite(buf, sizeof(char), n, file);
    }

    fclose(file);
    close(client_fd);

    printf("Succes!\n");
}

void* thread_handler(void* thread_args)
{
    pthread_detach(pthread_self());
    int client_fd = ((struct thread_arguments*)thread_args)->client_socket_fd;
    free(thread_args);
    handler_internal(client_fd);
    return NULL;
}

static void handler(int client_socket_fd)
{
    if (OPERATION_MODE == THREAD) {
        pthread_t tid;
        struct thread_arguments* thread_args;
        if ((thread_args = (struct thread_arguments*)malloc(sizeof(struct thread_arguments))) == NULL) {
            perror("Error: ");
            exit(1);
        }
        thread_args->client_socket_fd = client_socket_fd;
        pthread_create(&tid, NULL, thread_handler, (void*)thread_args);
    }
    else if (OPERATION_MODE == PROC) {
        if (fork() == 0) {
            handler_internal(client_socket_fd);
            exit(0);
        }
        else {
            perror("Error: ");
        }
    }
}

static void print_usage()
{
    printf("Server application must be started with following arguments\n");
    printf("./server <PORT> <MODE: THREAD || PROC >\n");
    printf("Example ./server 1337 THREAD\n");
}

int main(int argc, char* argv[])
{

    if (argc != 3) {
        print_usage();
        exit(1);
    }

    char* port = argv[1];
    char* mode = argv[2];

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        perror("Error: ");
        exit(1);
    }

    if (strcmp(mode, "THREAD") == 0) {
        printf("Server will run in thread mode!\n");
        OPERATION_MODE = THREAD;
    }
    else {
        printf("Server will run in new proccess mode!\n");
        OPERATION_MODE = PROC;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(port));

    printf("> Starting server...\n");

    if (bind(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error: ");
        return 1;
    }

    if (listen(socket_fd, 10) < 0) {
        perror("Error: ");
        return 1;
    }

    while (1) {
        printf("\nWaiting for connection!\n");
        int client_socket_fd;
        struct sockaddr_in client_address;
        unsigned int client_lenght;
        client_lenght = sizeof(client_address);

        if ((client_socket_fd = accept(socket_fd, (struct sockaddr*)&client_address, &client_lenght))) {
            printf("Handling client %s\n", inet_ntoa(client_address.sin_addr));
            handler(client_socket_fd);
        }
    }
    close(socket_fd);
    return 0;
}