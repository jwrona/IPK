#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstdio>
#include <cerrno>

#include <string.h>		//memset()
#include <unistd.h>		//getopt()
#include <sys/socket.h>
#include <netdb.h>

#define PR(MSG) cout << #MSG << '=' << MSG << endl

using namespace std;

class clArgs
{
  string hostname;
  int port;
  enum criterion
  { none, login, uid } searchCrit;
    vector < string > searchArgs;

  enum pr
  { LOGIN, UID, GID, NAME, HOME, SHELL, COUNT };
  int prArgs[COUNT];
  int flagCount;
public:
    clArgs();
  void setHostname(const string & hstnm)
  {
    hostname = hstnm;
  };
  void setHostname(const char *hstnm)
  {
    hostname = hstnm;
  };
  string getHostname();

  bool setPort(const char *);
  void setPort(const int intPort)
  {
    port = intPort;
  };

  int getPort();
  string getPortStr();

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

  void pushArg(const char *);

  string popArg();

  void setLogin();
  void setUID();
  void setGID();
  void setName();
  void setHome();
  void setShell();
  string getPrArgs();
  void print();
};

void errPrint(const char *message);
bool makeOpt(const int argc, char *argv[], clArgs & args);
addrinfo *resolveURI(const string uri, const string service, const int family,
		     const int socktype, const int protocol = 0, const int flags = 0);
int sockAndConn(addrinfo * res);
string generateRequest(clArgs & args);
string fetchMsg(const int sockfd);

#endif //CLIENT_H
