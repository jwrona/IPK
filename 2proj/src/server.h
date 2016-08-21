#ifndef SERVER_H
#define SERVER_H

#include <iostream>
#include <sstream>
#include <string>
#include <cerrno>
#include <cstdlib>
#include <cstdio>
#include <csignal>
#include <vector>

#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <sys/wait.h>
#include <pwd.h>

using namespace std;

class clientArgs
{
  enum criterion
  { none, login, uid } searchCrit;

public:
  static const int prCount = 6;
  int prArgs[prCount];
    vector < string > searchArgs;

    clientArgs();
  void setSearchByLogin()
  {
    searchCrit = login;
  };
  void setSearchByUID()
  {
    searchCrit = uid;
  };
  bool searchByLogin();
  bool searchByUID();

  void clearArg()
  {
    searchArgs.clear();
  };

  bool emptyArg()
  {
    return searchArgs.empty();
  };

  void pushArg(string);

  string popArg();
};



bool makeOpt(const int argc, char *argv[], char *port);
addrinfo *getSockAddr(const char *service, const int family, const int socktype,
		      const int protocol = 0, const int flags = 0);
int sockAndBind(addrinfo * res);
bool mySigaction();
void sigchldHandler(int);
void sigintHandler(int);
string fetchMsg(const int);
bool parseMsg(string &, clientArgs &);
bool generateMsg(clientArgs &, string &);
string stringizePwEnt(const clientArgs&, const struct passwd*);
#endif // SERVER_H
