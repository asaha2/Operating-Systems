#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <stdarg.h>

#define CHECK_NULL(val)     if(val == NULL) return -1;
#define MAX_CMD_CHAR        10
#define MAX_CMD_ARGS        50
#define DELIMETER_CHARS     " \t\n"
#define COMMAND_PROMPT      "simple-shell"

typedef void (*genericFuncPtr)(int cmdIdx, char **args, int argsLen);
void execBuiltInFnc(int cmdIdx, char **args, int argsLen);

typedef struct
{
    char *cmd;
    genericFuncPtr genericFunc;
    /* TODO: Need to expand for custom function execution */
} CommandTable_t;

CommandTable_t commandTable[] =
{
    {"history", NULL},
    {"cd",      execBuiltInFnc},
    {"pwd",     execBuiltInFnc},
    {"exit",    execBuiltInFnc},
};

int parseLine(char *prompt, char *args[], int argsLen, bool *isBackground)
{
    int length, argIdx = 0;
    char *token, *loc, *line;
    size_t lineCap = 0;

    /* Check for NULL pointer in passed arguments */
    CHECK_NULL(prompt)
    CHECK_NULL(isBackground)

    printf("%s: ", prompt);
    length = getline(&line, &lineCap, stdin);

    /* Handle Ctrl-D keystroke */
    if(feof(stdin))
    {
        args[argIdx++] = "exit";
        return argIdx;
    }

    if(length <= 0)
        return -1;

    /* Check if background flag is specified */
    if((loc = strchr(line, '&')) != NULL)
    {
        *isBackground = true;
        *loc = ' ';
    } else
    {
        *isBackground = false;
    }

    /* Tokenize line arguments */
    while((token = strsep(&line, DELIMETER_CHARS)) != NULL)
    {
        for(int idx = 0; idx < strlen(token); idx++)
        {
            /* Null terminate tokens */
            if(token[idx] <= ' ')
                token[idx] = '\0';
        }

        /* Need to ensure argument buffer isn't overflowed */
        if((strlen(token) > 0) && (argIdx < argsLen - 1))
            args[argIdx++] = token;
    }

    /* Return number of tokens parsed */
    return argIdx;
}

int scanTable(char *arg)
{
    CHECK_NULL(arg);

    /* Scan through command table and return match index */
    for(int idx = 0; idx < sizeof(commandTable) / sizeof(CommandTable_t); idx++)
    {
        if(strcmp(arg, commandTable[idx].cmd) == 0)
            return idx;
    }

    /* Default return invalid index */
    return -1;
}

void execBuiltInFnc(int cmdIdx, char **args, int argsLen)
{
    /* TODO: need to check for NULL */

    switch(cmdIdx)
    {
        char *currentDir;

        case 0: /* history */
            break;

        case 1: /* cd */
            if(chdir(args[1]) != 0)
                printf("Failed to change directory.\n");
            break;

        case 2: /* pwd */
            if((currentDir = getcwd(NULL, 0)) != NULL)
            {
                printf("%s\n", currentDir);
            } else
            {
                printf("Failed to retrieve current working directory.\n");
            }
            break;

        case 3: /* exit */
            exit(EXIT_SUCCESS);
            break;
    }

#if 0

    //if "cd" is typed
    else if(strcmp(args[0], bi_2) == 0)
    {
        int chk;
        chk = chdir(args[1]);
        if(chk != 0)
        {
            printf("Failed to execute chdir!\n");
        }
    }

    //if "pwd" is typed
    else if(strcmp(args[0], bi_3) == 0)
    {
        char cwd[1024], *chk;
        chk = getcwd(cwd, sizeof(cwd));
        if(chk == NULL)
        {
            printf("Failed to execute getcwd!\n");
        } else
        {
            printf("Current working dir: %s\n", cwd);
        }
    }

    //if "exit" is typed
    else
    {
        printf("Shell is exiting...\n");

        //de-allocate history story
        current = head;
        while(current != NULL)
        {
            int i;
            for(i = 0; !(current->values[i]); i++)
            {
                free(current->values[i]);
            }
            current = current->next;
        }
        free(current);
        free(head);

        //de-allocate job struct
        crnt = hd;
        while(crnt != NULL)
        {
            int j;
            for(j = 0; !(crnt->comm[j]); j++)
            {
                free(crnt->comm[j]);
            }
        }
        free(crnt);
        free(hd);

        exit(EXIT_SUCCESS);
    }
#endif
}

