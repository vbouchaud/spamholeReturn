#include "spamhole.h"

int main( int argc, char *argv[] ) {
	pid_t pid;

	/* Open our syslog connection */
	openlog( basename(argv[0]), LOG_PID, LOG_MAIL );

	/* Version Info */
	printf( "spamhole reference implementation %s by I)ruid [CAU]\n", VERSION );
	syslog( LOG_INFO, "spamhole reference implementation %s by I)ruid [CAU] started...\n", VERSION );

	/* Create our working dir if it doesn't exist */
	if( access( PWD, F_OK ) != 0 ) {
		if( mkdir( PWD, S_IEXEC|S_IRUSR|S_IWUSR ) < 0 ) {
			perror("mkdir");
			syslog( LOG_ERR, "Error: Unable to create working dir of '%s'\n", PWD );
			exit(-1);
		}
		if( chown( PWD, UID, 0 ) < 0 ) {
			perror("chown");
			syslog( LOG_ERR, "Error: Unable to change owner of '%s' to UID %d\n", PWD, UID );
			exit(-1);
		}
	}

	/* Change to our home */
	if( access( PWD, F_OK|R_OK|W_OK|X_OK ) == 0 ) {
		chdir( PWD );
	} else {
		perror("chdir");
		syslog( LOG_ERR, "Error: Unable to use working dir of '%s' Check permissions\n", PWD );
		exit(-1);
	}

	/* Daemonize to background */
	pid=fork();
	if (pid<0) {
		perror("fork");
		exit(-1);
	}
	else if (pid==0) spamhole();

	return 0;
}

