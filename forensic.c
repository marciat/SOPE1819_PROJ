// Main program that will be called from the terminal

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

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

    fore_args args = get_programs_to_execute(argc, argv, envp);

    //Read File Type
    int fd1 = open("temp_file.txt", O_RDWR, 0777);
    FILE *fp = fdopen(fd1, "r");
    char *file_string = malloc(255 * sizeof(char));
    sprintf(file_string, "file %s > temp_file.txt", args.f_or_dir);
    system(file_string);
    free(file_string);
    file_string = malloc(255 * sizeof(char));
    fgets(file_string, 255, fp);
    file_string = strstr(file_string, " ") + 1;
    fclose(fp);
    close(fd1);

    file_string[strcspn(file_string, "\n")] = '\0';

    //Read File Data
    fd1 = open("temp_file.txt", O_RDWR, 0777);
    fp = fdopen(fd1, "r");
    char *lsl_string = malloc(255 * sizeof(char));
    sprintf(lsl_string, "ls -l %s > temp_file.txt", args.f_or_dir);
    system(lsl_string);
    free(lsl_string);
    lsl_string = malloc(255 * sizeof(char));
    fgets(lsl_string, 255, fp); //Get ls -l string from file

    char *file_access_owner = malloc(3 * sizeof(char));

    for (unsigned i = 0, j = 0; i < 3; i++, j++)
    {
        if (lsl_string[i] == '-')
        {
            j--;
            continue;
        }
        file_access_owner[j] = lsl_string[i];
    }
    lsl_string = strstr(lsl_string, " ") + 1;
    lsl_string = strstr(lsl_string, " ") + 1;
    lsl_string = strstr(lsl_string, " ") + 1;
    lsl_string = strstr(lsl_string, " ") + 1;

    int file_size = 0;
    sscanf(lsl_string, "%d", &file_size);
    lsl_string = strstr(lsl_string, " ") + 1;

    char *file_modification_date = malloc(12 * sizeof(char));
    for (unsigned i = 0; i < 12; i++)
    {
        file_modification_date[i] = lsl_string[i];
    }

    fclose(fp);
    close(fd1);

    printf("%s,%s,%d,%s,%s", args.f_or_dir, file_string, file_size, file_access_owner, file_modification_date);

    //Calculate file fingerprints
    if (args.arg_h)
    {
        unsigned i = 0;
        while (args.h_args[i] != NULL || i < 3)
        {
            fd1 = open("temp_file.txt", O_RDWR, 0777);
            fp = fdopen(fd1, "r");
            char *h_string = malloc(255 * sizeof(char));
            char* tmp_string = malloc(25*sizeof(char));
            sprintf(h_string, "%ssum %s > temp_file.txt", args.h_args[i], args.f_or_dir);
            system(h_string);
            h_string = malloc(255 * sizeof(char));
            fgets(h_string, 255, fp);
            sscanf(h_string, "%s %s", h_string, tmp_string);
            printf(",%s", h_string);
            free(h_string);
            free(tmp_string);
            fclose(fp);
            close(fd1);
            i++;
        }
    }
    printf("\n");
    return 0;
}

fore_args get_programs_to_execute(int argc, char *argv[], char *envp[])
{
    fore_args args;
    args.arg_r = false;
    args.arg_h = false;
    for (int i = 0; i < 3; i++)
    {
        args.h_args[i] = NULL;
    }
    args.arg_o = false;
    args.outfile = NULL;
    args.arg_v = false;
    args.logfilename = NULL;
    args.f_or_dir = NULL;

    for (int i = 1; i < argc - 1; i++)
    {
        if (strcmp(argv[i], "-r") == 0)
        {
            args.arg_r = true;
            continue;
        }
        else if (strcmp(argv[i], "-h") == 0)
        {
            args.arg_h = true;
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
            args.arg_o = true;
            i++;
            args.outfile = argv[i];
            continue;
        }
        else if (strcmp(argv[i], "-v") == 0)
        {
            args.arg_v = true;
            args.logfilename = get_filename_var(envp);
            continue;
        }
    }

    args.f_or_dir = argv[argc - 1];

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
