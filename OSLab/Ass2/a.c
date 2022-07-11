#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/types.h>

#include <fcntl.h>

#define SHELL_BUFF_SIZE 1024
#define SHELL_TOKEN_SIZE 100
#define SHELL_BUILT_INS 3
#define SHELL_PIPE_DELIM '|'

int read_line(char *line);
int shell_exec_cd(char **args);
int shell_exec_exit(char **args);
int shell_exec_help(char **args);
void log_error(char *msg);
char **shell_split_pipes(char *line, int *num_pipes);
char **shell_split_command(char *command, int *tot_tokens);
int comm_execute(char **args, int num_args);
int shell_launch(char **args, int num_args, int in_fd, int out_fd);
// built-in commands for the parent process
char *builtIns[] = {
    "cd",
    "exit",
    "help"
};

int main()
{
    char *line;           // Input command line given
    char **args;          // args to pass for execution
    int status;           // code for status of the execution command
    int num_pipe_process; // no of commands separated by piped operator
    char **pipe_commands; // num_pipe_process are separated by num_pipe_process-1 pipes
    int tot_tokens;       // number of tokens in line
    do
    {
        line = (char *)malloc(sizeof(char) * SHELL_BUFF_SIZE);
        printf("\n>> ");
        // printf("yashi");
        status = read_line(line);

        // scanf("%[^\n]s", line);
        //  printf("%s", line);
        pipe_commands = shell_split_pipes(line, &num_pipe_process);
        // printf("%d", num_pipe_process);
        if (num_pipe_process == 1)
        {
            // split line into tokens
            args = shell_split_command(line, &tot_tokens);
            // printf(" %d", tot_tokens);
            // for (int yp = 0; yp < tot_tokens; yp++)
            //     printf(" %s", args[yp]);
            if (tot_tokens > 0 && args != NULL)
                status = comm_execute(args, tot_tokens);
        }
        else if (num_pipe_process > 1)
        {
            int i, in_fd = 0;
            bool pipe_err = false;
            int FD[2]; // store the read and write file descripters
            for (i = 0; i < num_pipe_process - 1; i++)
            {
                if (pipe(FD) < 0)
                {
                    pipe_err = 1;
                    log_error("Error: Error in piping\n");
                    break;
                }
                else
                {
                    args = shell_split_command(pipe_commands[i], &tot_tokens);
                    // execute pipe process and write to FD[1]
                    if (tot_tokens > 0)
                    {
                        status = shell_launch(args, tot_tokens, in_fd, FD[1]);
                        close(FD[1]);
                        // set the in file descriptor for next command as FD[0]
                        in_fd = FD[0];
                    }
                    else
                    {
                        pipe_err = true;
                        log_error("Error: Syntax error\n");
                        break;
                    }
                }
            }
            if (!pipe_err)
            {
                args = shell_split_command(pipe_commands[i], &tot_tokens);
                if (tot_tokens > 0)
                {
                    status = shell_launch(args, tot_tokens, in_fd, 1);
                }
                else
                {
                    pipe_err = true;
                    log_error("Error: Syntax error\n");
                    break;
                }
            }
        }
        // free(line);
        int len_line = strlen(line);
        for (int i = 0; i < len_line; i++)
            line[i] = '\0';
        free(args);
        fflush(stdin);
        fflush(stdout);

        /* code */
    } while (status == EXIT_SUCCESS);
}
void log_error(char *msg)
{
    printf("%s\n", msg);
}
// functions to execute for built-in commands
int (*builtInFuncs[])(char **) = {
    &shell_exec_cd,
    &shell_exec_exit,
    &shell_exec_help
};

int read_line(char *line)
{
    // char c;
    // int ind = 0, bufsize = SHELL_BUFF_SIZE;
    // scanf("%[^\n]s", &line);
    // while (1)
    // {
    //     c = getchar();
    //     // printf("%c", c);
    //     //  if ((c != EOF) && (c != '\n'))
    //     //      fflush(stdin);
    //     //  read till EOF or newline encountered
    //     if (c == EOF || c == '\n')
    //     {
    //         line[ind] = '\0';
    //         // printf("%d eof", ind);
    //         return EXIT_SUCCESS;
    //     }
    //     line[ind++] = c;
    //     if (ind >= bufsize)
    //     {
    //         // printf("%d exceed", ind);
    //         //  reallocate memory if length of command exceeds bufsize
    //         bufsize += SHELL_BUFF_SIZE;
    //         line = (char *)realloc(line, bufsize);
    //         if (line == NULL)
    //         {
    //             log_error("Error: Memory allocation error\n");
    //             exit(EXIT_FAILURE);
    //         }
    //     }
    //     // printf("%d same", ind);
    // }
    // printf("Line read!");

    char c;
    int position = 0;
    // maximum length of command line
    int bufsize = SHELL_BUFF_SIZE;

    while (1)
    {
        c = getchar();
        // read till EOF or newline encountered
        if (c == EOF || c == '\n')
        {
            line[position] = '\0';
            return EXIT_SUCCESS;
        }
        line[position] = c;
        position++;

        if (position >= bufsize)
        {
            // reallocate memory if length of command exceeds bufsize
            bufsize += SHELL_BUFF_SIZE;
            line = (char *)realloc(line, bufsize);
            if (line == NULL)
            {
                log_error("Error: failed to allocate memory\n");
                exit(EXIT_FAILURE);
            }
        }
    }
    return (EXIT_SUCCESS);
}

