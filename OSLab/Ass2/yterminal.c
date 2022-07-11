#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <signal.h>
#include <termios.h>
#include "fcntl.h"

#define LINE_BUFSIZE 1024
#define BUFF2D 64
#define MAX_HIST_SIZE 10000
#define PRINT_HIST_SIZE 1000
#define HISTORY_FILE ".history.txt"

char **history;
int history_index;
int history_buff = MAX_HIST_SIZE;

char *builtIns[] = {
    "cd",
    "exit",
    "help",
    "history"
};

int numBuiltins();
int shellCd(char **args);
int shellExit(char **args);
int shellHelp(char **args);
int shellHistory(char **args);
int shellLaunch(char **args, int numArgs, int fdIn, int fdOut);
int commandsExecute(char **args, int numArgs, int fdIn, int fdOut);
char **tokeniseLine(char *line, int *token_count);
void terminalLoop(void);
char *readLine(void);
char **pipeSplit(char *line, int *pipeCount);
void handle_sigint(int);
void handle_sigtstp(int sig);
void storeHistory();
int LCSubStr(char *X, char *Y, int m, int n);
void ctrlRHandler();
char *getsubstr(char arr[], int start, int end);
int issame(char s1[], char *s2);
void tabHandler(char *line, int *index);

char *getsubstr(char arr[], int start, int end)
{
    if (start > end)
        return NULL;
    char *s = (char *)malloc(sizeof(char) * (end - start + 1));
    int i = 0;
    while (start <= end)
    {
        s[i] = arr[start];
        start++;
        i++;
    }
    return s;
}
int issame(char s1[], char *s2)
{
    if (strcmp(s1, s2) == 0)
        return 1;
    return 0;
}

struct termios saved_attributes;

void reset_input_mode(void)
{
    tcsetattr(STDIN_FILENO, TCSANOW, &saved_attributes);
}

void set_input_mode(void)
{
    struct termios tattr;
    char *name;

    /* Make sure stdin is a terminal. */
    if (!isatty(STDIN_FILENO))
    {
        fprintf(stderr, "Not a terminal.\n");
        exit(EXIT_FAILURE);
    }

    /* Save the terminal attributes so we can restore them later. */
    tcgetattr(STDIN_FILENO, &saved_attributes);
    atexit(reset_input_mode);

    /* Set the funny terminal modes. */
    tcgetattr(STDIN_FILENO, &tattr);
    tattr.c_lflag &= ~(ICANON | ECHO); /* Clear ICANON and ECHO. */
    tattr.c_cc[VMIN] = 1;
    tattr.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &tattr);
}

int flag = 0;
int flag_tab = 0;
void handle_sigint(int sig)
{
    // char c;

    // signal(sig, SIG_IGN);
    // printf(" \n OUCH, did you hit Ctrl-C?\n"
    //        "Do you really want to quit? [y/n] ");
    // c = getchar();
    // if (c == 'y' || c == 'Y')
    //     exit(0); // we dont want to do exit here
    // else
    // {
    signal(SIGINT, handle_sigint);
    //     terminalLoop();
    // }
    // getchar(); // Get new line character
}
void handle_sigtstp(int sig)
{
    flag = 1;
    // signal(SIGINT, handle_sigtstp);
}
int (*builtInFuncs[])(char **) = {
    &shellCd,
    &shellExit,
    &shellHelp,
    &shellHistory
};

int main(int argc, char **argv)
{
    set_input_mode();

    signal(SIGINT, handle_sigint);
    signal(SIGTSTP, handle_sigtstp);
    history = (char **)malloc(sizeof(char *) * MAX_HIST_SIZE);
    FILE *fp;
    size_t len = 0;
    size_t read;
    char *line = NULL;
    history_index = 0;
    fp = fopen(HISTORY_FILE, "r");

    while ((read = getline(&line, &len, fp)) != -1)
    {
        history[history_index] = strdup(line);
        history_index++;
    }
    fclose(fp);
    // printf("%s", history[1]);
    // exit(1);

    terminalLoop();
    reset_input_mode();
    return 1;
}

