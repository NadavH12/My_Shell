#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#define PATH_MAX        4096    /* max chars in a path name including null */


//Function definitions
void mysh();
void print_prompt();
char* get_command();
char* strip_newline(char command[]);
void pwd();
void change_dir(char command[]);
void external_commands(char command[]);
char** tokenize_command(char command[]);


int main(){
    mysh();
    return 0;
}


//run shell
void mysh(){

    while (1){

        //print prompt and get user command 
        print_prompt();
        char* command = get_command();

        //if user input unsuccessful, try again
        if (strcmp(command, "") == 0)
            continue;
        
        //built-in commands
        //exit
        else if (strcmp(command, "exit") == 0)
            exit(0);
        
        //pwd
        else if (strcmp(command, "pwd") == 0) 
            pwd();
        
        //cd
        else if (sizeof(command) > 3 && command[0] == 'c' && command[1] == 'd' && command[2] == ' ') 
            change_dir(command);
        
        //external commands
        else 
            external_commands(command);
    }
}


//print prompt
void print_prompt(){
    char cwd[PATH_MAX];
    getcwd(cwd, sizeof(cwd));
    printf("%s", cwd);
    printf("%s", "$mysh> ");
}


//method to get user input
char* get_command(){
    //get command from console
    char command[257];
    fgets(command, 257, stdin);

    //handle fgets() failure
    if (command == NULL){
        fprintf(stderr, "%s", "Failed to read command\n");
        return "";
    }

    //strip newline if needed
    char* stripped_command = strip_newline(command);
    return stripped_command;
}



//method to strip newlines from user input
char* strip_newline(char command[]){
    int newline_index = -1;

    //get index of newline
    for (int i = 0; i < strlen(command); i++){
        char element = command[i];
        if (element == '\n'){
            newline_index = i;
            break;
        }
    }

    //if no newline to strip, return original string
    if (newline_index == -1){
        return command;
    }

    //Need to return local variable (stripped_newline[])
    //So must allocate memory for it on the heap and return a pointer to it
    char* stripped_command_ptr = malloc(sizeof(char) * newline_index);


    //copy substring of command string up until newline to remove the newline
    //into block of memory we allocated for the return value
    for (int i = 0; i < newline_index; i++){
        stripped_command_ptr[i] = command[i];
    }

    return stripped_command_ptr;
}


//print working directory
void pwd(){
    char cwd[PATH_MAX];
    getcwd(cwd, sizeof(cwd));
    printf("%s", cwd);
    printf("\n");

}


//cd
void change_dir(char command[]) {

    //cutting off 'cd ' from beginning of command to get wanted path
    char new_dir[strlen(command) - 3];
    for (int i = 0; i < sizeof(new_dir); i++){
        new_dir[i] = command[i+3];
    }

    //change dir & handle error cases
    int success = chdir(new_dir);
    if (success != 0){
        if (errno == ENOENT){
            fprintf(stderr, "%s", "No such directory\n");
        } else {
            fprintf(stderr, "%s", "Change directory failed\n");
        }
    }
}



//tokenize user input, using ' ' as delimiter
char** tokenize_command(char command[]){
    //Get max length of given command
    int command_max_length = strlen(command);
    
    //Need to return local variable (char*[])
    //So must allocate memory for it on the heap and return a pointer to it
    char** myargs_ptr = malloc(sizeof(char) * command_max_length);

    //Tokenize command into array 
    char delim[1] = " ";
    char* tok = strtok(command, delim);
    int i = 0;
    while (tok != NULL){
        myargs_ptr[i] = tok;
        i = i + 1;
        tok = strtok(NULL, delim);
    }

    //Denote end of array
    myargs_ptr[i] = NULL;

    //strip newline if needed
    char* last_arg = myargs_ptr[i-1];
    last_arg = strip_newline(last_arg);
    myargs_ptr[i-1] = last_arg;
    
    return myargs_ptr;
}


//handle external commands
void external_commands(char command[]){

    //tokenize user input
    char** myargs_ptr = tokenize_command(command);

    //get external command name
    char* file_name = myargs_ptr[0];

    //handle extra failure cases
    if (file_name == NULL || myargs_ptr == NULL){
        return;
    }
    
    int fork_ret = fork();

    //fork() fail case
    if (fork_ret < 0){
        fprintf(stderr, "%s", "Failed to create process\n");
    }

    //new process
    else if (fork_ret == 0){
        execvp(file_name, myargs_ptr);

        //if execvp returns here, it means it failed, so print error message
        fprintf(stderr, "%s", "Failed to execute command\n");

        //need to kill child process otherwise we will create another process for each failed external command
        kill(getpid(), SIGINT);
        
    }

    //parent process waits for child process to complete
    else {
        wait(NULL);
    }
}