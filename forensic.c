// Main program that will be called from the terminal

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "forensic.h"

int main(int argc, char *argv[], char *envp[])
{
    if (argc < 2)
    {
        printf("Error: program needs at least 1 parameter...\n");
        return 1;
    }
    else if (argc > 8)
    {
        printf("Error: program can't take %d parameters...\n", argc);
        return 1;
    }

    fore_args args = get_programs_to_execute(argc, argv);
    
    char *logfilename;
    if (args.arg_v)
        logfilename = get_filename_var(envp);
    else
        logfilename = NULL;


    free(logfilename);
    return 0;
}

fore_args get_programs_to_execute(int argc, char *argv[])
{
    fore_args args;

    return args;
}

char *get_filename_var(char *envp[])
{
    unsigned i = 0;

    while (envp[i] != NULL)
    {
        if (strncmp("LOGFILENAME=", envp[i], 12) == 0)
            break;

        i++;
    }
    char *logfilename;
    logfilename = (char *)malloc(25 * sizeof(char));
    sprintf(logfilename, "%s", envp[i] + 5);
    return logfilename;
}