void terminalLoop(void)
{
    char *line;
    char **args;
    int token_count = 0;
    int status;
    int pipeCount = 0;
    char **pipeCommands;

    do
    {
        printf(">> ");
        line = readLine();
        if (strlen(line) == 0)
            continue;
        char *line_copy = strdup(line);
        line_copy = strcat(line_copy, "\n");
        history[history_index++] = line_copy;
        // Realloc history array
        if (history_index >= history_buff)
        {
            history_buff += MAX_HIST_SIZE;
            history = (char **)realloc(history, sizeof(char *) * history_buff);
        }
        pipeCommands = pipeSplit(line, &pipeCount);
        printf("pipeCount : %d\n", pipeCount);

        if (pipeCount == 1)
        {
            //  printf("%s:Line %ld", line, strlen(line));
            args = tokeniseLine(line, &token_count);
            // if (flag_tab)
            // {
            //     for (int i = 0; i < token_count; i++)
            //     {
            //         int len_args = strlen(args[i]);
            //         printf("%s\n %ld", args[i], strlen(args[i]));
            //         args[i][len_args] = '\0';
            //         printf("%s %ld\n", args[i], strlen(args[i]));
            //     }
            //     flag_tab = 0;
            // }
            printf("Token Count : %d\n", token_count);

            if (args != NULL && token_count > 0)
                status = commandsExecute(args, token_count, 0, 1);
            else
                status = 1;
        }
        else if (pipeCount > 1)
        {
            int i, fd_out, fd_in = 0;
            int pipeFlag = 0;
            int FD[2];
            for (i = 0; i < pipeCount; i++)
            {
                if (pipe(FD) < 0)
                {
                    pipeFlag = 1;
                    printf("ERROR :: Piping\n");
                    break;
                }
                else
                {
                    args = tokeniseLine(pipeCommands[i], &token_count);
                    printf("Token Count : %d\n", token_count);
                    if (args != NULL && token_count > 0)
                    {
                        fd_out = FD[1];
                        if (i == pipeCount - 1)
                        {
                            fd_out = 1;
                        }
                        status = commandsExecute(args, token_count, fd_in, fd_out);
                    }
                    else
                    {
                        pipeFlag = 1;
                        printf("ERROR :: Pipe Error\n");
                        break;
                    }
                    close(FD[1]);
                    fd_in = FD[0];
                }
            }
        }
        printf("Status : %d\n", status);

        free(line);
        free(args);
        fflush(stdin);
        fflush(stdout);
    } while (status);
}

// TODO : Resolve backspace issue
char *readLine(void)
{
    int index = 0;
    int buffsize = LINE_BUFSIZE;
    char *line = malloc(sizeof(char) * buffsize);
    char c;

    while (1)
    {
        c = getchar();
        if ((int)c == 127)
        {
            fputs("\b \b", stdout);
        }
        else
        {
            if ((int)c == 18)
            {
                // printf("DETECTED\n");
                ctrlRHandler();
                return line;
            }
            else if ((int)c == 9)
            {
                line[index] = '\0';
                flag_tab = 1;
                tabHandler(line, &index);
                // exit(1);
                // return line;
            }
            else
            {
                putchar(c);
                if (c == EOF || c == '\n')
                {
                    line[index] = '\0';
                    return line;
                }
                else
                {
                    line[index] = c;
                }
                index++;

                // Buffer Exceeded -> reallocate
                if (index >= buffsize)
                {
                    buffsize += LINE_BUFSIZE;
                    line = realloc(line, buffsize);
                    if (!line)
                    {
                        printf("ERROR :: Failed to reallocation memory for line\n");
                        exit(EXIT_FAILURE);
                    }
                }
            }
        }
    }
}

