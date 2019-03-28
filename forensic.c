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

    char* file_or_dir = argv[argc-1];

    struct stat directory_stat;
    if ((stat(file_or_dir, &directory_stat) >= 0) && S_ISDIR(directory_stat.st_mode))
    { //Found directory
        DIR *dir;
        struct dirent *ent;
        if ((dir = opendir(file_or_dir)) != NULL)
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
        execvp("process_data", argv);
        //Call program to read file data
    }

    
    return 0;
}

