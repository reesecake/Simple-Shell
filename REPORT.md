# Project 1: sshell
Steven Tan
Reese Lam

### Implementation
The sshell was written following the suggested phases in the prompt file. 
1. Running commands with execvp() instead of system()
2. Parsing the command line for commands and arguments.
3. Adding the builtin commands: pwd and cd
4. Output redirection with the > operator
5. Pipeline command 
6. The extra features of standard error redirection and directory stack. Some 
error messaging was also added here.

Parsing the command line was done in the commandSeparator and createCommands 
functions. The line was split into a file variable for commands and an array 
of char pointers for 16 arguments. This is also where it is determined if 
output redirection '>', pipeline '|', standard error redirection, and certain 
errors are processed. 

For the extra features, the directory stack is strings of the directory lines. 
This is processed in its own function that uses getcwd() and chdir() to do the 
relevant operations for the purpose of dirs, pushd, and popd.

Pipeline commands are executed in main() after commands are parsed and the 
commandCount variable is incremented. Then, in the else if condition, the 
pipeline commands are counted and then exectued in the pipeCommands() function.
### Parsing
Since the cmd variable is initially stored in an array of chars'. We first 
parsed the cmd into tokens in the "commandSeparator" function. This will find 
if there is a "|" or redirection operand and to see if there are more than one 
command. If there is an operand, we store the commands in a 3d array. This 3d 
array is then passed into the next parser "createCommands", where it 
transforms into a neater array of structs that has a "file" and "args" char 
variables.

### Builtin Commands
These commands will be executed outside of the fork() to allow not unwanted 
outputs or status errors. This was done mainly in the "directoryStack" 
function where it takes in the array of Command structs and execute the 
appropriate command. Since the function pwd and cd do not need to call exec, 
we will use the command getcwd and chdir to execute these commands.

### Output redirection
This was done with the help of the parser which is able to identify 
redirection operators. When an operator is identified, the parser will change 
a variable called "commandType" to the specific type of redirection. Later 
into the code, there is an if statment that will check for this; If the 
commandType of NULL then it will execute a command using the exec function. 
If there is redirection, the appropriate file descriptor will be 
opened/created and redirected/opened/closed/dupped based on the type of 
redirection.


### Pipeline Commands
Currently, our pipeline implementation does not work with the test cases 
provided. However, commands like "date | tr 2 1 | tr 1 4" does work with the 
pipe command. The idea around our pipe implementation is passing all the 
commands' args to a function called "pipeCommands" and try looping through 
the children and execute back up to the parent.

### Extra Features (Redirection)
Stated in the previous section about redirection. This basically has the same 
concept as the previous. Each redirection is given a tag and this tag will be 
used as an 'if condition' to see how the file will be redirected.

### Extra Features (Directory Stack)
Currently, the directory stack implementation is not complete. This feature is 
where we spent the most time on. We initially tired to store the directorys in 
an 2d char array to represent a stack and later tried to transition into a 
linked list. However, the problem we faced is that getcwd(buffer,size) is used 
to store the directory onto the stack but whenver the buffer is changed, so 
will everything that used that buffer.

### Testing
Testing for sshell.c was done by running it in the CLion IDE and using any 
commands for the feature in development.
ie. To test for the stderr "Error: missing command" message processed in 
commandSeparator():
```sh
sshell$ > filename
sshell$  > filename
sshell$                    > filename
```
sshell.c was also tested on the CSIF computers using the makefile.


Resources:
https://docs.microsoft.com/en-us/windows-server/administration/windows-commands/pushd
https://www.computerhope.com/unix/umkdir.htm
https://stackoverflow.com/questions/8389033/implementation-of-multiple-pipes-in-c
https://stackoverflow.com/questions/45503324/add-an-element-at-the-beginning-of-an-array-and-shift-the-rest-but-keep-the-size
https://www.oreilly.com/library/view/learning-the-bash/1565923472/ch04s05.html
https://agilesysadmin.net/understanding-pushd-and-popd/
https://stackoverflow.com/questions/21248840/example-of-waitpid-in-use
https://unix.stackexchange.com/questions/427935/process-and-fork-method