char **pipeSplit(char *line, int *pipeCount)
{
    *pipeCount = 0;
    int bufsize = BUFF2D;
    char **pipeCommands = (char **)malloc(sizeof(char *) * BUFF2D);
    if (pipeCommands == NULL)
    {
        printf("Error :: Memory allocation :: Pipe Buffer\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < bufsize; i++)
    {
        pipeCommands[i] = (char *)malloc(sizeof(char) * strlen(line));
        if (pipeCommands[i] == NULL)
        {
            printf("Error :: Memory allocation :: Pipe Buffer[i]\n");
            exit(EXIT_FAILURE);
        }
    }
    int ind = 0;
    int pipIndex = 0;
    for (int i = 0; i < strlen(line); i++)
    {
        if (line[i] == '|')
        {
            pipIndex++;
            ind = 0;
            if (pipIndex >= BUFF2D)
            {
                bufsize += BUFF2D;
                pipeCommands = (char **)realloc(pipeCommands, sizeof(char *) * bufsize);
            }
        }
        else
        {
            pipeCommands[pipIndex][ind] = line[i];
            ind++;
        }
    }
    *pipeCount = pipIndex + 1;
    pipeCommands[*pipeCount] = NULL;
    return pipeCommands;
}

char **tokeniseLine(char *line, int *token_count)
{
    *token_count = 0;
    int bufsize = BUFF2D;
    int len = strlen(line);
    int is_token;
    // printf("%s %ld", line, strlen(line));
    //  args is an array of tokens in the command
    char **args = (char **)malloc(sizeof(char *) * BUFF2D);
    if (args == NULL)
    {
        printf("Error :: Memory allocation :: Token Buffer\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < BUFF2D; i++)
    {
        args[i] = (char *)malloc(sizeof(char) * len);
        if (args[i] == NULL)
        {
            printf("Error :: Memory allocation :: Token Buffer[i]\n");
            exit(EXIT_FAILURE);
        }
    }
    int ind = 0;
    int j = 0;

    for (int i = 0; i < len;)
    {

        // printf("%d i value", i);
        if (line[i] == ' ' || line[i] == '\n' || line[i] == '\r' || line[i] == '\a')
        {
            args[*token_count][ind] = '\0';
            // printf("LOOP:%s %ld", args[*token_count], strlen(args[*token_count]));
            if (strlen(args[(*token_count)]) > 0)
            {
                (*token_count)++;
            }
            ind = 0;
            i++;
        }
        else if (line[i] == '\'')
        {
            ind = 0;
            j = i + 1;
            while (j < len && line[j] != '\'')
            {
                args[(*token_count)][ind++] = line[j++];
            }
            if (strlen(args[(*token_count)]) > 0)
            {
                (*token_count)++;
            }
            ind = 0;
            i = j + 1;
        }
        else if (line[i] == '\"')
        {
            ind = 0;
            j = i + 1;
            while (j < len && line[j] != '\'')
            {
                args[(*token_count)][ind++] = line[j++];
            }
            if (strlen(args[(*token_count)]) > 0)
            {
                (*token_count)++;
            }
            ind = 0;
            i = j + 1;
        }
        else
        {
            args[(*token_count)][ind] = line[i];
            ind++;
            if (i == len - 1)
            {
                if (strlen(args[(*token_count)]) > 0)
                {
                    (*token_count)++;
                }
            }
            i++;
        }

        // Buffer Exceeded -> Reallocate
        if ((*token_count) >= bufsize)
        {
            bufsize += BUFF2D;
            args = (char **)realloc(args, sizeof(char *) * bufsize);
            if (args == NULL)
            {
                printf("ERROR :: Memory Reallocation :: Token Buffer\n");
                exit(EXIT_FAILURE);
            }
        }
    }
    if (*token_count > 0)
        args[*token_count - 1][ind] = '\0';
    // for (int i = 0; i < *token_count; i++)
    // {
    //     int len_args = strlen(args[i]);
    //     printf("TOEKNIZE:%s\n %ld", args[i], strlen(args[i]));
    //     // args[i][len_args] = '\0';
    //     // printf("%s %ld\n", args[i], strlen(args[i]));
    // }
    return args;
}

int commandsExecute(char **args, int numArgs, int fdIn, int fdOut)
{
    if (args[0] == NULL)
    {   // No command
        return EXIT_SUCCESS;
    }

    for (int i = 0; i < numBuiltins(); i++)
    {
        if (strcmp(args[0], builtIns[i]) == 0)
        {
            return (*builtInFuncs[i])(args);
        }
    }
    return shellLaunch(args, numArgs, fdIn, fdOut);
}

int shellLaunch(char **args, int numArgs, int fdIn, int fdOut)
{
    pid_t pid, wpid;
    int status;
    int isBackground = 0;
    if (strcmp(args[numArgs - 1], "&") == 0)
    {
        isBackground = 1;
    }
    // args[numArgs] = NULL;
    flag = 0;
    pid = fork();
    if (pid == 0)
    {
        // Child process
        // move in_fd to 0(redirect stdin to in_fd) as file descriptor for stdin is 0
        // printf("Child Process ID : %d\n", getpid());
        if (fdIn != 0)
        {
            dup2(fdIn, 0);
            close(fdIn);
        }
        // move out_fd to 1(redirect stdout to out_fd) as file descriptor for stdout is 1
        if (fdOut != 1)
        {
            dup2(fdOut, 1);
            close(fdOut);
        }

        int commandIndex = 0;
        int symbolFound = 0;
        for (int i = 0; i < numArgs; ++i)
        {
            if (!symbolFound)
            {
                if (strcmp(args[i], "<") == 0 || strcmp(args[i], ">") == 0)
                {
                    commandIndex = i;
                    symbolFound = 1;
                    if (i < numArgs - 3)
                    {
                        if (strcmp(args[i + 2], "<") == 0 || strcmp(args[i + 2], ">") == 0)
                        {
                            if (strcmp(args[i + 2], "<") == 0)
                            {
                                int redirect_in_fd = open(args[i + 3], O_RDONLY);
                                dup2(redirect_in_fd, STDIN_FILENO);
                            }
                            if (strcmp(args[i + 2], ">") == 0)
                            {
                                int redirect_out_fd = open(args[i + 3], O_CREAT | O_TRUNC | O_WRONLY, 0666);
                                dup2(redirect_out_fd, STDOUT_FILENO);
                            }
                        }
                    }

                    if (strcmp(args[i], "<") == 0)
                    {
                        int redirect_in_fd = open(args[i + 1], O_RDONLY);
                        dup2(redirect_in_fd, STDIN_FILENO);
                    }
                    if (strcmp(args[i], ">") == 0)
                    {
                        int redirect_out_fd = open(args[i + 1], O_CREAT | O_TRUNC | O_WRONLY, 0666);
                        dup2(redirect_out_fd, STDOUT_FILENO);
                    }
                }
            }
        }

        if (!symbolFound)
            commandIndex = numArgs;

        char **argsToExec = (char **)malloc(sizeof(char *) * (commandIndex + 1));
        for (int i = 0; i < commandIndex; ++i)
        {
            argsToExec[i] = args[i];
        }
        argsToExec[commandIndex] = NULL;

        if (execvp(argsToExec[0], argsToExec) == -1)
        {
            perror("shell: ");
            exit(EXIT_FAILURE);
        }
    }
    else if (pid < 0)
    {
        printf("ERROR :: Forking\n");
    }
    else
    {
        // Parent process
        // kill(pid, SIGINT);

        if (!isBackground)
        {
            do
            {
                if (flag)
                {
                    kill(pid, SIGCONT);
                    break;
                    // flag = 0;
                }
                wpid = waitpid(pid, &status, WUNTRACED);
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        }
    }

    return 1;
}

int shellCd(char **args)
{
    if (args[1] == NULL)
    {
        printf("ERROR :: Expected argument to \"cd\"\n");
    }
    else
    {
        if (chdir(args[1]) != 0)
        {
            printf("ERROR :: Directory not found\n");
        }
    }
    return 1;
}

void storeHistory()
{
    int index_temp = history_index > MAX_HIST_SIZE ? history_index - MAX_HIST_SIZE : 0;
    FILE *fp;
    fp = fopen(HISTORY_FILE, "w");

    for (index_temp; index_temp < history_index; index_temp++)
    {
        fprintf(fp, "%s", history[index_temp]);
    }
    fclose(fp);
}

int shellExit(char **args)
{
    reset_input_mode();
    storeHistory();
    return 0;
}

int shellHelp(char **args)
{
    int i;
    printf("Shell Help\n");
    printf("Built in commands :\n");

    for (i = 0; i < numBuiltins(); i++)
    {
        printf("  %s\n", builtIns[i]);
    }

    printf("type man <command> for manual of other commands\n");
    return 1;
}

int shellHistory(char **args)
{
    int index_temp = history_index > PRINT_HIST_SIZE ? history_index - PRINT_HIST_SIZE : 0;
    for (index_temp; index_temp < history_index; index_temp++)
    {
        printf("%d : %s", history_index - index_temp, history[index_temp]);
    }
    return 1;
}

int numBuiltins()
{
    return sizeof(builtIns) / sizeof(char *);
}

int LCSubStr(char *X, char *Y, int m, int n)
{
    int LCSuff[m + 1][n + 1];
    int result = 0;
    for (int i = 0; i <= m; i++)
    {
        for (int j = 0; j <= n; j++)
        {
            if (i == 0 || j == 0)
                LCSuff[i][j] = 0;

            else if (X[i - 1] == Y[j - 1])
            {
                LCSuff[i][j] = LCSuff[i - 1][j - 1] + 1;
                result = result > LCSuff[i][j] ? result : LCSuff[i][j];
            }
            else
                LCSuff[i][j] = 0;
        }
    }
    return result;
}

void ctrlRHandler()
{
    printf("Enter search term : ");

    int index = 0;
    int buffsize = LINE_BUFSIZE;
    char *commToSearch = malloc(sizeof(char) * buffsize);
    char c;

    while (1)
    {
        c = getchar();
        if ((int)c == 127)
        {
            fputs("\b \b", stdout);
        }
        else
        {
            putchar(c);
            if (c == EOF || c == '\n')
            {
                commToSearch[index] = '\0';
                // return commToSearch;
                break;
            }
            else
            {
                commToSearch[index] = c;
            }
            index++;

            // Buffer Exceeded -> reallocate
            if (index >= buffsize)
            {
                buffsize += LINE_BUFSIZE;
                commToSearch = realloc(commToSearch, buffsize);
                if (!commToSearch)
                {
                    printf("ERROR :: Failed to reallocation memory for commToSearch\n");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
    // printf("%s", commToSearch);
    // printf("%ld", strlen(commToSearch));
    int comLen = strlen(commToSearch);
    int maxMatch = 0;
    int indexMaxMatch = 0;
    int arr[MAX_HIST_SIZE];
    for (int i = history_index - 1; i >= history_index - MAX_HIST_SIZE; --i)
    {
        if (i == -1)
            break;
        int matchLen = LCSubStr(history[i], commToSearch, strlen(history[i]), comLen);
        if (matchLen == (strlen(history[i]) - 1) && matchLen > 2)
        {
            printf("%s", history[i]);
            return;
        }
        if (matchLen > maxMatch)
        {
            maxMatch = matchLen;
            indexMaxMatch = 0;
            arr[indexMaxMatch++] = i;
        }
        else if (matchLen == maxMatch)
        {
            arr[indexMaxMatch++] = i;
        }
    }
    if (maxMatch > 2)
    {
        for (int i = indexMaxMatch - 1; i >= 0; i--)
        {
            printf("%s", history[arr[i]]);
        }
    }
    else
    {
        printf("No match for search term in history\n");
    }
    return;
}

void tabHandler(char *line, int *index)
{
    char **arr2;
    char **pipeCommands;
    int pipeCount;
    int token;
    char *line_copy = strdup(line);
    pipeCommands = pipeSplit(line_copy, &pipeCount);
    arr2 = tokeniseLine(line_copy, &token);
    // printf("%s", arr2[0]);
    char *prefix_string = strdup(arr2[token - 1]);
    int len_pre = strlen(prefix_string);
    DIR *d;
    struct dirent *dir;
    char **arr = malloc(1010 * sizeof(char *)); // Allocate row pointers
    for (int i = 0; i < 1010; i++)
        arr[i] = malloc(100 * sizeof(char));
    d = opendir(".");
    int i = 0; // indicates the number of files
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            if (strcmp(".", dir->d_name) == 0)
                continue;
            if (strcmp("..", dir->d_name) == 0)
                continue;
            int len_str = strlen(dir->d_name);
            if (len_str < len_pre)
                continue;

            // printf("%s\n", dir->d_name);
            if (len_pre <= 0)
            {
                strcpy(arr[i], dir->d_name);
                i++;
            }
            else
            {
                if (issame(prefix_string, getsubstr(dir->d_name, 0, len_pre - 1)))
                {
                    strcpy(arr[i], dir->d_name);
                    i++;
                }
            }
        }
        if (i > 1)
        {
            printf("\n");
            for (int pp = 0; pp < i; pp++)
            {
                printf("%d %s  ", pp + 1, arr[pp]);
            }
            printf("\nEnter your choice: ");
            int ch;
            scanf("%d", &ch);
            int yp = 0;
            int ind = *index;
            while (arr[ch - 1][yp + len_pre] != '\0')
            {
                // printf("%c", arr[ch][yp + len_pre]);
                line[yp + *index] = arr[ch - 1][yp + len_pre];
                // printf("%d %d", yp + *index, yp + len_pre);
                yp++;
                ind++;
            }
            line[yp + *index] = '\0';
            *index = ind;
            printf("\n");
            printf("%s\n", line);
        }
        else if (i == 1)
        {
            int yp = 0;
            int ind = *index;
            while (arr[0][yp + len_pre] != '\0')
            {
                printf("%c", arr[0][yp + len_pre]);
                line[yp + *index] = arr[0][yp + len_pre];
                // printf("%d %d", yp + *index, yp + len_pre);
                yp++;
                ind++;
            }
            line[yp + *index] = '\0';
            *index = ind;
        }

        closedir(d);
    }
    // printf("\n%s %ld\n", line, strlen(line));
}