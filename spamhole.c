#include "spamhole.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <ctype.h>

int spamhole() {
	int lsock, csock, osock;
	FILE *cfile, *f1;
	char buf[4096];
	char *buf2;
	char *filename;
	char *header;
	char countstr[32];
	long int count;
	struct sockaddr_in laddr, caddr, oaddr;
	int caddrlen = sizeof(caddr);
	fd_set fdsr, fdse;
	struct hostent *h;
	//struct servent *s;
	int x, nbyt;
	unsigned long a;
	unsigned short oport;
	int replaced, hole = 0;

	/* Convert SMTP_RELAY into binary network byte order */
	a = inet_addr(SMTP_RELAY);
	if (!(h = gethostbyname(SMTP_RELAY)) &&
	   !(h = gethostbyaddr(&a, 4, AF_INET))) {
		perror(SMTP_RELAY);
		return 25;
	}

	/* Convert SMTP_PORT into an int */
	oport = atol(SMTP_PORT);

	/* Convert localport to unsigned short int, then to network byte order */
	laddr.sin_port = htons((unsigned short)(atol(LOCAL_PORT)));

	/* Open a TCP socket and assign it to lsock */
	if ((lsock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		perror("socket");
		return 20;
	}

	/* Convert AF_INET into an unsigned short int */
	laddr.sin_family = htons(AF_INET);
	if (!inet_aton(LOCAL_IP,&laddr.sin_addr))
		laddr.sin_addr.s_addr = INADDR_ANY;

	/* Bind our local address to our socket (lsock) */
	if (bind(lsock, (struct sockaddr*)&laddr, sizeof(laddr))) {
		perror("bind");
		return 20;
	}

	/* Set our socket to listen for new connections */
	if (listen(lsock, 1)) {
		perror("listen");
		return 20;
	}

	/* Fork our child off and return child PID in parent */
	if ((nbyt = fork()) == -1) {
		perror("fork");
		return 20;
	}
	if (nbyt > 0)
	return nbyt;

	/* Change UID to our UID */
	if( setuid( UID ) != 0 ) {
		perror("setuid");
		return 20;
	}

	/* Reset the session group */
	setsid();

	/* Accept any pending connections in the queue and assign to csock */
	while ((csock = accept(lsock, (struct sockaddr*)&caddr, &caddrlen)) != -1) {
		/* Open the socket for reading and writing */
		cfile = fdopen(csock,"r+");
		/* Fork connection to child, if fails, print error to socket and close it */
		if ((nbyt = fork()) == -1) {
			fprintf(cfile, "500 fork: %s\n", strerror(errno));
			shutdown(csock,2);
			fclose(cfile);
			continue;
		}
		/* Child process from fork() */
		if (nbyt == 0)
			goto gotsock;
		/* Parent process from fork() */
		fclose(cfile);

		/* Wait for any children to exit */
		while (waitpid(-1, NULL, WNOHANG) > 0);
	}
	return 20;

	/* Got an incoming socket, determine whether to modify data */
	gotsock:
	if(DEBUG) syslog( LOG_INFO, "Received connection\n" );

	/* Check for ip database */
	if( access( "ipdb", F_OK ) != 0 ) {
		mkdir( "ipdb", S_IEXEC|S_IRUSR|S_IWUSR );
	}

	filename = malloc(strlen("ipdb/") + caddrlen + 1 );
	sprintf( filename, "ipdb/%d", caddr.sin_addr.s_addr );
	if(DEBUG>1) syslog( LOG_DEBUG, "Checking filename %s...\n", filename );

	if( access( filename, F_OK ) == 0 ) {
		f1 = fopen( filename, "r" );
		fgets( countstr, sizeof(countstr), f1 );
		fclose( f1 );
		count = atoi(countstr) +1;
		if(DEBUG) syslog( LOG_DEBUG, "Host has connected %ld times...\n", count - 1 );
		/* Incriment the counter */
		if(DEBUG) syslog( LOG_DEBUG, "Incrimenting connection counter...\n" );
		f1 = fopen( filename, "w" );
		fprintf( f1, "%ld", count );
		fclose( f1 );
	} else {
		/* Initialize the counter to 1 */
		if(DEBUG) syslog( LOG_DEBUG, "No previous connections, initializing counter...\n" );
		f1 = fopen( filename, "w" );
		fprintf( f1, "1" );
		fclose( f1 );
		count = 1;
	}

	/* Decide whether or not to hole the spam */
	if( count > MAX_CON ) {
		if(DEBUG) syslog( LOG_INFO, "Connection count greater than %ld, to the hole with ye!\n", MAX_CON );
		hole = 1;
	} else {
		if(DEBUG) syslog( LOG_INFO, "Connection count less than %ld, allowing passthrough.\n", MAX_CON );
	}

	/* Send the connection to the REAL smtp server */
	if(DEBUG>1) syslog( LOG_DEBUG, "Sending connection to smtp relay...\n" );
	if(DEBUG>2) syslog( LOG_DEBUG, "Session transcript:\n" );

	/* create a new tcp socket and assign it to osock */
	if ((osock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		fprintf(cfile, "500 socket: %s\n", strerror(errno));
		goto quit1;
	}

	/* Define our outgoing address */
	oaddr.sin_family = h->h_addrtype;
	oaddr.sin_port = htons(oport);
	memcpy(&oaddr.sin_addr, h->h_addr, h->h_length);

	/* name our socket */
	if (connect(osock, (struct sockaddr*)&oaddr, sizeof(oaddr))) {
		fprintf(cfile, "500 connect: %s\n", strerror(errno));
		goto quit1;
	}
	while (1) {
		FD_ZERO(&fdsr);
		FD_ZERO(&fdse);
		FD_SET(csock,&fdsr);
		FD_SET(csock,&fdse);
		FD_SET(osock,&fdsr);
		FD_SET(osock,&fdse);
		buf[0] = '\0';

		/* Check for data to be read from the connection */
		if (select(20, &fdsr, NULL, &fdse, NULL) == -1) {
			fprintf(cfile, "500 select: %s\n", strerror(errno));
			goto quit2;
		}

		/* Inbound Data */
		if (FD_ISSET(csock,&fdsr) || FD_ISSET(csock,&fdse)) {
		/* Read */
		for( x=0; x<4096; x++ ) {
			if( read(csock,&buf[x],1) <= 0 ) goto quit2;
			if( buf[x] == '\n' ) {
				buf[x+1] = '\0';
				break;
			}
		}

		/* Write */
		/* If this session has been determined to be holed, watch for headers to replace */
		replaced = 0;
		if( hole ) {
			header = malloc(9);
			snprintf( header, 9, "%s", buf );
			for( x=0; x<strlen(header); x++ ) header[x] = tolower(header[x]);
			if( strcmp( header, "rcpt to:" ) == 0 ) {

				buf2 = malloc( strlen("RCPT TO: ") + strlen(HOLEADDR) + 2 );
				sprintf( buf2, "RCPT TO: %s\r\n", HOLEADDR );
				if( write(osock, buf2, strlen(buf2)) <= 0 ) goto quit2;
				if(DEBUG>2) syslog( LOG_DEBUG, "%s", buf2 );
				free(buf2);

				replaced = 1;
			}
			free(header);

			header = malloc(4);
			snprintf( header, 4, "%s", buf );			
			for( x=0; x<strlen(header); x++ ) header[x] = tolower(header[x]);
			if( strcmp( header, "to:" ) == 0 ) {

				buf2 = malloc( strlen("To: ") + strlen(HOLEADDR) + 2 );
				sprintf( buf2, "To: %s\r\n", HOLEADDR );
				if( write(osock, buf2, strlen(buf2)) <= 0 ) goto quit2;
				if(DEBUG>2) syslog( LOG_DEBUG, "%s", buf2 );
				free(buf2);

				buf2 = malloc( strlen("X-SpamHole-Ident: ") + strlen(IDENT) + 2 );
				sprintf( buf2, "X-SpamHole-Ident: %s\r\n", IDENT );
				if( write(osock, buf2, strlen(buf2)) <= 0 ) goto quit2;
				if(DEBUG>2) syslog( LOG_DEBUG, "%s", buf2 );
				free(buf2);

				buf2 = malloc( strlen("X-SpamHole-Orig") + strlen(buf) + 2 );
				sprintf( buf2, "X-SpamHole-Orig%s", buf );
				if( write(osock, buf2, strlen(buf2)) <= 0 ) goto quit2;
				if(DEBUG>2) syslog( LOG_DEBUG, "%s", buf2 );
				free(buf2);

				replaced = 1;
			}
			free(header);

			if( ! replaced ) {
				if(DEBUG>2) syslog( LOG_DEBUG, "%s", buf );
				if( write(osock, buf, strlen(buf)) <= 0 ) goto quit2;
			}
		} else { 
			if(DEBUG>2) syslog( LOG_DEBUG, "%s", buf );
			if( write(osock, buf, strlen(buf)) <= 0 ) goto quit2;
		}

		/* Outbound Data */
		} else if (FD_ISSET(osock,&fdsr) || FD_ISSET(osock,&fdse)) {
		/* Read */
		for( x=0; x<4096; x++ ) {
			if( read(osock,&buf[x],1) <= 0 ) goto quit2;
			if( buf[x] == '\n' ) {
				buf[x+1] = '\0';
				break;
			}
		}
		if(DEBUG>2) syslog( LOG_DEBUG, "%s", buf );
		/* Write */
		if( write(csock, buf, strlen(buf)) <= 0 ) goto quit2;
		}

	}

	quit2:
	shutdown(osock,2);
	close(osock);

	quit1:
	fflush(cfile);
	shutdown(csock,2);

	fclose(cfile);
	return 0;
}

