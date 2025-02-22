#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>
#include "dshlib.h"

char* strip_whitespace(char *str) {
    if (!str) return NULL;

    while (isspace((unsigned char)*str)) str++;

    if (*str == 0) return str;

    char* end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';

    return str;
}

int alloc_cmd_buff(cmd_buff_t *cmd_buff) {
    cmd_buff->_cmd_buffer = malloc(SH_CMD_MAX);
    if (cmd_buff->_cmd_buffer == NULL) return ERR_MEMORY;
    return OK;
}

int free_cmd_buff(cmd_buff_t *cmd_buff) {
    free(cmd_buff->_cmd_buffer);
    return OK;
}

int clear_cmd_buff(cmd_buff_t *cmd_buff) {
    memset(cmd_buff, 0, sizeof(cmd_buff_t));
    return OK;
}

int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd_buff) {
    if (!cmd_line || !cmd_buff) return ERR_CMD_OR_ARGS_TOO_BIG;

    clear_cmd_buff(cmd_buff);

    char cmd_line_copy[SH_CMD_MAX];
    strncpy(cmd_line_copy, cmd_line, SH_CMD_MAX - 1);
    cmd_line_copy[SH_CMD_MAX - 1] = '\0';

    char *trimmed_cmd = strip_whitespace(cmd_line_copy);

    if (strlen(trimmed_cmd) == 0) return WARN_NO_CMDS;

    char *saveptr1;
    char *pipe_token = strtok_r(trimmed_cmd, PIPE_STRING, &saveptr1);

    while (pipe_token != NULL) {
        if (cmd_buff->argc >= CMD_MAX) return ERR_TOO_MANY_COMMANDS;

        char *cmd_str = strip_whitespace(pipe_token);

        char *saveptr2;
        char *token = strtok_r(cmd_str, " ", &saveptr2);

        if (token != NULL) {
            if (strlen(token) >= EXE_MAX) return ERR_CMD_OR_ARGS_TOO_BIG;
            cmd_buff->argv[cmd_buff->argc++] = token;

            token = strtok_r(NULL, " ", &saveptr2);
            while (token != NULL) {
                if (cmd_buff->argc >= CMD_ARGV_MAX) return ERR_CMD_OR_ARGS_TOO_BIG;
                cmd_buff->argv[cmd_buff->argc++] = token;
                token = strtok_r(NULL, " ", &saveptr2);
            }
        }

        pipe_token = strtok_r(NULL, PIPE_STRING, &saveptr1);
    }

    return OK;
}

Built_In_Cmds match_command(const char *input) {
    if (strcmp(input, EXIT_CMD) == 0) return BI_CMD_EXIT;
    return BI_NOT_BI;
}

Built_In_Cmds exec_built_in_cmd(cmd_buff_t *cmd) {
    if (strcmp(cmd->argv[0], EXIT_CMD) == 0) {
        return BI_CMD_EXIT;
    }
    return BI_NOT_BI;
}

int exec_local_cmd_loop() {
    char *cmd_buff;
    int rc = 0;
    cmd_buff_t cmd;

    cmd_buff = malloc(SH_CMD_MAX * sizeof(char));
    if (cmd_buff == NULL) {
        perror("Memory allocation failed");
        return ERR_MEMORY;
    }

    while (1) {
        printf("%s", SH_PROMPT);

        if (fgets(cmd_buff, SH_CMD_MAX, stdin) == NULL) {
            printf("\n");
            break;
        }

        cmd_buff[strcspn(cmd_buff, "\n")] = '\0';

        build_cmd_buff(cmd_buff, &cmd);

        if (cmd.argc == 0) {
            printf(CMD_WARN_NO_CMD);
            continue;
        }

        if (match_command(cmd_buff) != BI_NOT_BI) {
            if (exec_built_in_cmd(&cmd) == BI_CMD_EXIT) {
                break;
            }
            continue;
        }

        pid_t pid = fork();
        if (pid == 0) {
            if (execvp(cmd.argv[0], cmd.argv) == -1) {
                perror("Execution failed");
                exit(1);
            }
        } else if (pid > 0) {
            waitpid(pid, &rc, 0);
        } else {
            perror("Fork failed");
        }
        free(cmd_buff);
    }

    return OK;
}
