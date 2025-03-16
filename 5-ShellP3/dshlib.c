#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "dshlib.h"

int alloc_cmd_buff(cmd_buff_t *cmd_buff) {
    cmd_buff->argc = 0;
    cmd_buff->_cmd_buffer = NULL;
    cmd_buff->in_redir_type = REDIR_NONE;
    cmd_buff->in_redir_file = NULL;
    cmd_buff->out_redir_type = REDIR_NONE;
    cmd_buff->out_redir_file = NULL;

    for (int i = 0; i < CMD_ARGV_MAX; i++) {
        cmd_buff->argv[i] = NULL;
    }

    return OK;
}

int free_cmd_buff(cmd_buff_t *cmd_buff) {
    if (cmd_buff->_cmd_buffer) {
        free(cmd_buff->_cmd_buffer);
        cmd_buff->_cmd_buffer = NULL;
    }

    for (int i = 0; i < cmd_buff->argc; i++) {
        cmd_buff->argv[i] = NULL;
    }

    cmd_buff->argc = 0;
    return OK;
}

int clear_cmd_buff(cmd_buff_t *cmd_buff) {
    cmd_buff->argc = 0;
    if (cmd_buff->_cmd_buffer) {
        cmd_buff->_cmd_buffer[0] = '\0';
    }

    for (int i = 0; i < CMD_ARGV_MAX; i++) {
        cmd_buff->argv[i] = NULL;
    }

    return OK;
}

int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd_buff) {
    size_t cmd_len = strlen(cmd_line);

    cmd_buff->in_redir_type = REDIR_NONE;
    cmd_buff->in_redir_file = NULL;
    cmd_buff->out_redir_type = REDIR_NONE;
    cmd_buff->out_redir_file = NULL;

    cmd_buff->_cmd_buffer = (char *)malloc(cmd_len + 1);
    if (!cmd_buff->_cmd_buffer) {
        return ERR_MEMORY;
    }

    strcpy(cmd_buff->_cmd_buffer, cmd_line);

    char *in_redir = strchr(cmd_buff->_cmd_buffer, REDIR_IN_CHAR);
    char *out_redir = strchr(cmd_buff->_cmd_buffer, REDIR_OUT_CHAR);
    char *append_redir = strstr(cmd_buff->_cmd_buffer, ">>");

    if (append_redir) {
        *append_redir = '\0';
        append_redir += 2;

        while (*append_redir && isspace(*append_redir)) {
            append_redir++;
        }

        if (*append_redir) {
            cmd_buff->out_redir_type = REDIR_APPEND;
            cmd_buff->out_redir_file = append_redir;

            char *end = append_redir + strlen(append_redir) - 1;
            while (end > append_redir && isspace(*end)) {
                *end = '\0';
                end--;
            }
        } else {
            printf(CMD_ERR_REDIR);
            return ERR_CMD_ARGS_BAD;
        }

        out_redir = NULL;
    }

    if (out_redir && !append_redir) {
        *out_redir = '\0';
        out_redir++;

        while (*out_redir && isspace(*out_redir)) {
            out_redir++;
        }

        if (*out_redir) {
            cmd_buff->out_redir_type = REDIR_OUT;
            cmd_buff->out_redir_file = out_redir;

            char *end = out_redir + strlen(out_redir) - 1;
            while (end > out_redir && isspace(*end)) {
                *end = '\0';
                end--;
            }
        } else {
            printf(CMD_ERR_REDIR);
            return ERR_CMD_ARGS_BAD;
        }
    }

    if (in_redir) {
        *in_redir = '\0';
        in_redir++;

        while (*in_redir && isspace(*in_redir)) {
            in_redir++;
        }

        if (*in_redir) {
            cmd_buff->in_redir_type = REDIR_IN;
            cmd_buff->in_redir_file = in_redir;

            char *end = in_redir + strlen(in_redir) - 1;
            while (end > in_redir && isspace(*end)) {
                *end = '\0';
                end--;
            }
        } else {
            printf(CMD_ERR_REDIR);
            return ERR_CMD_ARGS_BAD;
        }
    }

    char *token = strtok(cmd_buff->_cmd_buffer, " \t");
    while (token != NULL && cmd_buff->argc < CMD_ARGV_MAX - 1) {
        cmd_buff->argv[cmd_buff->argc++] = token;
        token = strtok(NULL, " \t");
    }

    cmd_buff->argv[cmd_buff->argc] = NULL;

    return OK;
}

