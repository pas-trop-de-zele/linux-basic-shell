#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_CMD_LINE_ARGS 128

int min(int a, int b) { return a < b ? a : b; }

// break a string into its tokens, putting a \0 between each token
//   save the beginning of each string in a string of char *'s (ptrs to chars)
int parse(char *s, char *argv[])
{
    const char break_chars[] = " \t;";
    char *p;
    int c = 0;
    /* TODO */ // write parser that breaks input into argv[] structure
    // e.g., cal  -h  2022\0      // would be
    //       |    |   |
    //       |  \0| \0|   \0      // '\0' where all the spaces are
    //       p0   p1  p2          // array of ptrs to begin. of strings ("cal", "-h", "2022")
    //                            // char* argv[]

    return c; // int argc
}

// execute a single shell command, with its command line arguments
//     the shell command should start with the name of the command
int execute(char *input)
{
    int i = 0;
    char *shell_argv[MAX_CMD_LINE_ARGS];
    memset(shell_argv, 0, MAX_CMD_LINE_ARGS * sizeof(char));

    int shell_argc = parse(input, shell_argv);
    // printf("after parse, what is input: %s\n", input);      // check parser
    // printf("argc is: %d\n", shell_argc);
    // while (shell_argc > 0) {
    //   printf("argc: %d: %s\n", i, shell_argv[i]);
    //   --shell_argc;
    //   ++i;
    // }

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
        if ((ret = execvp(*shell_argv, shell_argv)) < 0)
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

        if (strlen(input) > 0)
        {
            strncpy(last_input, input, min(strlen(input), BUFSIZ));
            memset(last_input, 0, BUFSIZ * sizeof(char));
        }

        if ((fgets(input, BUFSIZ, stdin)) == NULL)
        { // or gets(input, BUFSIZ);
            fprintf(stderr, "no command entered\n");
            exit(1);
        }
        input[strlen(input) - 1] = '\0'; // wipe out newline at end of string
        // printf("input was: '%s'\n", input);
        // printf("last_input was: '%s'\n", last_input);
        if (strncmp(input, "exit", 4) == 0)
        { // only compare first 4 letters
            finished = true;
        }
        else if (strncmp(input, "!!", 2) == 0)
        { // check for history command
          // TODO
        }
        else
        {
            execute(input);
        }
    }

    printf("\t\t...exiting\n");
    return 0;
}