int main(void)
{
    char *args[MAX_CMD_ARGS];
    bool isBackground;
    int tokenCnt, cmdIdx;

    while(1)
    {
        isBackground = false;
        tokenCnt = parseLine(COMMAND_PROMPT, args, MAX_CMD_ARGS, &isBackground);

        if(tokenCnt > 0)
        {
            cmdIdx = scanTable(args[0]);

            if(cmdIdx < 0)
            {
                printf("Command not supported.\n");
            } else
            {
                if(commandTable[cmdIdx].genericFunc)
                {
                    commandTable[cmdIdx].genericFunc(cmdIdx, args, tokenCnt - 1);
                } else
                {
                    printf("Command not supported.\n");
                }
            }
        }
    }

#if 0

    char *args[20];
    int bg;

    //variables for forked child process
    pid_t pid;
    int check;

    //variables for parent process
    int status;

    //variables for daemon processes: ie background processes
    pid_t pid_2, sid;
    int init = 0;

    struct job
    {
        pid_t proc_id;
        char *comm[20];
        int job_id;
        struct job *nxt;
    };
    typedef struct job jb;
    jb *crnt, *hd = NULL;
    int status_2;

    //variables for supporting built-in functionalities
    char *bi_1 = "history";
    char *bi_2 = "cd";
    char *bi_3 = "pwd";
    char *bi_4 = "exit";
    char *bi_5 = "fg";
    char *bi_6 = "jobs";

    //variables for supporting history functionalities
    struct history
    {
        char *values[20];
        struct history *next;
    };
    typedef struct history hist;
    hist *current, *head = NULL, *temp;
    int count = 0;

    while(1)
    {
        bg = 0;
        int cnt = getcmd("\n[aditya.saha-260453165] ", args, &bg);

        //if no argument is entered: ie just the new line entered
        if(cnt != 0)
        {

            //check if background is enabled or not
            if(bg == 0)
            {
                //check if built-in function is called: history buffered not incremented
                if((strcmp(args[0], bi_1) == 0) || (strcmp(args[0], bi_2) == 0)
                    || (strcmp(args[0], bi_3) == 0) || (strcmp(args[0], bi_4) == 0)
                    || (strcmp(args[0], bi_5) == 0) || (strcmp(args[0], bi_6) == 0))
                {

                    //if "history" is typed
                    if(strcmp(args[0], bi_1) == 0)
                    {
                        int counter = 0;
                        current = head;
                        while(current != NULL)
                        {
                            int smh;
                            printf("%d: ", counter);
                            for(smh = 0; ((current->values[smh]) != NULL); smh++)
                            {
                                printf("%s ", current->values[smh]);
                            }
                            printf("\n");
                            counter++;
                            current = current->next;
                        }
                        printf(
                            "@bug_alert: commands run in the background is not shown in history\n");
                        printf("@bug_alert: built-in commands run isn't shown in history\n");
                    }

                    //if "cd" is typed
                    else if(strcmp(args[0], bi_2) == 0)
                    {
                        int chk;
                        chk = chdir(args[1]);
                        if(chk != 0)
                        {
                            printf("Failed to execute chdir!\n");
                        }
                    }

                    //if "pwd" is typed
                    else if(strcmp(args[0], bi_3) == 0)
                    {
                        char cwd[1024], *chk;
                        chk = getcwd(cwd, sizeof(cwd));
                        if(chk == NULL)
                        {
                            printf("Failed to execute getcwd!\n");
                        } else
                        {
                            printf("Current working dir: %s\n", cwd);
                        }
                    }

                    //if "jobs" is typed
                    else if((strcmp(args[0], bi_6) == 0))
                    {
                        printf("Job ID\tPID\tCommand\n");
                        crnt = hd;
                        while(crnt != NULL)
                        {
                            printf(" %d\t", crnt->job_id);
                            printf("%d\t", crnt->proc_id);
                            int i;
                            for(i = 0; (crnt->comm[i] != NULL); i++)
                            {
                                printf("%s ", crnt->comm[i]);
                            }
                            printf("\n");
                            crnt = crnt->nxt;
                            // printf("\n");
                        }
                        printf("@info: jobs are printed from latest to oldest top-down\n");
                    }

                    //if "fg" is typed
                    else if((strcmp(args[0], bi_5) == 0))
                    {
                        if(args[1] == NULL)
                        {
                            printf("@usage: fg <job_id>\n");
                            printf("type jobs to get the job ID\n");
                        } else
                        {
                            crnt = hd;
                            while(crnt != NULL)
                            {
                                if((crnt->job_id) == (args[1][0] - '0'))
                                {
                                    waitpid((crnt->proc_id), &status_2, 0);
                                    break;
                                }

                                crnt = crnt->nxt;
                            }
                        }
                    }

                    //if "exit" is typed
                    else
                    {
                        printf("Shell is exiting...\n");

                        //de-allocate history story
                        current = head;
                        while(current != NULL)
                        {
                            int i;
                            for(i = 0; !(current->values[i]); i++)
                            {
                                free(current->values[i]);
                            }
                            current = current->next;
                        }
                        free(current);
                        free(head);

                        //de-allocate job struct
                        crnt = hd;
                        while(crnt != NULL)
                        {
                            int j;
                            for(j = 0; !(crnt->comm[j]); j++)
                            {
                                free(crnt->comm[j]);
                            }
                        }
                        free(crnt);
                        free(hd);

                        exit(EXIT_SUCCESS);
                    }

                }

                //if built-in function is not called: history buffer is incremented
                else
                {

                    //fork a child to execute commands that are not built-in
                    pid = fork();

                    //if child failed to spawn
                    if(pid < 0)
                    {
                        printf("child failed to spawn!");
                        exit(EXIT_FAILURE);
                    }

                    //if child process is active
                    else if(pid == 0)
                    {

                        //if history command is not executed: ie the first string is not "r"
                        if(strcmp(args[0], "r") != 0)
                        {
                            check = execvp(*args, args);
                            if(check == -1)
                            {
                                printf("execvp command failed to execute!");
                                exit(EXIT_FAILURE);
                            }
                            exit(EXIT_SUCCESS);
                        }

                        //if history command is executed: ie the first string is "r"
                        else
                        {

                            //if no letter follows "r"
                            if((args[1]) == NULL)
                            {
                                int i;
                                printf("Most recent command found: ");
                                current = head;
                                for(i = 0; ((current->values[i]) != NULL); i++)
                                {
                                    printf("%s ", current->values[i]);
                                }
                                printf("\n");

                                int x;
                                x = execvp((current->values[0]), (current->values));
                                if(x == -1)
                                {
                                    printf("execvp command failed to execute\n");
                                    exit(EXIT_FAILURE);
                                }
                                exit(EXIT_SUCCESS);
                            }

                            //if a letter follows "r"
                            else
                            {
                                current = head;
                                while(current != NULL)
                                {
                                    if(*args[1] == *(current->values[0]))
                                    {
                                        int i;
                                        printf("Most recent command found: ");
                                        for(i = 0; ((current->values[i]) != NULL); i++)
                                        {
                                            printf("%s ", current->values[i]);
                                        }
                                        printf("\n");

                                        execvp(*(current->values), current->values);
                                        exit(EXIT_SUCCESS);
                                    }
                                    current = current->next;
                                }
                                printf("No commands found in history!\n");
                                exit(EXIT_FAILURE);
                            }

                        }

                    }

                    //if parent process is active
                    else
                    {

                        //parent waits until child finishes execution
                        waitpid(pid, &status, 0);
                        // printf("Status code is %d\n", endId);

                        //adds node to the history buffer upon successful command execution
                        //stores the latest typed-in commands
                        if(status != 256 && (strcmp(args[0], "r")) != 0)
                        {
                            current = (hist *) malloc(sizeof(hist));
                            int j;
                            for(j = 0; j < cnt; j++)
                            {
                                current->values[j] = args[j];
                            }

                            current->values[j] = NULL;
                            current->next = head;
                            head = current;
                            count++;

                            //<--debugger code-->
                            // current = head;
                            // while(current != NULL){
                            //  int i;
                            //  for(i = 0; i < count; i++){
                            //      printf("iterator %d: %s\n", i, current->values[i]);
                            //  }
                            //  printf("\n");
                            //  current = current->next;
                            // }

                        }

                        //adds node to the history buffer upon successful command execution
                        //stores the latest commands called from history
                        else if(status != 256 && (strcmp(args[0], "r")) == 0)
                        {

                            //if no parameter follows 'r'
                            if((args[1]) == NULL)
                            {
                                current = (hist *) malloc(sizeof(hist));
                                int m;
                                for(m = 0; (head->values[m]) != NULL; m++)
                                {
                                    current->values[m] = head->values[m];
                                }
                                current->values[m] = NULL;
                                current->next = head;
                                head = current;
                                count++;

                                //<--debugger code-->
                                // printf("Here goes:\n");
                                // current = head;
                                // while(current != NULL){
                                //  int i;
                                //  for(i = 0; i < count; i++){
                                //      printf("iterator %d: %s\n", i, current->values[i]);
                                //  }
                                //  printf("\n");
                                //  current = current->next;
                                // }

                            }

                            //if 'r' is followed by a parameter
                            else
                            {

                                temp = head;
                                while(temp != NULL)
                                {
                                    if(*(temp->values[0]) == *(args[1]))
                                    {
                                        current = (hist *) malloc(sizeof(hist));
                                        int i;
                                        for(i = 0; (temp->values[i]) != NULL; i++)
                                        {
                                            current->values[i] = temp->values[i];
                                        }
                                        current->values[i] = NULL;
                                        current->next = head;
                                        head = current;
                                    }
                                    temp = temp->next;
                                }

                                //<--debugger code-->
                                // printf("Here goes:\n");
                                // current = head;
                                // while(current != NULL){
                                //  int i;
                                //  for(i = 0; i < count; i++){
                                //      printf("iterator %d: %s\n", i, current->values[i]);
                                //  }
                                //  printf("\n");
                                //  current = current->next;
                                // }

                            }
                        }
                    }
                }
            }

            //background is enabled
            else
            {

                pid_2 = fork();

                if(pid_2 < 0)
                {
                    printf("Failed to spawn child!\n");
                    exit(EXIT_FAILURE);
                }

                else if(pid_2 == 0)
                {
                    sid = setsid();

                    if(sid < 0)
                    {
                        printf("Failed to create a new session!\n");
                        exit(EXIT_FAILURE);
                    }

                    close(STDIN_FILENO);
                    close(STDOUT_FILENO);
                    close(STDERR_FILENO);

                    execvp(*args, args);
                }

                // printf("The pid of the background process is %d: \n", pid_2);

                crnt = (jb *) malloc(sizeof(jb));
                int j;
                for(j = 0; j < cnt; j++)
                {
                    crnt->comm[j] = args[j];
                }
                crnt->proc_id = pid_2;
                crnt->job_id = init++;
                crnt->nxt = hd;
                hd = crnt;

                //<--debugger code-->
                // printf("Here goes:\n");
                // crnt = hd;
                // while(crnt != NULL){
                //  int i;
                //  for(i = 0; (crnt->comm[i] != NULL); i++){
                //      printf("%s ", crnt->comm[i]);
                //  }
                //  printf("\n");
                //  printf("Process id: %d\n", crnt->proc_id);
                //  printf("Job id: %d\n", crnt->job_id);
                //  crnt = crnt->nxt;
                //  printf("\n");
                // }
            }
        }

    }

#endif

    return 0;
}
