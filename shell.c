#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#include <limits.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/stat.h>
#include <dirent.h>
#include "util.h"

#define LIMIT 256 // max number of tokens for a command
#define MAXLINE 1024 // max number of characters from user input
#define min(a, b) a<b?a:b

/**
 * Method used to print the welcome screen of our shell
 */
void welcomeScreen() {
    printf("\n\t============================================\n");
    printf("\t             Shell made by:\n");
    printf("\t--------------------------------------------\n");
    printf("\t             Abhinav Gupta - 150123001\n");
    printf("\t             Hiten Sethiya - 150123015\n");
    printf("\t             Manas Daruka  - 150123020\n");
    printf("\t============================================\n");
    printf("\n\n");
}

/**
 * signal handler for SIGCHLD
 */
void signalHandler_child(int p) {
    /* Wait for all dead processes.
     * We use a non-blocking call (WNOHANG) to be sure this signal handler will not
     * block if a child was cleaned up in another part of the program. */
    while (waitpid(-1, NULL, WNOHANG) > 0) {
    }
    printf("\n");
}

/**
 * Signal handler for SIGINT
 */
void signalHandler_int(int p) {
    // We send a SIGTERM signal to the child process
    if (kill(pid, SIGTERM) == 0) {
        printf("\nProcess %d received a SIGINT signal\n", pid);
        no_reprint_prmpt = 1;
    } else {
        printf("\n");
    }
}

/**
 *	Displays the prompt for the shell
 */
void shellPrompt() {
    // We print the prompt in the form "<user>@<host> <cwd> >"
    char hostn[1204] = "";
    gethostname(hostn, sizeof(hostn));
    printf("%s > ", getcwd(currentDirectory, 1024));
}

/**
 * Method to change directory
 */
int changeDirectory(char *args[]) {
    // If we write no path (only 'cd'), then go to the home directory
    if (args[1] == NULL) {
        chdir(getenv("HOME"));
        return 1;
    }
        // Else we change the directory to the one specified by the
        // argument, if possible
    else {
        if (chdir(args[1]) == -1) {
            printf(" %s: no such directory\n", args[1]);
            return -1;
        }
    }
    return 0;
}

/**
* Method used to manage I/O redirection
*/
void fileIO(char *args[], char *inputFile, char *outputFile, int option) {


    int fileDescriptor; // between 0 and 19, describing the output or input file
    pid = fork();
    if (pid == -1) {
        printf("Child process could not be created\n");
        return;
    }
    int flag = 0;
    if (pid == 0) {
        // Option 0: output redirection
        if (option == 0) {
            // We open file for read only (it's STDIN)
            fileDescriptor = open(inputFile, O_RDONLY, 0600);
            if(fileDescriptor == -1)
                flag = 1;
            // We replace de standard input with the appropriate file
            dup2(fileDescriptor, STDIN_FILENO);
            close(fileDescriptor);
        } 
        else if (option == 1) {
            // We open (create) the file truncating it at 0, for write only
            fileDescriptor = open(outputFile, O_CREAT | O_TRUNC | O_WRONLY, 0600);
            if(fileDescriptor == -1)
                flag = 1;
            // We replace de standard output with the appropriate file
            dup2(fileDescriptor, STDOUT_FILENO);
            close(fileDescriptor);
            // Option 1: input and output redirection
        } 
        else if (option == 2) {
            // We open file for read only (it's STDIN)
            fileDescriptor = open(inputFile, O_RDONLY, 0600);
            if(fileDescriptor == -1)
                flag = 1;
            // We replace de standard input with the appropriate file
            dup2(fileDescriptor, STDIN_FILENO);
            close(fileDescriptor);
            // Same as before for the output file
            fileDescriptor = open(outputFile, O_CREAT | O_TRUNC | O_WRONLY, 0600);
            if(fileDescriptor == -1)
                flag = 1;
            dup2(fileDescriptor, STDOUT_FILENO);
            close(fileDescriptor);
        }

        setenv("parent", getcwd(currentDirectory, 1024), 1);
        
        if (flag == 1 || execvp(args[0], args) < 0) {
            printf("Wrong arguments/command\n");
            kill(getpid(), SIGTERM);
        }
    }
    waitpid(pid, NULL, 0);
}

