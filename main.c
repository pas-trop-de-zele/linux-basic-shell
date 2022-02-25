#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/fcntl.h>

#define MAX_CMD_LINE_ARGS 128

bool is_daemon = false;
bool input_redirect = false;
bool output_redirect = false;
bool is_piped = false;
int redirect_argument_index = -1;
int pipe_argument_index = -1;

int min(int a, int b) { return a < b ? a : b; }

int parse(char *input, char *argv[])
{
    int argument_start = 0;
    int arg_no = 0;
    int i = 0;
    is_daemon = false;
    input_redirect = false;
    output_redirect = false;
    is_piped = false;
    redirect_argument_index = -1;
    pipe_argument_index = -1;

    int input_length = strlen(input);
    if (input[input_length - 1] == '&')
    {
        is_daemon = true;
    }

    while (i < input_length + 1)
    {
        if (input[i] == '<')
        {
            input_redirect = true;
            redirect_argument_index = arg_no;
        }
        else if (input[i] == '>')
        {
            output_redirect = true;
            redirect_argument_index = arg_no;
        }
        else if (input[i] == '|')
        {
            is_piped = true;
            pipe_argument_index = arg_no;
        }

        if (input[i] == ' ' || i == input_length)
        {
            if (argument_start < i)
            {
                input[i] = '\0';
                argv[arg_no] = input + argument_start;
                arg_no++;
            }
            argument_start = i + 1;
        }
        i++;
    }
    return arg_no;
}

// execute a single shell command, with its command line arguments
//     the shell command should start with the name of the command
int execute(char *input)
{
    int i = 0;
    char *shell_argv[MAX_CMD_LINE_ARGS];
    memset(shell_argv, 0, MAX_CMD_LINE_ARGS * sizeof(char));

    int shell_argc = parse(input, shell_argv);

    if (is_daemon)
    {
        shell_argv[shell_argc - 1] = NULL;
    }

    pid_t pid = fork();

    if (pid < 0)
    {
        fprintf(stderr, "Fork() failed\n");
    }
    else if (pid == 0)
    {
        if (redirect_argument_index != -1)
        {
            shell_argv[redirect_argument_index] = NULL;
            char *file_name = shell_argv[redirect_argument_index + 1];
            if (input_redirect)
            {
                int file = open(file_name, O_RDONLY);
                if (file == -1)
                {
                    printf("FILE DOES NOT EXIST!\n");
                    exit(-1);
                }
                dup2(file, STDIN_FILENO);
                close(file);
            }
            else if (output_redirect)
            {
                int file = open(file_name, O_WRONLY | O_CREAT, 0777);
                dup2(file, STDOUT_FILENO);
                close(file);
            }
            int ret = 0;
            if ((ret = execvp(shell_argv[0], shell_argv)) < 0)
            {
                fprintf(stderr, "execlp(%s) failed with error code: %d\n", *shell_argv, ret);
            }

            printf("\n");
        }
        else if (pipe_argument_index != -1)
        {
            shell_argv[pipe_argument_index] = NULL;
            int fd[2];
            if (pipe(fd) == -1)
            {
                exit(1);
            }

            pid_t write_proc = fork();
            if (write_proc < 0)
            {
                exit(1);
            }
            else if (write_proc == 0)
            {
                close(fd[0]);
                dup2(fd[1], STDOUT_FILENO);
                int ret = 0;
                if ((ret = execvp(shell_argv[0], shell_argv)) < 0)
                {
                    fprintf(stderr, "execlp(%s) failed with error code: %d\n", *shell_argv, ret);
                }
                close(fd[1]);
            }

            close(fd[1]);
            dup2(fd[0], STDIN_FILENO);
            int ret = 0;
            if ((ret = execvp(shell_argv[pipe_argument_index + 1], shell_argv + pipe_argument_index + 1)) < 0)
            {
                fprintf(stderr, "execlp(%s) failed with error code: %d\n", *shell_argv, ret);
            }
            close(fd[0]);

            close(fd[0]);
            close(fd[1]);
            waitpid(write_proc, NULL, 0);
        }
        else
        {
            int ret = 0;
            if ((ret = execvp(shell_argv[0], shell_argv)) < 0)
            {
                fprintf(stderr, "execlp(%s) failed with error code: %d\n", *shell_argv, ret);
            }
        }
    }
    else
    {
        if (!is_daemon)
        {
            wait(NULL);
        }
    }

    return 0;
}

int main(int argc, const char *argv[])
{
    char input[BUFSIZ];
    char last_input[BUFSIZ];
    bool finished = false;

    memset(last_input, 0, BUFSIZ * sizeof(char));
    while (!finished)
    {
        memset(input, 0, BUFSIZ * sizeof(char));

        printf("osh > ");
        fflush(stdout);

        fgets(input, BUFSIZ, stdin);
        if (input[0] == '\n')
        {
            fprintf(stderr, "No command entered\n");
            exit(1);
        }

        input[strlen(input) - 1] = 0; // wipe out newline at end of string
        // printf("input was: '%s'\n", input);
        // printf("last_input was: '%s'\n", last_input);
        if (strncmp(input, "exit", 4) == 0)
        { // only compare first 4 letters
            finished = true;
        }
        else if (strncmp(input, "!!", 2) == 0)
        {
            if (strlen(last_input) == 0)
            {
                printf("No commands in history\n");
            }
            else
            {
                fflush(stdout);
                printf("%s\n", last_input);
                execute(last_input);
            }
        }
        else
        {
            strcpy(last_input, input);
            execute(input);
        }
    }

    printf("\t\t...exiting\n");
    return 0;
}
