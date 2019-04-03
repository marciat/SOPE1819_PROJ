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
#include <signal.h>

#include "forensic.h"

void sigint_handler(int signo)
{
    printf("In SIGINT handler ...\n");
    (void)signo;
    system("pause");
}

int main(int argc, char *argv[], char *envp[])
{
    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);
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

    struct sigaction action;
    action.sa_handler = sigint_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    if (sigaction(SIGINT, &action, NULL) < 0)
    {
        perror("sigaction");
        exit(1);
    }

    fore_args arguments = parse_data(argc, argv, envp);

    //char *originalDirectory = malloc(strlen(argv[argc - 1]));
    //strcpy(originalDirectory, argv[argc - 1]);

	if(arguments.arg_v){
        struct timespec event;
    
	    char* event_desc = malloc(500 * sizeof(char));

	    sprintf(event_desc, "forensic ");
	    if(arguments.arg_r)
	        sprintf(event_desc + strlen(event_desc), "-r ");
	    if(arguments.arg_h){
	        sprintf(event_desc + strlen(event_desc), "-h ");
	        for(int i=0; i<3; i++){
	            if(arguments.h_args[i] == NULL)
	                break;
	            if(i == 0)
	                sprintf(event_desc + strlen(event_desc), "%s", arguments.h_args[i]);
	            else
	                sprintf(event_desc + strlen(event_desc), ",%s", arguments.h_args[i]);
	        }
	    }
	    if(arguments.arg_o)
	        sprintf(event_desc + strlen(event_desc), " -o ");
	    if(arguments.outfile != NULL)
	        sprintf(event_desc + strlen(event_desc), "%s", arguments.outfile);
	    sprintf(event_desc + strlen(event_desc), " -v ");
	    if(arguments.f_or_dir)
	        sprintf(event_desc + strlen(event_desc), "%s", arguments.f_or_dir);
 	    if(arguments.logfilename == NULL)
	    {
 	    	printf("LOGFILENAME variable not defined!!!\n");
	        exit(1);
	    }
    	int logfile = open(arguments.logfilename, O_RDWR | O_CREAT | O_APPEND, 0777);
    	if(logfile < 0)
		{
    		perror("open");
 	    	exit(-1);
 	   	}

        clock_gettime(CLOCK_MONOTONIC, &event);
    	write_to_logfile(logfile, (double)(event.tv_nsec-start.tv_nsec)/1000000000.0+(double)(event.tv_sec - start.tv_sec), getpid(), COMMAND, event_desc);
    	free(event_desc);
    	close(logfile);
	}

    if (forensic(arguments, start)) //, originalDirectory))
        return 1;

    //free(originalDirectory);

    free_arguments(&arguments);

    return 0;
}

int forensic(fore_args arguments, struct timespec start) //, char *originalDirectory)
{
    struct stat directory_stat;
    if ((stat(arguments.f_or_dir, &directory_stat) >= 0) && S_ISDIR(directory_stat.st_mode)) //Found directory
    {
        DIR *dir;
        struct dirent *ent;
        if ((dir = opendir(arguments.f_or_dir)) != NULL)
        {
            while ((ent = readdir(dir)) != NULL)
            {
                if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
                    continue;

                char *new_name = malloc(strlen(arguments.f_or_dir) + sizeof(ent->d_name) + 1);
                sprintf(new_name, "%s/%s", arguments.f_or_dir, ent->d_name);

                if (stat(new_name, &directory_stat) != 0)
                {
                    perror("stat");
                }

                if (S_ISDIR(directory_stat.st_mode))
                {
                    if (arguments.arg_r) //Calling forensic recursively if -r is specified in the arguments
                    {
                        if (fork() == 0)
                        {
                            strcpy(arguments.f_or_dir, new_name); //New directory name

                            if (forensic(arguments, start)) //, originalDirectory))
                                return 1;
                            break;
                        }
                    }
                }
                else //File in directory
                {
                    char *tmp_f_or_dir = malloc(strlen(arguments.f_or_dir));
                    strcpy(tmp_f_or_dir, arguments.f_or_dir);
                    strcpy(arguments.f_or_dir, new_name); //New file name

                    if (process_data(arguments, start)) //Calling process_data for the new file
                    {
                        printf("ERROR!!!");
                        exit(1);
                    }

                    strcpy(arguments.f_or_dir, tmp_f_or_dir);
                    free(tmp_f_or_dir);
                }

                free(new_name);
            }

            closedir(dir); //Closing directory
        }
    }
    else //Found file
    {
        if (process_data(arguments,start)) //Process data for just one file
        {
            printf("ERROR!!!");
            exit(1);
        }
    }

    return 0;
}
