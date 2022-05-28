#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

const int BUFFER_SIZE = 10000;

void printWelcome();
void printPrompt();
void printHelp();
void printPwd();
void handleExit(char * arg);
void handleCd(char * arg);
int handleBuiltIns(char * cmd, char * arg);
int getNumArgs(char * buffer);
int isPiped(char *buffer);
void pipedCmds(int piped, char *buffer);
void call(char * cmd[]);
int isFiled(char *buffer);
void filedCmd(char * buffer);
void regularOrPipedCmd(char * buffer, int stdin_bak, int stdout_bak);


int main(void)
{

  char buffer[BUFFER_SIZE]; //string array of size 1000.

  printWelcome();

  // make back ups for stdin and stdout
  int stdin_bak = dup(STDIN_FILENO); // reads from the keyboard
  int stdout_bak = dup(STDOUT_FILENO); // writes to the console

  while(1){

        printPrompt(); //print the command line prompt

        // 'get' user input. NOTE: user input automatically inserts a newline character at the end
        fgets(buffer, sizeof(buffer), stdin);


        // check for file redirection command(s)
        int filed = isFiled(buffer); // filed will have a value greater than zero if the user entered a command with '>' or '<'

        if(filed != 0)
        {
            filedCmd(buffer);

        } //end of if filed !=0
        else
        {
            // if the user entered a 'piped command' or a 'regular command'
            regularOrPipedCmd(buffer, stdin_bak, stdout_bak);
        } //end of else if piped !=0



    } //end of while loop

    // close backups of stdin and stdout
    close(stdin_bak);
    close(stdout_bak);

    return 0;

} // end of main

void printWelcome() {

  printf("******************************************\n");
  printf("*                                        *\n");
  printf("*      ****Welcome to my shell!****      *\n");
  printf("*                                        *\n");
  printf("******************************************\n");

} //end of printWelcome

void printPrompt(){
  char buffer[BUFFER_SIZE];
  getcwd(buffer, BUFFER_SIZE);
  printf("%s$ ", buffer);

} //end of printPrompt

void printHelp() {
  printf("\n*** SHELL FEATURES ***"
    "\nList of Built-Ins:"
    "\n> exit <optional exit code>"
    "\n> cd <directory>"
    "\n> pwd"
    "\n> help"
    "\n\nYou can press Ctrl-C to terminate this shell.\n");
}

void printPwd() {
  char buffer[BUFFER_SIZE];
  getcwd(buffer, BUFFER_SIZE);
  printf("%s\n", buffer);
}

void handleExit(char * arg) {
  if (arg == NULL) {
    printf("Goodbye!\n");
    exit(0);
  } //end of if

  char * endptr = NULL;
  long exitCode = strtol(arg, & endptr, 10); // 1st parameter = string to convert to number. 2nd parameter = see below, 3rd parameter = base (in this case, it's base 10)
  // 2nd parameter = first character in string that is not numeric. ex. "45Ab", endptr will point to the A

  if ( * endptr != '\0') { //if != '\0' that means that the argument is non-numeric
    fprintf(stderr, "Exit code must be a number. You entered: %s\n", arg);
    return;
  } //end of if

  if (exitCode < 0 || exitCode > 255) {
    fprintf(stderr, "Exit code must be a number from 0 to 255! You entered: %ld\n", exitCode);
    return;
  } // end of if

  printf("Goodbye!\n");
  exit(exitCode);

} // end of handleExit function

void handleCd(char * arg) {
  if (arg == NULL) { //the user typed in cd but no directory
    int err = chdir(getenv("HOME")); // chdir = change directory, getnv = get environment variable, HOME = the environement variable and will return the string with the
    // environment variable, home
    if (err == -1) {
      fprintf(stderr, "cd failed: No such file or directory\n");
    } //end of if
    return;
  } //end of if

  //if the user does type in an argument, like cd unixstuff
  int err = chdir(arg);
  if (err == -1) {
    fprintf(stderr, "cd failed: No such file or directory\n");
  } // end of if

} //end of handleCd

int handleBuiltIns(char * cmd, char * arg) {
  if (cmd == NULL) {
    return 1; // return to command line
  }
  if (strcmp(cmd, "exit") == 0) { // if buffer and "exit" are the exact same string.
    handleExit(arg);
    return 1;
  } //end of if
  else if (strcmp(cmd, "help") == 0) {
    printHelp();
    return 1;
  } //end of else if
  else if (strcmp(cmd, "pwd") == 0) {
    printPwd();
    return 1;
  } //end of else if
  else if (strcmp(cmd, "cd") == 0) {
    handleCd(arg);
    return 1;
  } // end of else if

  return 0; // whatever the user entered into the command, it was not exit, help, pwd, or cd.

} //end of handleBuiltIns

