// Main program that will be called from the terminal

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
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
    args.arg_r = false;
    args.arg_h = false;
    for(int i = 0; i < 3; i++){
        args.h_args[i] = NULL;
    }
    args.arg_o = false;
    args.outfile = NULL;
    args.arg_v = false;
    args.f_or_dir = NULL;

    for (int i = 1; i < argc - 1; i++)
    {
        if (strcmp(argv[i], "-r") == 0)
        {
            args.arg_r = 1;
            continue;
        }
        else if (strcmp(argv[i], "-h") == 0)
        {
            args.arg_h = 1;
            i++;

            char *h_arg = (char *)malloc(20);
            char *auxiliar_string;

            auxiliar_string = strstr(argv[i], "md5");
            strcpy(h_arg, auxiliar_string);

            if (h_arg != NULL)
            {
                if (h_arg[3] != ',' && h_arg[3] != '\0')
                {
                    printf("Invalid argument!\n");
                    exit(1);
                }

                args.h_args[0] = "md5";
            }

            auxiliar_string = strstr(argv[i], "sha1");
            strcpy(h_arg, auxiliar_string);

            if (h_arg != NULL)
            {
                if (h_arg[4] != ',' && h_arg[4] != '\0')
                {
                    printf("Invalid argument!\n");
                    exit(1);
                }

                if (args.h_args[0] != NULL)
                {
                    args.h_args[1] = "sha1";
                }
                else
                {
                    args.h_args[0] = "sha1";
                }
            }

            auxiliar_string = strstr(argv[i], "sha256");
            strcpy(h_arg, auxiliar_string);

            if (h_arg != NULL)
            {
                if (h_arg[6] != ',' && h_arg[6] != '\0')
                {
                    printf("Invalid argument!\n");
                    exit(1);
                }

                if (args.h_args[0] != NULL)
                {
                    if (args.h_args[1] != NULL)
                    {
                        args.h_args[2] = "sha256";
                    }
                    else
                    {
                        args.h_args[1] = "sha256";
                    }
                }
                else
                {
                    args.h_args[0] = "sha256";
                }
            }

            free(h_arg);

            continue;
        }
        else if (strcmp(argv[i], "-o") == 0)
        {
            args.arg_o = 1;
            i++;
            args.outfile = argv[i];
            continue;
        }
        else if (strcmp(argv[i], "-v") == 0)
        {
            args.arg_v = 1;
            continue;
        }
    }

    args.f_or_dir = argv[argc - 1];

    /*printf("%d\n", args.arg_r);
    printf("%d\n", args.arg_h);
    if (args.h_args[0] != NULL)
        printf("%s\n", args.h_args[0]);
    if (args.h_args[1] != NULL)
        printf("%s\n", args.h_args[1]);
    if (args.h_args[2] != NULL)
        printf("%s\n", args.h_args[2]);
    printf("%d\n", args.arg_o);
    printf("%s\n", args.outfile);
    printf("%d\n", args.arg_v);
    printf("%s\n", args.f_or_dir);*/

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