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

    char *file_or_dir = argv[argc - 1];

    struct stat directory_stat;
    if ((stat(file_or_dir, &directory_stat) >= 0) && S_ISDIR(directory_stat.st_mode))
    { //Found directory
        DIR *dir;
        struct dirent *ent;
        if ((dir = opendir(file_or_dir)) != NULL)
        {
            while ((ent = readdir(dir)) != NULL)
            {
                if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
                    continue;

                if ((stat(ent->d_name, &directory_stat) >= 0) && S_ISDIR(directory_stat.st_mode))
                {
                    printf("%s\n", ent->d_name);
                }
                else
                { //File in directory
                
                    char** tmp_argv = malloc(argc); //New pointer for forensic arguments
                    for(int i = 0; i < argc; i++){
                        tmp_argv[i] = malloc(sizeof(argv[i]));
                        strcpy(tmp_argv[i], argv[i]);
                    }
                    memset(tmp_argv[argc-1], '\0', sizeof(argv[argc-1])*sizeof(char)); //Clearing the field of tmp_argv with the name of the file

                    char* file_name = malloc(sizeof(file_or_dir)+sizeof(ent->d_name));
                    sprintf(file_name, "%s/%s", file_or_dir, ent->d_name);
                    strcpy(argv[argc-1], file_name); //New file name
                    free(file_name);
                    
                    process_data(argc, tmp_argv, envp); //Calling process_data for the new file

                    for(int i = 0; i < argc; i++){ //Free memory of tmp_argv
                        free(tmp_argv[i]);
                    }
                    free(tmp_argv);
                }
            }
            closedir(dir); //Closing directory
        }
    }
    else
    { //Found file
        if (process_data(argc, argv, envp)) //Process data for just one file
        {
            printf("ERROR!!!");
            exit(1);
        }
    }

    return 0;
}
