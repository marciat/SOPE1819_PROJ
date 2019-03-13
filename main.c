/* 
    Main program that will be called from the terminal
*/

#include <stdio.h>

int main(int argc, char* argv[], char* envp[]){
    if(argc < 2) {
        printf("Error: program needs at least 1 parameter...\n");
        return 1;
    }

    return 0;
}
