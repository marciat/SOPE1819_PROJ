// Header file that declares every function and data structure used in the main function of forensic

typedef struct
{
    bool arg_r;        //Option -r of forensic program
    bool arg_h;        //Option -h of forensic program
    char *h_args[3];   //Option -h may have 3 arguments
    bool arg_o;        //Option -o of forensic program
    char *outfile;     //If -o has an argument, it will be stored here
    bool arg_v;        //Option -v of forensic program
    char* logfilename; //If -v is selected, this stores the value of LOGFILENAME variable
    char *f_or_dir;    //File or directory to be evaluated
} fore_args;

fore_args get_programs_to_execute(int argc, char* argv[], char* envp[]);

char* get_filename_var(char* envp[]);

void remove_newline(char* string);

int process_data(int argc, char* argv[], char* envp[]);