int getNumArgs(char * buffer) { //<<<<<<<<<<<< this is unused in this program. I was just trying to figure out how to count the number of arguments excluding the command from
  // the command line.
  int numArgs = 0;
  char * ptr = buffer;

  while ((ptr = strpbrk(ptr, " \n")) != NULL) {
    numArgs++;
    ptr++;
  }
  numArgs--; // decrement because we are not including the command from the command line.  Ex., "echo -n "Hello!""  We don't count the echo command, only the arguments. So, in
  // this case, it would return 2.

  return numArgs;
} //end of numArgs

void call(char *argv[]){
    if(handleBuiltIns(argv[0], argv[1]) == 1){ //if it's not a built in
        return;
    } //end of if
        //fprintf(stderr, "Command, %s, not recognized!\n", cmd);

    pid_t pid = 0;
    int status;

    pid = fork(); // fork a new shell

    if (pid == 0) // In the child proceess
    {
        // execute the command
        if (execvp(argv[0], argv) == -1)
        { // swap out all instructions in the child process completely with the instructions for cmd
            // execvp will return -1 if it can't find the command the user entered
            perror("execvp");
            exit(EXIT_FAILURE); // exit out of the child process and return to our regular shell
        } //end of if

    }
    if (pid > 0) // In the parent process
    {
        pid = wait( &status); //Parent sees that process finished with status, pid, status
    }
    if (pid < 0) // not in child or parent. Something "went wrong in fork"
    {
        fprintf(stderr, "Fork failed!\n");
    }

    return;

} //end of call

int isPiped(char *buffer){
    // make a copy of buffer and determine if the user entered a piped command
    char tmpBuffer[BUFFER_SIZE];
    int piped = 0;
    int i = 0;
    while (buffer[i] != '\0') {
      tmpBuffer[i] = buffer[i];
      if (buffer[i] == '|') {
        piped++;
      }
      i++;
    }

    return piped; // if piped is greater than zero then the user entered a piped command

} //end of isPiped

void pipedCmds(int piped, char *buffer)
{
    // make a copy of the bufffer
    char tmpBuffer[BUFFER_SIZE/2+1];
    int i = 0;
    while (buffer[i] != '\0') {
      tmpBuffer[i] = buffer[i];
      i++;
    }

    char * pipedArgs[BUFFER_SIZE / 2 + 1]; //this array of strings will hold our commands separated by pipe

      //tokenize  by pipe or newline characters
      pipedArgs[0] = strtok(tmpBuffer, "|\n"); // get the first piped command

      // get the remaining 'piped' commands
      i = 0;
      for (i = 1; i < piped + 1; i++)
      {
        pipedArgs[i] = strtok(NULL, "|\n");
      }

      // print pipedArgs  <<<< This is only used for testing. will delete later
      i = 0;
      for (i = 0; i < piped + 1; i++)
      {
        //printf("pipedArgs[%d] = %s\n", i, pipedArgs[i]);
      } //end of while loop


      int stdin_bak = dup(STDIN_FILENO); // reads from the keyboard
      int stdout_bak = dup(STDOUT_FILENO); // writes to the console

      int pipefd[2];

       char * tmpArgs[BUFFER_SIZE / 2 + 1];
      //
      i = 0;
      for (i = 0; i < piped + 1; i++) {

        strcpy(tmpBuffer, pipedArgs[i]);
        //printf("i = %d\n", i);
        //printf("tmpBuffer = %s\n", tmpBuffer);

        tmpArgs[0] = strtok(tmpBuffer, " \n");
        int k = 0;
        while(tmpArgs[k] != NULL)
        {
          tmpArgs[++k] = strtok(NULL, " \n");
          //printf("tmpArgs[%d] %s\n", k, tmpArgs[k]);
        }


        // handle the piped commands
        if(i != piped) // if not the last 'piped command'
        {
          // create file descriptors for the pipe with a write end and read end of the pipe
          if (pipe(pipefd) < 0)
          {
            perror("pipe");
            exit(EXIT_FAILURE);
          }

          dup2(pipefd[1], STDOUT_FILENO);
          call(tmpArgs);
          dup2(pipefd[0], STDIN_FILENO);

          close(pipefd[1]);
          close(pipefd[0]);
        }
        else //if the last 'piped command'
        {

          dup2(stdout_bak, STDOUT_FILENO);
          call(tmpArgs);
          dup2(stdin_bak, STDIN_FILENO);
          dup2(stdout_bak, STDOUT_FILENO);

          //close out the read and write
          close(stdin_bak);
          close(stdout_bak);

        }



      } //end of for loop
} //end of pipedCmds