int close_cmd_buff(cmd_buff_t *cmd_buff) {
    return free_cmd_buff(cmd_buff);
}

int build_cmd_list(char *cmd_line, command_list_t *clist) {
    clist->num = 0;

    if (!cmd_line || strlen(cmd_line) == 0) {
        printf("%s", CMD_WARN_NO_CMD);
        return WARN_NO_CMDS;
    }

    char *cmd_copy = strdup(cmd_line);
    if (!cmd_copy) {
        return ERR_MEMORY;
    }

    char *cmd_ptr = cmd_copy;
    char *pipe_pos;

    while (cmd_ptr && *cmd_ptr) {
        if (clist->num >= CMD_MAX) {
            printf(CMD_ERR_PIPE_LIMIT, CMD_MAX);
            free(cmd_copy);
            return ERR_TOO_MANY_COMMANDS;
        }

        pipe_pos = strchr(cmd_ptr, PIPE_CHAR);
        if (pipe_pos) {
            *pipe_pos = '\0';
        }

        while (*cmd_ptr && isspace(*cmd_ptr)) {
            cmd_ptr++;
        }

        char *end = cmd_ptr + strlen(cmd_ptr) - 1;
        while (end > cmd_ptr && isspace(*end)) {
            *end = '\0';
            end--;
        }

        if (*cmd_ptr) {
            alloc_cmd_buff(&clist->commands[clist->num]);

            int rc = build_cmd_buff(cmd_ptr, &clist->commands[clist->num]);
            if (rc != OK) {
                free(cmd_copy);
                return rc;
            }

            clist->num++;
        }

        if (pipe_pos) {
            cmd_ptr = pipe_pos + 1;
        } else {
            cmd_ptr = NULL;
        }
    }

    free(cmd_copy);

    if (clist->num == 0) {
        printf("%s", CMD_WARN_NO_CMD);
        return WARN_NO_CMDS;
    }

    return OK;
}

int free_cmd_list(command_list_t *cmd_lst) {
    for (int i = 0; i < cmd_lst->num; i++) {
        free_cmd_buff(&cmd_lst->commands[i]);
    }

    cmd_lst->num = 0;
    return OK;
}

Built_In_Cmds match_command(const char *input) {
    if (!input || !*input) {
        return BI_NOT_BI;
    }

    if (strcmp(input, EXIT_CMD) == 0) {
        return BI_CMD_EXIT;
    }

    if (strcmp(input, "dragon") == 0) {
        return BI_CMD_DRAGON;
    }

    if (strcmp(input, "cd") == 0) {
        return BI_CMD_CD;
    }

    return BI_NOT_BI;
}

Built_In_Cmds exec_built_in_cmd(cmd_buff_t *cmd) {
    if (!cmd || cmd->argc == 0) {
        return BI_NOT_BI;
    }

    Built_In_Cmds bi_cmd = match_command(cmd->argv[0]);

    switch (bi_cmd) {
        case BI_CMD_EXIT:
            printf("exiting...\n");
            return BI_CMD_EXIT;

        case BI_CMD_CD:
            if (cmd->argc < 2) {
                char *home = getenv("HOME");
                if (home) {
                    chdir(home);
                }
            } else {
                if (chdir(cmd->argv[1]) != 0) {
                    perror("cd");
                }
            }
            return BI_EXECUTED;

        default:
            return BI_NOT_BI;
    }
}