char **shell_split_pipes(char *line, int *num_pipes)
{
    int bufsize = SHELL_TOKEN_SIZE, len = strlen(line);
    *num_pipes = 0;
    char **arr_commands = (char **)malloc(sizeof(char *) * SHELL_TOKEN_SIZE);
    if (arr_commands == NULL)
    {
        log_error("Error: Memory allocation error\n");
        exit(EXIT_FAILURE);
    }
    // printf("%s %d", line, len);

    for (int i = 0; i < bufsize; i++)
    {
        arr_commands[i] = (char *)malloc(sizeof(char) * len);
        if (arr_commands[i] == NULL)
        {
            log_error("Error: Memory allocation error\n");
            exit(EXIT_FAILURE);
        }
    }
    // return arr_commands;
    int j = 0, ind = 0;
    for (int i = 0; i < len;)
    {
        if (line[i] == '|')
        {
            if (strlen(arr_commands[ind]) <= 0)
            {
                fprintf(stderr, "Error: Syntax error\n");
                *num_pipes = 0;
                return NULL;
            }
            ind++;
            if (ind >= bufsize)
            {
                bufsize += SHELL_TOKEN_SIZE;
                arr_commands = (char **)realloc(arr_commands, sizeof(char *) * bufsize);
                if (arr_commands == NULL)
                {
                    log_error("Error: Memory allocation error\n");
                    exit(EXIT_FAILURE);
                }
            }
            j = 0;
            i++;
        }
        //" is encountered
        else if (line[i] == '\"')
        {
            arr_commands[ind][j++] = line[i++];
            while (i < len && line[i] != '\"')
            {
                arr_commands[ind][j++] = line[i++];
            }
            arr_commands[ind][j++] = line[i++];
        }
        //' is emcountered
        else if (line[i] == '\'')
        {
            arr_commands[ind][j++] = line[i++];
            while (i < len && line[i] != '\'')
            {
                arr_commands[ind][j++] = line[i++];
            }
            arr_commands[ind][j++] = line[i++];
        }
        // if any other character is encountered
        else
        {
            arr_commands[ind][j++] = line[i++];
        }
    }

    *num_pipes = ind + 1;
    arr_commands[*num_pipes] = NULL;
    return arr_commands;
}
char **shell_split_command(char *command, int *tot_tokens)
{
    *tot_tokens = 0;
    int bufsize = SHELL_TOKEN_SIZE, len = strlen(command), is_token;
    // arr is an array of tokens in the command
    char **arr = (char **)malloc(sizeof(char *) * SHELL_TOKEN_SIZE);
    if (arr == NULL)
    {
        log_error("Error: Memory allocation error\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < SHELL_TOKEN_SIZE; i++)
    {
        arr[i] = (char *)malloc(sizeof(char) * len);
        if (arr[i] == NULL)
        {
            log_error("Error: Memory allocation error\n");
            exit(EXIT_FAILURE);
        }
    }
    int j, ind = 0;
    for (int i = 0; i < len;)
    {
        // unsigned char sp = ' ';    // space
        // unsigned char n = '\n';    // newline
        // unsigned char lapo = '\''; // left apostrophe
        // if any whitespace or carriage return or end of line encountered then increase tot_arr
        if (command[i] == ' ' || command[i] == '\n' || command[i] == '\r' || command[i] == '\a' || command[i] == '\t')
        {
            if (strlen(arr[(*tot_tokens)]) > 0)
            {
                (*tot_tokens)++;
            }
            ind = 0;
            i++;
        }
        // if single quote encountered read till next single quote
        else if (command[i] == '\'')
        {
            ind = 0;
            j = i + 1;
            while (j < len && command[j] != '\'')
            {
                arr[(*tot_tokens)][ind++] = command[j++];
            }
            if (strlen(arr[(*tot_tokens)]) > 0)
            {
                (*tot_tokens)++;
            }
            ind = 0;
            i = j + 1;
        }
        // if double quote encountered read till next double quote
        else if (command[i] == '\"')
        {
            // argument in double quotes can't be passed to awk command
            if ((*tot_tokens) - 1 > 0 && arr[(*tot_tokens) - 1][0] == 'a' && arr[(*tot_tokens) - 1][1] == 'w' && arr[(*tot_tokens) - 1][2] == 'k')
            {
                fprintf(stderr, "Error: Syntax error\n");
                return NULL;
            }
            ind = 0;
            j = i + 1;
            while (j < len && command[j] != '\"')
            {
                arr[(*tot_tokens)][ind++] = command[j++];
            }
            if (strlen(arr[(*tot_tokens)]) > 0)
            {
                (*tot_tokens)++;
            }
            ind = 0;
            i = j + 1;
        } // any other character encountered add to current tokens index
        else
        {
            arr[(*tot_tokens)][ind] = command[i];
            ind++;
            if (i == len - 1)
            {
                if (strlen(arr[(*tot_tokens)]) > 0)
                {
                    (*tot_tokens)++;
                }
            }
            i++;
        } // reallocate memory if tot_tokens exceed current bufsize
        if ((*tot_tokens) >= bufsize)
        {
            bufsize += SHELL_TOKEN_SIZE;
            arr = (char **)realloc(arr, sizeof(char *) * bufsize);
            if (arr == NULL)
            {
                log_error("Error: Memory allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
    return arr;
}
// Takes as input a command and its arguments and executes it in shell
int comm_execute(char **args, int num_args)
{
    // printf("%s", args[0]);
    if (args[0] == NULL) // exit if no command
        return EXIT_SUCCESS;
    else
    {
        // compare command with a built-in command
        // need to execute separately
        for (int i = 0; i < SHELL_BUILT_INS; i++)
        {
            if (strcmp(builtIns[i], args[0]) == 0)
            {
                return (*builtInFuncs[i])(args);
            }
        }
        // if no built-in command call shell_launch
        return shell_launch(args, num_args, 0, 1);
    }
}
int shell_launch(char **args, int num_args, int in_fd, int out_fd)
{
    // printf("Hi");
    int status;
    pid_t pid = fork(); // create a child process
    pid_t wpid;
    char *argument;
    if (pid < 0)
    {
        fprintf(stderr, "Error: error in forking\n");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0) // Child Process
    {
        printf("Child");
        // move in_fd to 0(redirect stdin to in_fd) as file descriptor for stdin is 0
        if (in_fd != 0)
        {
            dup2(in_fd, 0);
            close(in_fd);
        }
        // move out_fd to 1(redirect stdout to out_fd) as file descriptor for stdout is 1
        if (out_fd != 1)
        {
            dup2(out_fd, 1);
            close(out_fd);
        }
        int main_cmd = 0;
        int len_pos = 0; // index position of main command
        bool first_sym = false;
        int firstSymbolFound = 0;
        // mainCommand is the command before any < > or & character
        // firstSymbolFound is flag to check if any < > or & symbol is found or not
        for (int i = 0; i < num_args; i++)
        {
            argument = args[i];
            if (!first_sym)
            {
                if (strcmp(argument, "<") == 0 || strcmp(argument, ">") == 0 || strcmp(argument, "&") == 0)
                {
                    main_cmd = i;     // mainCommand length is i
                    first_sym = true; // first < > or & symbol has been found
                    len_pos = i;
                }
            }
            if (strcmp(argument, "<") == 0 && i + 1 < num_args) // input redirection
            {
                // open file to read from
                int rd_in_fd = open(args[i + 1], O_RDONLY);
                // redirect stdin to rd_in_fd
                dup2(rd_in_fd, STDIN_FILENO);
            }
            if (strcmp(argument, ">") == 0 && i + 1 < num_args) // output redirection
            {
                // open file to write
                int rd_out_fd = open(args[i + 1], O_CREAT | O_TRUNC | O_WRONLY, 0666);
                // redirect stdout to redirect_out_fd
                dup2(rd_out_fd, STDOUT_FILENO);
            }
        }
        if (!first_sym)
            main_cmd = num_args;
        char **args2 = (char **)malloc(sizeof(char *) * (main_cmd + 1));
        if (args2 == NULL)
        {
            log_error("Error: Memory allocation error\n");
            exit(EXIT_FAILURE);
        }
        int i;
        for (i = 0; i < main_cmd; i++)
        {
            args2[i] = args[i];
        }
        args2[i] = NULL;
        // execute the command by allocating child process address space & pid
        // to new process by using execvp
        printf("%s", args[0]);
        if (execvp(args2[0], args2) == -1)
        {
            log_error("Error: Memory allocation error\n");
            exit(EXIT_FAILURE);
        }
    }    // error in forking
    else // parent process
    {
        // wait for completion of child process unless last character is &
        if (num_args - 1 >= 0 && strcmp(args[num_args - 1], "&") != 0)
        {
            do
            {
                wpid = waitpid(pid, &status, WUNTRACED);
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        }
    }
    return EXIT_SUCCESS;
}
// display help
int shell_exec_help(char **args)
{
    printf("SHELL HELP\n");
    printf("These are the built-in commands:\n");
    for (int i = 0; i < SHELL_BUILT_INS; i++)
    {
        printf("%s\n", builtIns[i]);
    }
    printf("Type man <command> to know about a command\n");
    return EXIT_SUCCESS;
}
int shell_exec_cd(char **args)
{
    if (args[1] == NULL)
    {
        log_error("Error:No arguments to cd command\n");
    }
    else
    {
        if (chdir(args[1]) != 0)
        {
            log_error("Error: directory not found\n");
        }
    }
    return EXIT_SUCCESS;
}
// exit from shell
int shell_exec_exit(char **args)
{
    return 1;
}