int isFiled(char *buffer){
    // make a copy of buffer and determine if the user entered a '>' or a '<' command
    char tmpBuffer[BUFFER_SIZE];
    int filed = 0;
    int i = 0;
    while (buffer[i] != '\0') {
      tmpBuffer[i] = buffer[i];
      if (buffer[i] == '>' || buffer[i] == '<') {
        filed++;
      }
      i++;
    }

    return filed;

} //end of isFiled

void filedCmd(char * buffer)
{

    // handle file redirection commands

    // 0. parse the command to obtain input and output file names
          char *output = strrchr(buffer, '>');
          if (output != NULL) {
            *output = '\0'; // buffer = "cat < in ", output = " out\n"
            output++;
            output  = strtok(output, " \n");  // output = "out"
          }

          char *input  = strrchr(buffer, '<');// buffer = "cat < in "

          if (input != NULL)
          {
            *input = '\0'; // buffer = "cat \0", input = " in "
            input++;
            input  = strtok(input, " \n");
          }

          char *argv[1000];
          int i = 0;
          argv[i] = strtok(buffer, " \n");
          while(argv[++i] = strtok(NULL, " \n"));


          // 1. Backup stdin and stdout
          int stdin_bak = dup(STDIN_FILENO);
          int stdout_bak = dup(STDOUT_FILENO);

          // 2. Open input/output files if needed
          // 3. Use dup2 to duplicate the file resourses for the opened files and give them file descriptors stdin and stdout
          // 4. Close the file descriptors for the opened files.
          if(output != NULL)
          {
	          int outfd = open(output, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR);
            dup2(outfd, STDOUT_FILENO);
            close(outfd);
          }

          if(input != NULL){
	          int infd = open(input, O_RDONLY, S_IRUSR | S_IWUSR);
	          dup2(infd, STDIN_FILENO);
	          close(infd);
          }

          // 5. Execute command as usual
          call(argv);

          // 6. Restore stdin and stdout
          dup2(stdout_bak, STDOUT_FILENO);
          dup2(stdin_bak, STDIN_FILENO);
} //end of filedCMD

void regularOrPipedCmd(char * buffer, int stdin_bak, int stdout_bak){
    //char * argv[BUFFER_SIZE / 2 + 1]; // will hold the command and arguments
          char * cmds[BUFFER_SIZE / 2 + 1]; // will hold the command and arguments
          int piped = isPiped(buffer); // piped will have a value greater than zero if the user entered a commaned with '|'

          int pipefd[2]; // file descriptors for piped command (read or write)

          // split by pipe AKA "|"
          int i = 0;
          cmds[i] = strtok(buffer, "|");
          while(cmds[++i] = strtok(NULL, "|"));
          //NOTE: if there is no pipe, cmd is the contents of the entire buffer

          // get the arguments
          for(int q = 0; cmds[q] != NULL; q++)
          {
            char *argv[BUFFER_SIZE / 2 + 1];
            int j = 0;
            argv[j] = strtok(cmds[q], " \n");
            while(argv[++j] = strtok(NULL, " \n")); // split sub commands by space AKA " "

            if(cmds[q+1] == NULL){ // if the last piped command
                dup2(stdout_bak, STDOUT_FILENO); // stdout writes to the console
                call(argv); // the return value of the call() function is written to stdout (the console)
                dup2(stdin_bak, STDIN_FILENO); // stdin reads from the keyboard
            }
            else // if NOT the last piped command
            {
                if(pipe(pipefd) < 0)
                {
	                perror("pipe");
	                continue;
                }
                dup2(pipefd[1], STDOUT_FILENO); // stdout writes to the pipe
                call(argv); // the return value of the call() function is written to the pipe
                dup2(pipefd[0], STDIN_FILENO); // stdin reads from the pipe

                close(pipefd[0]);
                close(pipefd[1]);
            }

          } // end of for loop
} //end of regularOrPipeCmd
