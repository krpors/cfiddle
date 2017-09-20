#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <unistd.h>

/*
 * Check man 7 daemon. These steps are redundant on systemd though. This
 * function is a SysV style daemon, where the program itself handles all 
 * the cruft about forking and whatnot.
 */
static void sysv_daemonize() {

	// Create the background process here.
	pid_t pid = fork();
	if (pid == -1) {
		// We have a failure during forking. Just exit.
		perror("Unable to fork");
		exit(1);
	}

	if (pid > 0) {
		// Success! Terminate the parent process.
		exit(0);
	}

	// Detach from any terminal, create an independent session.
	if (setsid() < 0) {
		perror("Unable to setsid");
		exit(1);
	}

	// At this point, we're in the child. Fork again, to ensure the daemon
	// can never re-acquire a terminal again.
	pid = fork();

	if (pid == -1) {
		perror("Unable to fork for a second time");
		exit(1);
	}

	if (pid > 0) {
		exit(0);
	}


	// dont inherit file creation permissions plx.
	umask(0);

	// change dir so we don't occupy directories
	if (chdir("/") == -1) {
		perror("Could not change to directory");
		exit(1);
	}

	// Close file descriptors; there's no way we know where this ends up
	// and stuff.
	close( STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	// Point stdin, out and err to /dev/null
	if (freopen("/dev/null", "r", stdin) == NULL) {
		perror("Unable to reopen stdin to /dev/null");
		exit(1);
	}
	if (freopen("/dev/null", "w", stdout) == NULL) {
		perror("Unable to reopen stdout to /dev/null");
		exit(1);
	}
	if (freopen("/dev/null", "w", stderr) == NULL) {
		perror("Unable to reopen stderr to /dev/null");
		exit(1);
	}
}

int main(int argc, char* argv[]) {
	sysv_daemonize();

	printf("Daemonzied");
	sleep(20);
	return 0;
}
