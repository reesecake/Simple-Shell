#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <stdbool.h>
#include <limits.h>
#include <sys/fcntl.h>
#define CMDLINE_MAX 512
#define ARGUMENT_MAX 16
#define TOKEN_MAX 32
#define BUFF_AMOUNT 100

struct Commands {
    char *file;
    char *args[16];
};
// passing in the tokens from "allCommands" and puts them into the Command struct.
void createCommands (struct Commands Command[ARGUMENT_MAX], char allCommands[ARGUMENT_MAX][TOKEN_MAX], int *commandAmount) {
    int i = 0;
    int currentCmd = 0;
    int buf = 0;
    while (allCommands[i][0] != '\0'){
        //checks to see if there is more one command by searching or a '|' and it to the struct..
        if (strcmp(allCommands[i], "|") == 0 || strcmp(allCommands[i], "|&") == 0){
            currentCmd++;
            buf = 0;
            *commandAmount = *commandAmount + 1;
        }
            //if there is no redirection operand, store it together.
        else if (strcmp(allCommands[i], ">") != 0 && strcmp(allCommands[i], ">&") != 0){
            if (buf == 0 ) {
                Command[currentCmd].file = allCommands[i];
                Command[currentCmd].args[buf] = allCommands[i];
            }
            else {
                Command[currentCmd].args[buf] = allCommands[i];
            }
            buf++;
        }
        i++;

    }
    //ending of args must be NULL for execvp.
    Command -> args[buf] = NULL;
}
//gives
char *commandSeparator(char *cmd, char allCommands[ARGUMENT_MAX][TOKEN_MAX], int *argCount)
{
    char *commandType = NULL;
    // stores all the commands that the user inputs.
    // is the current token.
    int cmdBuffer = 0;
    // is the current char of the cmd.
    int charBuffer = 0;
    for (int i = 0; i < strlen(cmd); i++){
        //deletes all white spaces from the command and stores it
        if (cmd[i] != ' ' && cmd[i] != '>' && (cmd[i] != '|' || cmd[i+1] != '&')) {
            allCommands[cmdBuffer][charBuffer] = cmd[i];
            charBuffer++;
        }
            //if the current command is not empty and there is a white space, add the white space (ex. tr 2 1)
        else if (cmd[i] == ' ' && allCommands[cmdBuffer][0] != '\0') {
            cmdBuffer++;
            charBuffer = 0;
            *argCount = *argCount +1;
        }
            // if there is an operand, store it.
        else if (cmd[i] == '>' || (cmd[i] == '|' && cmd[i+1] == '&')) {
            if (cmd[i-1] != ' '){
                cmdBuffer++;
                charBuffer = 0;
                *argCount = *argCount +1;
            }
            //store the redirection operand together with the previous.
            if (cmd[i+1] == '&'){
                if (cmd[i] == '|'){
                    commandType = "pipeRedirect";
                }
                else{
                    commandType = "double redirect";
                }
                allCommands[cmdBuffer][charBuffer] = cmd[i];
                charBuffer++;
                i++;
                allCommands[cmdBuffer][charBuffer] = cmd[i];
                cmdBuffer++;
                charBuffer=0;
            }
            else {
                allCommands[cmdBuffer][charBuffer] = cmd[i];
                commandType = "redirect";
                cmdBuffer++;
                charBuffer = 0;
            }

        }
    }
    return commandType;
}

static void pipeCommands(char ***commands)
{
    int fd[2];
    //holds previous fd[0].
    int fdBuffer = 0;
    pid_t pid;

    //multipiping using knowledge from slides.
    while (*commands != NULL) {
        pipe(fd);
        if ((pid = fork()) == -1) {
            perror("fork");
            exit(1);
        }
        else if (pid == 0) {
            dup2(fdBuffer, 0);
            if (*(commands + 1) != NULL) {
                dup2(fd[1], 1);
            }
            close(fd[0]);
            execvp((*commands)[0], *commands);
            exit(1);
        }
        else {
            wait(NULL); 		/* Collect childs */
            close(fd[1]);
            fdBuffer = fd[0];
            commands++;
        }
    }
}
void directoryStack(struct Commands Command[ARGUMENT_MAX], char *dirStack[BUFF_AMOUNT], int *stackSize, int bufferSize,
                    char buf1[BUFF_AMOUNT], char buf2[BUFF_AMOUNT]){
    char *getDir = getcwd(buf1,BUFF_AMOUNT);
    if (*stackSize == 0) {

        dirStack[*stackSize] = getDir;
        *stackSize = *stackSize + 1;
        bufferSize++;
    }
    // if input is to pwd
    if (strcmp(Command[0].file, "pwd") == 0) {
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s\n", cwd);
        }
        else {
            perror("getcwd() error");
        }

    }
    else if (strcmp(Command[0].file, "dirs") == 0) {
        for (int i = bufferSize-1; i >= 0; i--) {
            printf("%s\n", dirStack[i]);
        }
    }
    else if (strcmp(Command[0].file, "popd") == 0) {
        if (bufferSize > 1) {
            chdir(Command[0].args[1]);
            *stackSize = *stackSize - 1;
        }
        else{
            fprintf(stderr, "Nothing to popd\n");
        }
    }
    else {
        chdir(Command[0].args[1]);
        char *newDir = getcwd(buf2, BUFF_AMOUNT);
        //compare if the new directory is actually a file found and also checks if there is an argument.
        if (strcmp(getDir, newDir) == 0 && (Command[0].args[1] != NULL)) {
            fprintf(stderr, "Error: no such directory\n");
        }
        else if (strcmp(Command[0].file, "pushd") == 0) {
            //save old directory into the stack;
            dirStack[*stackSize] = getDir;
            *stackSize = *stackSize + 1;
        }
    }
}


