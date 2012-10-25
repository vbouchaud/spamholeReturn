#define VERSION "0.4"
#define DEBUG 1

/* Configuration Parameters */

/* Home/Working dir */
#define PWD "/var/spamhole"

/* UID to run as */
#define UID 99

/* A unique identifier for this spamhole */
#define IDENT "my.host.name.com"

/* Listening interface address and port*/
#define LOCAL_IP "0.0.0.0"
#define LOCAL_PORT "25"

/* SMTP Relay host and port */
#define SMTP_RELAY "my.mailserver.com"
#define SMTP_PORT "25"

/* Max 'good' connections (unmodified relaying allowed) */
#define MAX_CON ((long)0)

/* HOLEADDR */
#define HOLEADDR "spam@spamhole.net"



/* ############################################## */
/* We suggest you don't touch anything below here */
/* ############################################## */


/* Includes */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <syslog.h>
#include <libgen.h>

/* Function Prototypes */
extern int spamhole();
