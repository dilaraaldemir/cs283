#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <fcntl.h>
#include "dshlib.h"
#include "rshlib.h"

int boot_server(char *ifaces, int port) {
    int svr_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (svr_socket < 0) {
        perror("socket");
        return ERR_RDSH_COMMUNICATION;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ifaces, &addr.sin_addr);

    int enable = 1;
    setsockopt(svr_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

    if (bind(svr_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(svr_socket);
        return ERR_RDSH_COMMUNICATION;
    }

    if (listen(svr_socket, 20) < 0) {
        perror("listen");
        close(svr_socket);
        return ERR_RDSH_COMMUNICATION;
    }

    return svr_socket;
}

int stop_server(int svr_socket) {
    return close(svr_socket);
}

int process_cli_requests(int svr_socket) {
    int cli_socket;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    while (1) {
        cli_socket = accept(svr_socket, (struct sockaddr *)&client_addr, &client_len);
        if (cli_socket < 0) {
            perror("accept");
            return ERR_RDSH_COMMUNICATION;
        }

        int rc = exec_client_requests(cli_socket);
        if (rc == OK_EXIT) {
            close(cli_socket);
            break;
        }

        close(cli_socket);
    }

    return OK;
}

int exec_client_requests(int cli_socket) {
    char buffer[RDSH_COMM_BUFF_SZ];
    int recv_size;
    command_list_t clist;

    while (1) {
        recv_size = recv(cli_socket, buffer, RDSH_COMM_BUFF_SZ, 0);
        if (recv_size <= 0) {
            return ERR_RDSH_COMMUNICATION;
        }

        buffer[recv_size - 1] = '\0'; // Null-terminate the command

        int rc = build_cmd_list(buffer, &clist);
        if (rc != OK) {
            send_message_string(cli_socket, "Error: Failed to parse command\n");
            continue;
        }

        rc = rsh_execute_pipeline(cli_socket, &clist);
        if (rc == OK_EXIT) {
            return OK_EXIT;
        }

        send_message_eof(cli_socket);
    }

    return OK;
}

int rsh_execute_pipeline(int cli_sock, command_list_t *clist) {
    int pipes[clist->num - 1][2];
    pid_t pids[clist->num];

    for (int i = 0; i < clist->num - 1; i++) {
        pipe(pipes[i]);
    }

    for (int i = 0; i < clist->num; i++) {
        pids[i] = fork();
        if (pids[i] == 0) {
	    if (i == 0) {
                dup2(cli_sock, STDIN_FILENO);
            }
	    if (i == clist->num - 1) {
                dup2(cli_sock, STDOUT_FILENO);
            }
            if (i > 0) {
                dup2(pipes[i - 1][0], STDIN_FILENO);
            }
            if (i < clist->num - 1) {
                dup2(pipes[i][1], STDOUT_FILENO);
            }

            for (int j = 0; j < clist->num - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            execvp(clist->commands[i].argv[0], clist->commands[i].argv);
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < clist->num - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    for (int i = 0; i < clist->num; i++) {
        waitpid(pids[i], NULL, 0);
    }

    return OK;
}

int send_message_eof(int cli_socket) {
    int send_len = (int)sizeof(RDSH_EOF_CHAR);
    int sent_len = send(cli_socket, &RDSH_EOF_CHAR, send_len, 0);

    if (sent_len != send_len) {
        return ERR_RDSH_COMMUNICATION;
    }
    return OK;
}

int send_message_string(int cli_socket, char *buff) {
    int send_len = strlen(buff) + 1;
    int sent_len = send(cli_socket, buff, send_len, 0);

    if (sent_len != send_len) {
        return ERR_RDSH_COMMUNICATION;
    }

    return send_message_eof(cli_socket);
}

int start_server(char *ifaces, int port, int is_threaded) {
    int svr_socket = boot_server(ifaces, port);
    if (svr_socket < 0) {
        return svr_socket;
    }
    if (is_threaded) {
        printf("Threaded mode is not implemented.\n");
    }
    int rc = process_cli_requests(svr_socket);
    stop_server(svr_socket);

    return rc;
}