void kill_child(int sig)
{
	kill(pid,SIGKILL);
}

void fileIO_limited(char *args[], char *inputFile, char *outputFile, int option, int m) {

    int fileDescriptor; // between 0 and 19, describing the output or input file
    signal(SIGALRM,(void (*)(int))kill_child);
    pid = fork();
    if (pid == -1) {
        printf("Child process could not be created\n");
        return;
    }
    int flag = 0;
    if (pid == 0) {
        // Option 0: output redirection
        if (option == 0) {
            // We open file for read only (it's STDIN)
            fileDescriptor = open(inputFile, O_RDONLY, 0600);
            if(fileDescriptor == -1)
                flag = 1;
            // We replace de standard input with the appropriate file
            dup2(fileDescriptor, STDIN_FILENO);
            close(fileDescriptor);
        } 
        else if (option == 1) {
            // We open (create) the file truncating it at 0, for write only
            fileDescriptor = open(outputFile, O_CREAT | O_TRUNC | O_WRONLY, 0600);
            if(fileDescriptor == -1)
                flag = 1;
            // We replace de standard output with the appropriate file
            dup2(fileDescriptor, STDOUT_FILENO);
            close(fileDescriptor);
            // Option 1: input and output redirection
        } 
        else if (option == 2) {
            // We open file for read only (it's STDIN)
            fileDescriptor = open(inputFile, O_RDONLY, 0600);
            if(fileDescriptor == -1)
                flag = 1;
            // We replace de standard input with the appropriate file
            dup2(fileDescriptor, STDIN_FILENO);
            close(fileDescriptor);
            // Same as before for the output file
            fileDescriptor = open(outputFile, O_CREAT | O_TRUNC | O_WRONLY, 0600);
            if(fileDescriptor == -1)
                flag = 1;
            dup2(fileDescriptor, STDOUT_FILENO);
            close(fileDescriptor);
        }

        setenv("parent", getcwd(currentDirectory, 1024), 1);
        if (flag == 1 || execvp(args[0], args) < 0) {
            printf("Wrong arguments/command\n");
            kill(getpid(), SIGTERM);
        }
    }
    alarm(m);
    waitpid(pid, NULL, 0);
}

/*
 * Remove file */
void removeFile(char *file) {
    int ret = remove(file);
    if (ret == 0) {
        printf("File deleted successfully\n");
    } else {
        printf("Error: unable to delete the file\n");
    }
}

int removeDirectory(const char *path) {
    DIR *d = opendir(path);
    size_t path_len = strlen(path);
    int r = -1;
    if (d) {
        struct dirent *p;
        r = 0;
        while (!r && (p = readdir(d))) {
            int r2 = -1;
            char *buf;
            size_t len;
            /* Skip the names "." and ".." as we don't want to recurse on them. */
            if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, "..")) {
                continue;
            }
            len = path_len + strlen(p->d_name) + 2;
            buf = malloc(len);
            if (buf) {
                struct stat statbuf;
                snprintf(buf, len, "%s/%s", path, p->d_name);
                if (!stat(buf, &statbuf)) {
                    if (S_ISDIR(statbuf.st_mode)) {
                        r2 = removeDirectory(buf);
                    } else {
                        r2 = unlink(buf);
                    }
                }
                free(buf);
            }
            r = r2;
        }
        closedir(d);
    }
    if (!r) {
        r = rmdir(path);
        printf("Directory Deleted Successfully\n");
    }
    return r;
}

