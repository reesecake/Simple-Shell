#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>

#define CMDLINE_MAX 512

int main(void)
{
  char cmd[CMDLINE_MAX];

  while (1) {
    char *nl;
    int retval;

    /* Print prompt */
    printf("sshell$ ");
    fflush(stdout);

    /* Get command line */
    fgets(cmd, CMDLINE_MAX, stdin);

    /* Print command line if stdin is not provided by terminal */
    if (!isatty(STDIN_FILENO)) {
      printf("%s", cmd);
      fflush(stdout);
    }

    /* Remove trailing newline from command line */
    nl = strchr(cmd, '\n');
    if (nl)
      *nl = '\0';

    /* Break cmd into each argument */
    char *command[3] = {0};
    char *pch;
    //printf("Splitting string \"%s\" into tokens:\n", cmd);
    pch = strtok(cmd, " ,.-");
    int i = 0;
    while (pch != NULL) {
    	command[i] = pch;
    	//printf("%s\n", command[i]);
    	pch = strtok(NULL, " ,.");
    	i++;
    }

    /* Builtin command */
    if (!strcmp(cmd, "exit")) {
      fprintf(stderr, "Bye...\n");
      break;
    }

    /* Regular command */
    // retval = system(cmd); system is too high-level to be used
    pid_t pid;
    pid = fork();
    if (pid == 0) {
    	//printf("I am the child and I will die soon!\n");
    	execvp(command[0], command);
    	perror("execvp");
    	exit(1);
    }
    else if (pid > 0) {
    	int status;
    	waitpid(-1, &status, 0);
    	printf("Child exited with return code %d\n", WEXITSTATUS(status));
    }
    else
    	printf("I am the initial process! But something went wrong...\n");
    //printf("I am here now!\n");
    // fprintf(stdout, "Return status value for '%s': %d\n", cmd, retval);
  }

  return EXIT_SUCCESS;
}
