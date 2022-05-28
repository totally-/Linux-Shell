# Linux-Shell
Rudimentay Linux shell written in C

It handles basic commands, fork and piped commands, system calls, and file redirection. 

## This shell handles the "built-in" commands, "exit", "help", "pwd", and "cd" as follows:

Built-in      Option                       Result
Command  
---------     -----------------------      ----------------------------------------------------------------------------------
exit          a number for the exit        Exits the shell with the given exit code. If no option is given, exit with exit
              code                         code 0. 


help          none                         Prints a help message. (See below for the message.)


pwd           none                         Prints the current working directory.


cd            a directory                  Changes the current working directory to the given director. If no directory
                                           is given, the directory will be changed to the directory set in the HOME 
                                           environment variable.

So, as an example, if you type "exit" into the shell, the shell should exit with exit code 0. If you type "exit 4", 
the shell should exit with exit code 4.

For the "help" command, it will print:

*** SHELL FEATURES ***"
List of Built-Ins:
exit <optional exit code>
cd <directory>
pwd
help

You can press Ctrl-C to terminate this shell.

-------------------------------------------------------------------------------------------------------------------------------------

### Error Handling

Built-in     Error                   Output to stderr
Command
--------     ---------------------   -------------------------------------------------------------
exit         non-numeric exit        "Exit code must be a number. You entered: asdf\n"
             code, example "asdf"


exit         exit code not between   "Exit code must be a number from 0 to 255! You entered 400\n"
             0 and 255 (inclusive),
             example 400


cd           directory does not      "cd faild: No such file or directory"
             exit


--------------------------------------------------------------------------------------------------------------------------------------------

#### Fork-Exec-Wait and Piping


The shell determines if the command is a built-in. If so, it executes the built-in as described above. 
If the command is not a built-in, then it will
        1. fork a new process to create a child process,
        2. use execvp to replace the child process with the command entered by the user,
        3. have the parent wait for the child process to execute.
   	If execvp fails, we call perror("execvp"); to print an error message. Then, call exit(EXIT_FAILURE); to exit 
           with EXIT_FAILURE exit code.
        4. If the command contains multiple commands piped together, such as, "echo "hello, hello, hello!" | grep -o h | wc -l", 
           execute each command piping output to input appropriately using dup and dup2.


-------------------------------------------------------------------------------------------------------------------------------------

##### File Redirection


To enable file redirection operators, i.e., '<' and '>', we will need to parse commands with those characters. For example, 
a command might be, "cat < input > output".

We will do file redirection using dup and dup2 using the following steps:

1. Backup stdin and stdout using dup.
2. If < and/or > are present in the command (In other words, if infile and outfile are non-empty after 
calling getFilenamesReturnCommand, open infile for input and outfile for output.), then

    a.) Open input and/or output files
    b.) Use dup2 to duplicate the file resources for the opened files and give them file descriptors stdin and stdout
    c.) Close the file descriptors for the opened files.

3. Execute the command as usual.
4. Restore stdin and stdout to keyboard and console using the backups from Step 1. 


--------------------------------------------------------------------------------------------------------------------------------------------


