#include "systemcalls.h"

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
 *  TODO: DONE.
 *  Call the system() function with the command set in the cmd
 *   and return a boolean true if the system() call completed with success
 *   or false() if it returned a failure
*/

    int ret;
    ret = system(cmd);
    if (ret != 0) {
        perror("System call returned with a failure\n");
        return false;
    }

    return true;
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
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    command[count] = command[count];

    va_end(args);

/*
 * TODO: DONE.
 *   Execute a system command by calling fork, execv(),
 *   and wait instead of system (see LSP page 161).
 *   Use the command[0] as the full path to the command to execute
 *   (first argument to execv), and use the remaining arguments
 *   as second argument to the execv() command.
 *
*/

    pid_t pid;
    pid = fork();

    // Error in creating a child.
    if (pid == -1) {
        perror("Failed to create a child process\n");
        return false;
    }

    // Child process code.
    else if (pid == 0) {
        // Call command to be executed.
        int ret;
        ret = execv(command[0], command);

        // Exit child process based on return value.
        if (ret == 0)
            exit(EXIT_SUCCESS);
        else
            exit(EXIT_FAILURE);
    }

    // Parent process code for post child process termination behaviour.
    else if (pid != 0) {
        int status;
        wait(&status);

        if (WIFEXITED(status)) {
            // When status is 0, child process successfully exited.
            if (WEXITSTATUS(status) == 0)
                return true;

            // Otherwise, there was an error.
            else
                perror("Child process exited with error\n");
        }
    }

    return false;
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
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    command[count] = command[count];

    va_end(args);

/*
 * TODO: DONE.
 *   Call execv, but first using https://stackoverflow.com/a/13784315/1446624 as a refernce,
 *   redirect standard out to a file specified by outputfile.
 *   The rest of the behaviour is same as do_exec()
 *
*/

    // Open the file for redirected output storage.
    int fp = creat(outputfile, 0644);
    if (fp < 0) {
        perror("Failed to open outputfile\n");
        return false;
    }

    // Flush the output buffer to prevent duplication of output.
    fflush(stdout);

    pid_t pid;
    pid = fork();

    // Error in creating a child.
    if (pid == -1) {
        perror("Failed to create a child process\n");
        return false;
    }

    // Child process code.
    else if (pid == 0) {
        // Redirect the output by making the fp and output buffer equivalent using dup2.
        if (dup2(fp, 1) == -1) {
            perror("Error redirecting output\n");
            return -1;
        }
        close(fp);

        // Call command to be executed.
        int ret;
        ret = execv(command[0], command);

        // Exit child process based on return value.
        if (ret == 0)
            exit(EXIT_SUCCESS);
        else
            exit(EXIT_FAILURE);
    }

    // Parent process code for post child process termination behaviour.
    else if (pid != 0) {
        // Close file for parent.
        close(fp);

        int status;
        wait(&status);

        if (WIFEXITED(status)) {
            // When status is 0, child process successfully exited.
            if (WEXITSTATUS(status) == 0)
                return true;

            // Otherwise, there was an error.
            else
                perror("Child process exited with error\n");
        }
    }

    return false;
}
