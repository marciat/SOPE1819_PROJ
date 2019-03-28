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
#include <time.h>
#include <dirent.h>

#include "forensic.h"

int main(int argc, char *argv[], char *envp[])
{
    setbuf(stdout, NULL);

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

    if (args.arg_h)
    {
        if (args.h_args[0] == NULL)
        {
            perror("-h flag requires arguments!!!\n");
            exit(1);
        }
    }

    struct stat directory_stat;
    if ((stat(args.f_or_dir, &directory_stat) >= 0) && S_ISDIR(directory_stat.st_mode))
    { //Found directory
        DIR *dir;
        struct dirent *ent;
        if ((dir = opendir(args.f_or_dir)) != NULL)
        {
            while ((ent = readdir(dir)) != NULL)
            {
                printf("%s\n", ent->d_name);
            }
            closedir(dir);
        }

        exit(0);
    }
    else
    { //Found file
        //Call program to read file data
    }

    //Read File Type
    int fd1 = open("temp_file.txt", O_RDWR | O_CREAT, 0777);
    if (fd1 < 0)
    {
        perror("open");
        exit(-1);
    }

    FILE *fp = fdopen(fd1, "r");
    if (fp == NULL)
    {
        perror("fdopen");
        exit(-1);
    }

    char *file_string = malloc(255 * sizeof(char));
    sprintf(file_string, "file %s > temp_file.txt", args.f_or_dir);
    system(file_string);
    memset(file_string, '\0', sizeof(file_string) * sizeof(char));
    fgets(file_string, 255, fp);
    fclose(fp);
    close(fd1);
    
    char *file_string_result = strstr(file_string, " ") + 1;

    size_t file_string_len = strlen(file_string_result);
    file_string_result[file_string_len - 1] = '\0';

    //Read File Data
    char *path_string = malloc(255 * sizeof(char));
    struct stat statbuf;
    sprintf(path_string, "%s", args.f_or_dir);
    if (stat(path_string, &statbuf) < 0)
    {
        exit(1);
    }
    free(path_string);

    char file_access_owner[3];
    time_t file_modification_date;
    time_t file_access_date;
    int file_size = 0;
    for (unsigned int i = 0; i < 3; i++)
    {
        file_access_owner[i] = '\0';
    }

    file_size = statbuf.st_size;

    if (statbuf.st_mode & S_IRUSR)
    {
        file_access_owner[0] = 'r';
    }

    if (statbuf.st_mode & S_IWUSR)
    {
        if (file_access_owner[0] == '\0')
        {
            file_access_owner[0] = 'w';
        }
        else
        {
            file_access_owner[1] = 'w';
        }
    }

    if (statbuf.st_mode & S_IXUSR)
    {
        if (file_access_owner[0] == '\0')
        {
            file_access_owner[0] = 'x';
        }
        else if (file_access_owner[1] == '\0')
        {
            file_access_owner[1] = 'x';
        }
        else
        {
            file_access_owner[2] = 'x';
        }
    }

    file_access_date = statbuf.st_atime;
    file_modification_date = statbuf.st_mtime;

    struct tm *access_info;
    struct tm *modification_info;
    char accessDate[20];
    char modificationDate[20];
    access_info = localtime(&file_access_date);
    modification_info = localtime(&file_modification_date);

    strftime(accessDate, 20, "%Y-%m-%dT%H:%M:%S", access_info);
    strftime(modificationDate, 20, "%Y-%m-%dT%H:%M:%S", modification_info);

    char *info_to_write = malloc(1000 * sizeof(char));

    sprintf(info_to_write, "%s,%s,%d,%s,%s,%s", args.f_or_dir, file_string_result, file_size, file_access_owner, accessDate, modificationDate);
    free(file_string);
    //Calculate file fingerprints
    if (args.arg_h)
    {
        for (unsigned int i = 0; i < 3; i++)
        {
            if (args.h_args[i] != NULL)
            {
                fd1 = open("temp_file.txt", O_RDWR, 0777);
                fp = fdopen(fd1, "r");
                char *h_string = malloc(255 * sizeof(char));
                char *tmp_string = malloc(25 * sizeof(char));
                sprintf(h_string, "%ssum %s > temp_file.txt", args.h_args[i], args.f_or_dir);
                system(h_string);
                h_string = malloc(255 * sizeof(char));
                fgets(h_string, 255, fp);
                sscanf(h_string, "%s %s", h_string, tmp_string);
                sprintf(info_to_write + strlen(info_to_write), ",%s", h_string);
                free(h_string);
                free(tmp_string);
                fclose(fp);
                close(fd1);
            }
        }
    }

    sprintf(info_to_write + strlen(info_to_write), "\n");

    if (args.arg_o)
    {
        int fd_o = open(args.outfile, O_RDWR, 0777);
        write(fd_o, info_to_write, strlen(info_to_write));
    }
    else
        write(STDOUT_FILENO, info_to_write, strlen(info_to_write));

    system("rm temp_file.txt");
    free(info_to_write);

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
            if (auxiliar_string != NULL)
            {
                strcpy(h_arg, auxiliar_string);
            }
            else
            {
                h_arg = NULL;
            }

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
            if (auxiliar_string != NULL)
            {
                h_arg = strcpy(h_arg, auxiliar_string);
            }
            else
            {
                h_arg = NULL;
            }

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
            if (auxiliar_string != NULL)
            {
                write(STDOUT_FILENO, "OLA\n", 5);
                h_arg = strcpy(h_arg, auxiliar_string);
            }
            else
            {
                h_arg = NULL;
            }

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
