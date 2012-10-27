//
// main.cpp for Spamhole in /home/vianney/Work/other/spamholeReturn
// 
// Made by Vianney Bouchaud
// Login   <vianney@bouchaud.org>
// 
// Started on  Tue Oct 23 10:48:30 2012 Vianney Bouchaud
// Last update Tue Oct 23 14:50:17 2012 Vianney Bouchaud
//

#include <ligben.h>
#include <syslog.h>
#include <errno.h>

#include "Spamhole.hpp"

int main(int ac, char **av) {
  Spamhole sh;

  openlog(basename(av[0]), LOG_PID, LOG_MAIL);

  sh.showVersion();

  if (useDirectory() == false)
    return (-1);

  if ((pid = fork()) >= 0) {
    sh.run();
  }
  else {
    perror("fork");
    return (-1);
  }
  return (0);
}