int exec_cmd(cmd_buff_t *cmd) {
    if (!cmd || cmd->argc == 0) {
        return ERR_CMD_ARGS_BAD;
    }

    Built_In_Cmds bi_result = exec_built_in_cmd(cmd);
    if (bi_result == BI_CMD_EXIT) {
        return OK_EXIT;
    } else if (bi_result == BI_EXECUTED) {
        return OK;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return ERR_EXEC_CMD;
    } else if (pid == 0) {
        if (cmd->in_redir_type == REDIR_IN) {
            int fd = open(cmd->in_redir_file, O_RDONLY);
            if (fd < 0) {
                perror("open input");
                exit(EXIT_FAILURE);
            }
            if (dup2(fd, STDIN_FILENO) < 0) {
                perror("dup2 input");
                exit(EXIT_FAILURE);
            }
            close(fd);
        }

        if (cmd->out_redir_type != REDIR_NONE) {
            int flags = O_WRONLY | O_CREAT;
            if (cmd->out_redir_type == REDIR_APPEND) {
                flags |= O_APPEND;
            } else {
                flags |= O_TRUNC;
            }

            int fd = open(cmd->out_redir_file, flags, 0644);
            if (fd < 0) {
                perror("open output");
                exit(EXIT_FAILURE);
            }
            if (dup2(fd, STDOUT_FILENO) < 0) {
                perror("dup2 output");
                exit(EXIT_FAILURE);
            }
            close(fd);
        }

        execvp(cmd->argv[0], cmd->argv);
        perror("execvp");
        exit(EXIT_FAILURE);
    } else {
        int status;
        waitpid(pid, &status, 0);
        return OK;
    }
}

int execute_pipeline(command_list_t *clist) {
    if (!clist || clist->num == 0) {
        return WARN_NO_CMDS;
    }

    if (clist->num == 1) {
        return exec_cmd(&clist->commands[0]);
    }

    int num_pipes = clist->num - 1;
    int pipes[num_pipes][2];
    pid_t pids[clist->num];

    for (int i = 0; i < num_pipes; i++) {
        if (pipe(pipes[i]) < 0) {
            perror("pipe");
            return ERR_EXEC_CMD;
        }
    }

    for (int i = 0; i < clist->num; i++) {
        pids[i] = fork();

        if (pids[i] < 0) {
            perror("fork");
            return ERR_EXEC_CMD;
        } else if (pids[i] == 0) {
	    if (i < clist->num - 1) {
                dup2(pipes[i][1], STDOUT_FILENO);
            }
            if (i > 0) {
                dup2(pipes[i - 1][0], STDIN_FILENO);
            }

            for (int j = 0; j < num_pipes; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            execvp(clist->commands[i].argv[0], clist->commands[i].argv);
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < num_pipes; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    for (int i = 0; i < clist->num; i++) {
        int status;
        waitpid(pids[i], &status, 0);
    }

    return OK;
}

int exec_local_cmd_loop() {
    char cmd_buff[SH_CMD_MAX];
    command_list_t clist;

    while (1) {

        if (fgets(cmd_buff, SH_CMD_MAX, stdin) == NULL) {
            printf("\n");
            break;
        }

        cmd_buff[strcspn(cmd_buff, "\n")] = '\0';

        if (strlen(cmd_buff) == 0) {
            continue;
        }

        if (strcmp(cmd_buff, EXIT_CMD) == 0) {
            printf("exiting...\n");
            break;
        }

        int rc = build_cmd_list(cmd_buff, &clist);
        if (rc != OK) {
            if (rc == WARN_NO_CMDS) {
                printf("%s", CMD_WARN_NO_CMD);
            } else if (rc == ERR_TOO_MANY_COMMANDS) {
                printf(CMD_ERR_PIPE_LIMIT, CMD_MAX);
            } else if (rc == ERR_MEMORY) {
                fprintf(stderr, "Memory allocation failed\n");
            }
            continue;
        }

        rc = execute_pipeline(&clist);
        if (rc == OK_EXIT) {
            break;
        }
	printf("dsh3> ");
        free_cmd_list(&clist);
	printf("%s", SH_PROMPT);
        fflush(stdout);
    }

    return OK;
}
