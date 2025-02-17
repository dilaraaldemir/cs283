#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "dshlib.h"

static char* strip_whitespace(char* str) {
    if (!str) return NULL;

    while (isspace((unsigned char)*str)) str++;

    if (*str == 0) return str;

    char* end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';

    return str;
}

/*
 *  build_cmd_list
 *    cmd_line:     the command line from the user
 *    clist *:      pointer to clist structure to be populated
 *
 *  This function builds the command_list_t structure passed by the caller
 *  It does this by first splitting the cmd_line into commands by spltting
 *  the string based on any pipe characters '|'.  It then traverses each
 *  command.  For each command (a substring of cmd_line), it then parses
 *  that command by taking the first token as the executable name, and
 *  then the remaining tokens as the arguments.
 *
 *  NOTE your implementation should be able to handle properly removing
 *  leading and trailing spaces!
 *
 *  errors returned:
 *
 *    OK:                      No Error
 *    ERR_TOO_MANY_COMMANDS:   There is a limit of CMD_MAX (see dshlib.h)
 *                             commands.
 *    ERR_CMD_OR_ARGS_TOO_BIG: One of the commands provided by the user
 *                             was larger than allowed, either the
 *                             executable name, or the arg string.
 *
 *  Standard Library Functions You Might Want To Consider Using
 *      memset(), strcmp(), strcpy(), strtok(), strlen(), strchr()
 */

int build_cmd_list(char *cmd_line, command_list_t *clist) {
    if (!cmd_line || !clist) return ERR_CMD_OR_ARGS_TOO_BIG;

    memset(clist, 0, sizeof(command_list_t));

    char cmd_line_copy[ARG_MAX];
    strncpy(cmd_line_copy, cmd_line, ARG_MAX - 1);
    cmd_line_copy[ARG_MAX - 1] = '\0';

    char *trimmed_cmd = strip_whitespace(cmd_line_copy);

    if (strlen(trimmed_cmd) == 0) return WARN_NO_CMDS;

    char *saveptr1;
    char *pipe_token = strtok_r(trimmed_cmd, "|", &saveptr1);

    while (pipe_token != NULL) {
        if (clist->num >= CMD_MAX) return ERR_TOO_MANY_COMMANDS;

        char *cmd_str = strip_whitespace(pipe_token);

        char *saveptr2;
        char *token = strtok_r(cmd_str, " ", &saveptr2);

        if (token != NULL) {
            if (strlen(token) >= EXE_MAX) return ERR_CMD_OR_ARGS_TOO_BIG;
            strcpy(clist->commands[clist->num].exe, token);

            char arg_buffer[ARG_MAX] = "";
            token = strtok_r(NULL, " ", &saveptr2);
            while (token != NULL) {
                if (strlen(arg_buffer) + strlen(token) + 2 >= ARG_MAX) {
                    return ERR_CMD_OR_ARGS_TOO_BIG;
                }
                if (strlen(arg_buffer) > 0) strcat(arg_buffer, " ");
                strcat(arg_buffer, token);
                token = strtok_r(NULL, " ", &saveptr2);
            }

            if (strlen(arg_buffer) > 0) strcpy(clist->commands[clist->num].args, arg_buffer);

            clist->num++;
        }

        pipe_token = strtok_r(NULL, "|", &saveptr1);
    }

    return OK;
}