/**
* Method used to handle the commands entered via the standard input
*/
int commandHandler(char *args[]) {

    // 'exit' command quits the shell
    if (strcmp(args[0], "exit") == 0) exit(0);
        // 'pwd' command prints the current directory
    else if (strcmp(args[0], "pwd") == 0) {
        printf("%s\n", getcwd(currentDirectory, 1024));
    }
        // 'clear' command clears the screen
    else if (strcmp(args[0], "clear") == 0) system("clear");
        // 'cd' command to change directory
    else if (strcmp(args[0], "cd") == 0) changeDirectory(args);
        // 'ls' command to list out contents of current directory
    else if (strcmp(args[0], "ls") == 0) {
        int count, i;
        struct dirent **files;
        char pathname[1000];
        getcwd(pathname, 1024);
        count = scandir(pathname, &files, NULL, alphasort);

        /* If no files found, make a non-selectable menu item */
        if (count <= 0) {
            perror("No files in this directory\n");
            return -1;
        }
        for (i = 0; i < count; ++i){
            if((files[i]->d_name)[0] != '.')
                printf("%s\n", files[i]->d_name);
        }
    }
        /*history n : Prints the most recent n commands issued by the numbers. If n is omitted, prints all commands issued by the user.*/
    else if (strcmp(args[0], "history") == 0) {
        if (args[1]) {
            int i = 0;
            while (i < (min(atoi(args[1]), number - 1))) {
                printf("%s", history[number - 2 - i]);
                i++;
            }
        } else {
            int i = 0;
            while (i < number - 1) {
                printf("%s", history[number - 2 - i]);
                i++;
            }
        }
    }
    else if (strcmp(args[0], "issue") == 0) {
        if(!args[1]){
            printf("Not enough input arguments\n");
            return -1;
        }
        char *tokens[LIMIT];
        int numTokens;
        int n=atoi(args[1]);
        char *line=history[number-1-n];
        tokens[0] = strtok(line, " \n\t");
        numTokens = 1;
        while ((tokens[numTokens] = strtok(NULL, " \n\t")) != NULL) numTokens++;
        commandHandler(tokens);
        printf("\n");
    }  
    else if (strcmp(args[0], "rm") == 0) {
        char tag ;
        char file[20][10000];
        struct stat path_stat;
        int i = 0;
        if (args[1][0] == '-') {
            if (args[1][1]) {
                // Acquire Option type
                tag = args[1][1];
                if (args[1][2] != '\0') {
                    printf("rm function can use only -f,-r or -v");
                    return -1;
                }
                //Acquire File Path
                if (strpbrk(args[2], "/")) {
                    while (args[i + 2] != NULL) {
                        strcat(file[i], args[i+2]);
                        i++;
                    }
                } else {
                    while (args[i + 2] != NULL) {
                        getcwd(file[i], 1000);
                        strcat(file[i], "/");
                        strcat(file[i], args[i+2]);
                        i++;
                    }
                }

                //Option Wise operations
                if (tag == 'r') {

                    for (int j = 0; j < i; j++) {
                        //determine information about a file based on its file path
                        if (stat(file[j], &path_stat) != 0) {
                            printf("error\n");
                            return -1;
                        }

                        if (S_ISDIR(path_stat.st_mode))removeDirectory(file[j]);
                        else {
                            printf("Error : Option -r requires path to be a directory.");
                            return -1;
                        }
                    }
                } else if (tag == 'f') {

                    for (int j = 0; j < i; j++) {
                        //determine information about a file based on its file path
                        if (stat(file[j], &path_stat) != 0) {
                            printf("error\n");
                            return -1;
                        }
                        if (!S_ISREG(path_stat.st_mode)) {
                            printf("Error : Option -f with rm requires path to be a file.");
                            return -1;
                        }
                        remove(file[j]);
                    }
                } else if (tag == 'v') {
                    for (int j = 0; j < i; j++) {
                        //determine information about a file based on its file path
                        if (stat(file[j], &path_stat) != 0) {
                            printf("error\n");
                            return -1;
                        }

                        if (!S_ISREG(path_stat.st_mode)) {
                            printf("Error : Option -v with rm requires path to be a file.");
                            return -1;
                        }
                        printf("%s Removed", file[j]);
                        removeFile(file[j]);
                    }
                } else {
                    printf("Error : Invalid Option Specified: rm function can use only -f,-r or -v");
                    return -1;
                };
            }
        }
            //if no option is specified
        else {
            while (args[i + 1] != NULL) {
                strcat(file[i], args[i+1]);
                i++;
            }
            for (int j = 0; j < i; j++) {
                //determine information about a file based on its file path
                if (stat(file[j], &path_stat) != 0) {
                    printf("error\n");
                    return -1;
                }

                if (S_ISREG(path_stat.st_mode))removeFile(file[j]);
                else {
                    printf("Error : Invalid Path");
                    return -1;
                };
            }
        }
    }
    else if (strcmp(args[0], "rmexcept") == 0) {
        int count, i,k=0,j,l=1,flag=0;
        struct dirent **files;
        char pathname[1000];
        getcwd(pathname, 1024);
        count = scandir(pathname, &files, NULL, alphasort);
        char file[20][10000] ;
        while (args[k + 1] != NULL){
            strcat(file[k], args[k+1]);
            //printf("%s\n",file[k]);
            k++;
        }
        /* If no files found, make a non-selectable menu item */
        if (count <= 0) {
            perror("No files in this directory\n");
            return -1;
        }
        char *args[LIMIT];
        args[0] = "rm";
        for (i = 2; i < count; ++i){
            flag=0;
            for(j = 0;j<k;j++){
                if(strcmp(files[i]->d_name,file[j])==0)flag=1;
            }
            if(flag==0){(args[l]=files[i]->d_name);l++;}
        }
        for(int j=0;j<l;j++)printf("%s\n",args[j]);
        commandHandler(args);

    }
    else {
        // If none of the preceding commands were used, we invoke the
        // specified program. We have to detect I/O redirection.
        if (args[1] == NULL){
            fileIO(args, NULL, NULL, 3);
        }
        else if(strcmp(args[1], "<") == 0){
            if(args[2] == NULL){
                printf("Not enough input arguments\n");
                return -1;
            }
            if(args[3] != NULL && strcmp(args[3], ">") == 0){
                if(args[4] == NULL){
                    printf("Not enough output arguments\n");
                    return -1;
                }
                fileIO(args, args[2], args[4], 2);
            }
            else{
                fileIO(args, args[2], NULL, 0);
            }
        }
        else if(strcmp(args[1], ">") == 0){
            if(args[2] == NULL){
                printf("Not enough output arguments\n");
                return -1;
            }
            fileIO(args, NULL, args[2], 1);
        }
        else{
            if (args[2] == NULL){
                fileIO_limited(args, NULL, NULL, 3, atoi(args[1]));
            }
            else if(strcmp(args[2], "<") == 0){
                if(args[3] == NULL){
                    printf("Not enough input arguments\n");
                    return -1;
                }
                if(args[4] != NULL && strcmp(args[4], ">") == 0){
                    if(args[5] == NULL){
                        printf("Not enough output arguments\n");
                        return -1;
                    }
                    fileIO_limited(args, args[3], args[5], 2, atoi(args[1]));
                }
                else{
                    fileIO_limited(args, args[3], NULL, 0, atoi(args[1]));
                }
            }
            else if(strcmp(args[2], ">") == 0){
                if(args[3] == NULL){
                    printf("Not enough output arguments\n");
                    return -1;
                }
                fileIO_limited(args, NULL, args[3], 1, atoi(args[1]));
            }
        }
    }
    return 1;
}


