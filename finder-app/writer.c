#include <syslog.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int main(int argc, char **argv) {
	// Set up syslog
	openlog(NULL, 0, LOG_USER);
	
	// Check if arguments are 3.
	// (executable, writefile, writestr)
	if (argc != 3) {
		syslog(LOG_ERR, "Require 2 arguments. Given: %d\n", argc);
		closelog();
		return 1;
	}

	// Parse the arguments.
	char *writefile = argv[1];
	char *writestr = argv[2];

	// Create/Truncate and open the writefile.
	int fp;
	fp = creat(writefile, 0644);

	// Syslog and exit if an error in file creation.
	if (fp == -1) {
		syslog(LOG_ERR, "Could not create file: %s\n", writefile);
		closelog();
		return 1;
	}

	// Syslog and write writestr to writefile.
	syslog(LOG_DEBUG, "Writing %s to %s\n",writestr, writefile);
	int strcount = strlen(writestr);
	int writecount = write(fp, writestr, strcount);

	// Syslog and exit if entire content is not written.
	if (writecount != strcount) {
		syslog(LOG_ERR, "Could not fully write to file: %s\n", writefile);
		closelog();
		return 1;
	}

	// Exit if successful.
	closelog();
	return 0;
}

