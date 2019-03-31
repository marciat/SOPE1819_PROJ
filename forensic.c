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

    fore_args arguments = parse_data(argc, argv, envp);

    struct stat directory_stat;
    if ((stat(arguments.f_or_dir, &directory_stat) >= 0) && S_ISDIR(directory_stat.st_mode)) //Found directory
    { 
        DIR *dir;
        struct dirent *ent;
        if ((dir = opendir(arguments.f_or_dir)) != NULL)
        {
            char *originalDirectory = malloc(strlen(argv[argc - 1]));
            strcpy(originalDirectory, argv[argc - 1]);

            while ((ent = readdir(dir)) != NULL)
            {
                if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
                    continue;

                char *new_name = malloc(sizeof(arguments.f_or_dir) + sizeof(ent->d_name) + 1);
                sprintf(new_name, "%s/%s", arguments.f_or_dir, ent->d_name);

                int ret_value = stat(new_name, &directory_stat);
                if (ret_value != 0)
                {
                    perror("stat");
                }

                if (S_ISDIR(directory_stat.st_mode))
                {
                    if (arguments.arg_r) //Calling forensic recursively if -r is specified in the arguments
                    {
                        if (fork() == 0)
                        {
                            strcpy(argv[argc - 1], new_name); //New directory name
                            execvp("forensic", argv);
                        }
                    }
                }
                else //File in directory
                { 

                    strcpy(argv[argc - 1], new_name); //New file name

                    process_data(arguments); //Calling process_data for the new file
                }

                strcpy(argv[argc - 1], originalDirectory);

                free(new_name);
            }

            closedir(dir); //Closing directory
        }
    }
    else
    {                                //Found file
        if (process_data(arguments)) //Process data for just one file
        {
            printf("ERROR!!!");
            exit(1);
        }
    }

    return 0;
}
