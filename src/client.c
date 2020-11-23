
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

static void print_command()
{
    printf("\nEnter file name to be uploaded to server\n");
    printf("$>");
}

static void print_usage()
{
    printf("Client application must be started with following arguments\n");
    printf("./client <IP_OF_SERVER> <PORT>\n");
}

static int connect_to_server(char* ip, char* port, int socket_fd)
{
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(ip);
    servaddr.sin_port = htons(atoi(port));
    for (int i = 1; i <= 10; ++i) {
        if (connect(socket_fd, (struct sockaddr*)&servaddr, sizeof(servaddr))) {
            printf("Connection failed retrying, attempt %d!\n", i);
            sleep(2);
        }
        else
            return 0;
    }
    return -1;
}

int main(int argc, char* argv[])
{

    if (argc != 3) {
        print_usage();
        exit(1);
    }

    char* ip = argv[1];
    char* port = argv[2];

    FILE* file = NULL;

    char buffer[4096];
    while (1) {

        int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (socket_fd == -1) {
            perror("Error: ");
            exit(1);
        }

        int rc = connect_to_server(ip, port, socket_fd);
        if (rc == -1) {
            printf("Connection to %s:%s failed terminating application!\n", ip, port);
            close(socket_fd);
            exit(1);
        }

        print_command();

        // Optimization name points to buffer + 1 address
        // From buffer perspective name will be stored buffer+1
        // This is used to reuse resources since buffer[0] will containt file name size
        char* name = buffer + 1;

        // Handle CTRL + D and CTRL+Z in safe manner
        if (scanf("%s", name) == EOF) {
            clearerr(stdin);
            continue;
        }

        buffer[0] = strlen(name);

        // Opening file using 'rb' option since we do not actually need to modify file content.
        file = fopen(name, "rb");
        if (file == NULL) {
            printf("Could not open file %s", buffer);
            close(socket_fd);
            continue;
        }

        printf("File: %s, Size of message: %d\n", buffer, (int)strlen(buffer));

        if (send(socket_fd, buffer, buffer[0] + 1, 0) < 0) {
            perror("Error: ");
            close(socket_fd);
            continue;
        }

        while (!feof(file)) {
            // Reuse same buffer since it's max Buffer size 4096 as we need it
            int n = fread(buffer, 1, 4096, file);
            if (send(socket_fd, buffer, n, 0) < 0) {
                perror("Error: ");
                close(socket_fd);
                continue;
            }
        }
        fclose(file);
        close(socket_fd);
    }

    return 0;
}
