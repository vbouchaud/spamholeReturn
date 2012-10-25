//
// useDirectory.cpp for Spamhole in /home/vianney/Work/other/spamholeReturn
// 
// Made by Vianney Bouchaud
// Login   <vianney@bouchaud.org>
// 
// Started on  Tue Oct 23 11:37:49 2012 Vianney Bouchaud
// Last update Tue Oct 23 14:52:50 2012 Vianney Bouchaud
//

#include <syslog.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>

bool	useDirectory() {
  if (access(PWD, F_OK) != 0) {
    if (mkdir(PWD, S_IEXEC|S_IRUSR|S_IWUSR) < 0) {
      perror("mkdir");
      syslog(LOG_ERR, "Error: Unable to create working dir of '%s'\n", PWD);
      return (false);
    }
    if (chown(PWD, UID, 0) < 0) {
      perror("chown");
      syslog(LOG_ERR, "Error: Unable to change owner of '%s' to UID %d\n", PWD, UID);
      return (false);
    }
  }

  if (access(PWD, F_OK|R_OK|W_OK|X_OK) == 0) {
    if (chdir(PWD) < 0) {
      perror("chdir");
      syslog(LOG_ERR, "Error: Unable to change directory to '%s'\n", PWD);
      return (false);
    }
  }
  else {
    perror("chdir");
    syslog(LOG_ERR, "Error: Unable to use working dir of '%s' Check permissions\n", PWD);
    return (false);
  }
  return (true);
}