/**
* Main method of our shell
*/
int main(int argc, char *argv[], char **envp) {
    char line[MAXLINE]; // buffer for the user input
    char *tokens[LIMIT]; // array for the different tokens in the command
    int numTokens;

    pid = -10; // we initialize pid to an pid that is not possible

    // We call the method of initialization and the welcome screen
    //init();
    welcomeScreen();

    // We set our extern char** environ to the environment, so that
    // we can treat it later in other methods
    environ = envp;

    // We set shell=<pathname>/simple-c-shell as an environment variable for
    // the child
    setenv("shell", getcwd(currentDirectory, 1024), 1);

    // Main loop, where the user input will be read and the prompt
    // will be printed
    number = 0;
    while (TRUE) {
        // We print the shell prompt if necessary
        shellPrompt();
        // We empty the line buffer
        memset (line, '\0', MAXLINE);

        // We wait for user input
        fgets(line, MAXLINE, stdin);
        for (int i = 0; i < MAXLINE; i++) {
            history[number][i] = line[i];
        }
        // If nothing is written, the loop is executed again
        if ((tokens[0] = strtok(line, " \n\t")) == NULL) continue;
        number++;
        // We read all the tokens of the input and pass it to our
        // commandHandler as the argument
        numTokens = 1;
        
        while ((tokens[numTokens] = strtok(NULL, " \n\t")) != NULL) numTokens++;
        //printf("%d\n", numTokens);
        commandHandler(tokens);
        printf("\n");

    }
    exit(0);
}
