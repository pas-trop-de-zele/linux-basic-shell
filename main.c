#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_CMD_LINE_ARGS 128

int min(int a, int b) { return a < b ? a : b; }

int parse(char *input, char *argv[])
{
    int argument_start = 0;
    int arg_no = 0;
    int i = 0;

    int input_length = strlen(input);
    while (i < input_length + 1)
    {
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
    for (int i = 0; i < shell_argc; i++)
    {
        printf("%s \n", shell_argv[i]);
    }

    int status = 0;
    pid_t pid = fork();

    if (pid < 0)
    {
        fprintf(stderr, "Fork() failed\n");
    } // send to stderr
    else if (pid == 0)
    { // child
        int ret = 0;
        // if ((ret = execlp("cal", "cal", NULL)) < 0) {  // can do it arg by arg, ending in NULL
        if ((ret = execvp(shell_argv[0], shell_argv)) < 0)
        {
            fprintf(stderr, "execlp(%s) failed with error code: %d\n", *shell_argv, ret);
        }
        printf("\n");
    }
    else
    { // parent -----  don't wait if you are creating a daemon (background) process
        while (wait(&status) != pid)
        {
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