int main(void)
{
    char cmd[CMDLINE_MAX];
    char *dirStack[BUFF_AMOUNT];
    int stackSize = 0;

    while (1) {
        char *nl;
        char buf1[BUFF_AMOUNT];
        char buf2[BUFF_AMOUNT];


        /* Print prompt */
        printf("sshell$ ");


        fflush(stdout);

        /* Get command line */
        fgets(cmd, CMDLINE_MAX, stdin);
        //makes sure line does not exceed 512 characters.
        while (strlen(cmd) > CMDLINE_MAX) {
            fprintf(stderr, "Command to long\n");
            printf("sshell$ ");
            fgets(cmd, CMDLINE_MAX, stdin);

        }
        /* Print command line if stdin is not provided by terminal */
        if (!isatty(STDIN_FILENO)) {
            printf("%s", cmd);
            fflush(stdout);
        }

        /* Remove trailing newline from command line */
        nl = strchr(cmd, '\n');
        if (nl)
            *nl = '\0';

        /* Builtin command */
        if (!strcmp(cmd, "exit")) {
            fprintf(stderr, "Bye...\n");
            fprintf(stderr, "+ completed '%s' [%d]\n", cmd, WEXITSTATUS(0));
            break;

        }

        /* Regular command */

        //start of fork implementation.


        int status;
        int argumentCount = 0;
        int commandCount = 1;
        //fork fails

        //Get a list all of the command inputs.
        char allCommands[ARGUMENT_MAX][TOKEN_MAX];
        memset(allCommands, '\0', sizeof(allCommands));
        for (int i = 0; i < ARGUMENT_MAX; i++){
            allCommands[i][0] = '\0';
        }
        // Separates commands and puts them in the variable allCommands
        // Assigns commandType a string
        char *commandType = commandSeparator(cmd, allCommands, &argumentCount);
        //Argument count checker
        if (argumentCount > ARGUMENT_MAX){
            perror("Error: too many process arguments\n");
            break;
        }
        struct Commands Command[ARGUMENT_MAX];
        createCommands(Command, allCommands, &commandCount);

        //do not need these commands using fork(), will cause bugs.
        if (strcmp(Command[0].file, "cd") == 0 || strcmp(Command[0].file, "dirs") == 0 ||
            strcmp(Command[0].file, "popd") == 0 || strcmp(Command[0].file, "pushd") == 0
            || strcmp(Command[0].file, "pwd") == 0){
            directoryStack(Command, dirStack, &stackSize, stackSize, buf1, buf2);
        }

        else if(commandCount == 1){
            pid_t pid;
            pid = fork();
            if (pid == 0) {


                if (commandType != NULL) {
                    if (strcmp(commandType, "redirect") == 0) {
                        //open the last argument of the command and deletes it (this is the redirect file).
                        int fd = open(Command[0].args[argumentCount], O_CREAT | O_WRONLY | O_TRUNC, 0644);
                        //delete the name of the file name argument from the object.
                        Command->args[argumentCount] = NULL;
                        dup2(fd, STDOUT_FILENO);
                        execvp(Command[0].file, Command[0].args);
                        close(fd);
                    } else {
                        int fd = open(Command[0].args[argumentCount], O_CREAT | O_WRONLY | O_TRUNC, 0644);
                        //delete the name of the file name argument from the object.
                        Command->args[argumentCount] = NULL;
                        dup2(fd, STDERR_FILENO);
                        execvp(Command[0].file, Command[0].args);
                        close(fd);
                    }
                } else {
                    execvp(Command[0].file, Command[0].args);
                    fprintf(stderr, "Error: command not found\n");
                }

                exit(1);
            }
            else if (pid > 0) {
                //wait for the child process to finish and print the status of the child.
                waitpid(-1, &status, 0);
                if (WEXITSTATUS(status) == 0) {
                    fprintf(stderr,"+ completed '%s' [%d]\n", cmd, WEXITSTATUS(status));
                }
                else {
                    fprintf(stderr, "+ failed '%s' [%d]\n", cmd, WEXITSTATUS(status));
                }

            }
        }
            //uses if there needs multiple pipes. (WORKS BUT ONLY ONCE THEN EXITS)
        else if (commandCount > 1) {
            char **pipeCmds[] = {};
            int i;
            int count = commandCount;
            for (i = 0; i < count; i++){
                pipeCmds[i] = Command[i].args;
            }
            pipeCmds[i] = NULL;
            pipeCommands(pipeCmds);
        }



    }

    return EXIT_SUCCESS;
}

