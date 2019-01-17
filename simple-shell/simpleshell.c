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

/* Function definitions */
void execBuiltInFnc(int cmdIdx, char **args, int argsLen);

/* Structure definitions */
typedef void (*genericFuncPtr)(int cmdIdx, char **args, int argsLen);

typedef struct
{
    char *cmd;
    genericFuncPtr genericFunc;
    /* TODO: Need to expand for custom function execution */
} CommandTable_t;

struct history
{
    char *values[MAX_CMD_ARGS];
    int tokens;
    struct history *next;
};

typedef struct history History_t;
History_t *currHistory, *headHistory = NULL;

CommandTable_t commandTable[] =
{
    {"history", execBuiltInFnc},
    {"cd",      execBuiltInFnc},
    {"pwd",     execBuiltInFnc},
    {"exit",    execBuiltInFnc},
};

int updateHistory(char *args[], int tokens)
{
    currHistory = (History_t*) malloc(sizeof(History_t));
    if(currHistory)
    {
        for(int idx = 0; idx < tokens; idx++)
        {
            currHistory->values[idx] = args[idx];
        }

        currHistory->tokens = tokens;
        currHistory->next = headHistory;
        headHistory = currHistory;

        return 0;
    }

    /* Default return failure */
    return -1;
}

void clearHistory(void)
{
    while(headHistory)
    {
        currHistory = headHistory->next;
        free(headHistory);
        headHistory = currHistory;
    }
}

void dumpHistory(void)
{
    while(headHistory)
    {
        currHistory = headHistory->next;
        for(int idx = 0; idx < headHistory->tokens; idx++)
        {
            printf("%s ", headHistory->values[idx]);
        }

        printf("\n");
        headHistory = currHistory;
    }
}

int parseLine(char *prompt, char *args[], int argsLen, bool *isBackground)
{
    int length, argIdx = 0;
    char *token, *loc, *line;
    size_t lineCap = 0;

    /* Check for NULL pointer in passed arguments */
    CHECK_NULL(prompt)
    CHECK_NULL(isBackground)

    printf("(%s)$ ", prompt);
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
            dumpHistory();
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
            clearHistory();
            exit(EXIT_SUCCESS);
            break;
    }
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
            /* Scan cmd table and retrieve function call index */
            cmdIdx = scanTable(args[0]);

            if(cmdIdx < 0)
            {
                printf("Command not supported.\n");
            } else
            {
                /* Log in history buffer */
                if(updateHistory(args, tokenCnt) == 0)
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
    }

    return 0;
}
