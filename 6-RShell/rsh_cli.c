#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <fcntl.h>
#include "dshlib.h"
#include "rshlib.h"

int start_client(char *server_ip, int port) {
    int cli_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (cli_socket < 0) {
        perror("socket");
        return ERR_RDSH_CLIENT;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, server_ip, &addr.sin_addr);

    if (connect(cli_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("connect");
        close(cli_socket);
        return ERR_RDSH_CLIENT;
    }

    return cli_socket;
}

int client_cleanup(int cli_socket, char *cmd_buff, char *rsp_buff, int rc) {
    if (cli_socket > 0) {
        close(cli_socket);
    }

    free(cmd_buff);
    free(rsp_buff);

    return rc;
}

int exec_remote_cmd_loop(char *address, int port) {
    char *cmd_buff = malloc(RDSH_COMM_BUFF_SZ);
    char *rsp_buff = malloc(RDSH_COMM_BUFF_SZ);
    int cli_socket = start_client(address, port);

    if (cli_socket < 0) {
        return client_cleanup(cli_socket, cmd_buff, rsp_buff, ERR_RDSH_CLIENT);
    }

    while (1) {
        printf("dsh4> ");
        if (fgets(cmd_buff, RDSH_COMM_BUFF_SZ, stdin) == NULL) {
            break;
        }

        send(cli_socket, cmd_buff, strlen(cmd_buff) + 1, 0);

        int recv_size;
        while ((recv_size = recv(cli_socket, rsp_buff, RDSH_COMM_BUFF_SZ, 0)) > 0) {
            if (rsp_buff[recv_size - 1] == RDSH_EOF_CHAR) {
                rsp_buff[recv_size - 1] = '\0';
                printf("%s", rsp_buff);
                break;
            }
            printf("%.*s", recv_size, rsp_buff);
        }
    }

    return client_cleanup(cli_socket, cmd_buff, rsp_buff, OK);
}
