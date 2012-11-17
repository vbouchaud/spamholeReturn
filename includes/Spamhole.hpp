//
// Spamhole.hpp for Spamhole in /home/vianney/Work/other/spamholeReturn
// 
// Made by Vianney Bouchaud
// Login   <vianney@bouchaud.org>
// 
// Started on  Tue Oct 23 14:54:52 2012 Vianney Bouchaud
// Last update Tue Oct 23 15:01:42 2012 Vianney Bouchaud
//

#ifndef __SPAMHOLE_HH__
# define __SPAMHOLE_HH__

#include <string>

class	Spamhole {
private:
  Spamhole(const Spamhole &);
  Spamhole	&operator=(const Spamhole &);
  std::string	_version;

  int		_lsock;
  int		_csock;
  int		_osock;
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

public:
  Spamhole();
  ~Spamhole();
  void	showVersion(void) const;
  void	run(void);
};

#endif /* !__SPAMHOLE_HH__ */
