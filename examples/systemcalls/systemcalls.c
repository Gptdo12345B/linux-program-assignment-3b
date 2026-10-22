#include "systemcalls.h"
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
/**
 * @param cmd the command to execute with system()
 * @return true if the command in @param cmd was executed
 *   successfully using the system() call, false if an error occurred,
 *   either in invocation of the system() call, or if a non-zero return
 *   value was returned by the command issued in @param cmd.
*/
bool do_system(const char *cmd)
{
/*
 *  Call the system() function with the command set in the cmd
 *   and return a boolean true if the system() call completed with success
 *   or false() if it returned a failure
*/
    int retCode = system (cmd);
    bool success = (retCode == 0) ? true : false;
    return success;
}

/**
* @param count -The numbers of variables passed to the function. The variables are command to execute.
*   followed by arguments to pass to the command
*   Since exec() does not perform path expansion, the command to execute needs
*   to be an absolute path.
* @param ... - A list of 1 or more arguments after the @param count argument.
*   The first is always the full path to the command to execute with execv()
*   The remaining arguments are a list of arguments to pass to the command in execv()
* @return true if the command @param ... with arguments @param arguments were executed successfully
*   using the execv() call, false if an error occurred, either in invocation of the
*   fork, waitpid, or execv() command, or if a non-zero return value was returned
*   by the command issued in @param arguments with the specified arguments.
*/

bool do_exec(int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
        printf("\nloop: %d. command: %s\n", i, command[i]);
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    command[count] = command[count];

/*
 *   Execute a system command by calling fork, execv(),
 *   and wait instead of system (see LSP page 161).
 *   Use the command[0] as the full path to the command to execute
 *   (first argument to execv), and use the remaining arguments
 *   as second argument to the execv() command.
 *
*/
    // Make a child process.  The parent will wait for it, while the child
    // replaces its program image with the requested command.
    pid_t pid = fork();

    if (pid < 0) {
        // fork() failed, so no child exists to execute or wait for.
        va_end(args);
        return false;
    }

    if (pid == 0) {
        // This branch executes only in the child.  A successful execv() never
        // returns: it replaces this process with command[0].
        execv(command[0], command);

        // execv() returns only on failure.  End the child immediately so it does
        // not continue into the parent's wait/status logic.
        perror("execv");
        _exit(EXIT_FAILURE);
    }

    int status;
    // This branch is the parent.  Wait for this specific child so its exit
    // status can be used to decide whether the requested command succeeded.
    if (waitpid(pid, &status, 0) < 0) {
        va_end(args);
        return false;
    }

    // The variadic argument list is no longer needed after command[] is built.
    va_end(args);

    // status is encoded by waitpid().  Return true only when the child ended
    // normally and the program reported the conventional success exit code (0).
    // return true if executed successfully, else false. In linux, successful command 
    // yeids 0.
    return WIFEXITED(status) && (WEXITSTATUS(status) == 0);
  
}

/**
* @param outputfile - The full path to the file to write with command output.
*   This file will be closed at completion of the function call.
* All other parameters, see do_exec above
*/
bool do_exec_redirect(const char *outputfile, int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
        printf("\nloop: %d. command: %s\n", i, command[i]);
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    command[count] = command[count];
/*
 *   Call execv, but first using https://stackoverflow.com/a/13784315/1446624 as a refernce,
 *   redirect standard out to a file specified by outputfile.
 *   The rest of the behaviour is same as do_exec()
 *
*/

   // Make a child process.  The parent will wait for it, while the child
    // replaces its program image with the requested command.
    pid_t pid = fork();

    if (pid < 0) {
        // fork() failed, so no child exists to execute or wait for.
        va_end(args);
        return false;
    }

    if (pid == 0) {
        int fd = open(outputfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0) {
            _exit(EXIT_FAILURE);
        }

        if (dup2(fd, STDOUT_FILENO) < 0) {
            close(fd);
            _exit(EXIT_FAILURE);
        }

        close(fd);
        // This branch executes only in the child.  A successful execv() never
        // returns: it replaces this process with command[0].
        execv(command[0], command);

        // execv() returns only on failure.  End the child immediately so it does
        // not continue into the parent's wait/status logic.
        perror("execv");
        _exit(EXIT_FAILURE);
    }

    int status;
    // This branch is the parent.  Wait for this specific child so its exit
    // status can be used to decide whether the requested command succeeded.
    if (waitpid(pid, &status, 0) < 0) {
        va_end(args);
        return false;
    }

    // The variadic argument list is no longer needed after command[] is built.
    va_end(args);

    // status is encoded by waitpid().  Return true only when the child ended
    // normally and the program reported the conventional success exit code (0).
    // return true if executed successfully, else false. In linux, successful command 
    // yeids 0.
    return WIFEXITED(status) && (WEXITSTATUS(status) == 0);
}
