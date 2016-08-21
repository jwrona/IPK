/*
 * webclient.h
 * Jan Wrona, xwrona00@stud.fit.vutbr.cz
 * project #1, IPK
 */
#ifndef WEBCLIENT_H
#define WEBCLIENT_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/socket.h>		//socket(), ...
#include <string.h>		//memset()
#include <cstdlib>
#include <cstdio>
#include <string>
#include <netdb.h>		//struct addrinfo
#include <regex.h>		//regular expressions
#include <assert.h>
using namespace std;

enum codeType
{ codeNotSet, ok, redirect, error };
enum lengthType
{ lengthNotSet, none, chunked, contentLength };

class headerInfo
{
  codeType action;
  lengthType length;
  int contentLength;
  string redirURI;
public:
    headerInfo();
  void setAction(const int code);
  codeType showAction();
  void setLengthType(lengthType passedLen);
  lengthType showLengthType();
  void setLength(const int passedLen);
  int showLength();
  void setURI(const string URI);
  string showURI();
};

//debug
#define PR(MSG) cout << #MSG << " =   \t" << MSG << endl
void printAddrinfo(addrinfo * res);

//declarations
void errPrint(const char *message);
addrinfo *resolveURI(const string uri, const string service, const int family,
		     const int socktype, const int protocol = 0, const int flags = 0);
int sockAndConn(addrinfo * res);
bool makeParams(int argc, char *argv[], string & uri);
bool parseURI(const string uri, string & port, string & host, string & path, string & filename);
bool fetchHeader(string & headerBody, const int sockfd);
bool scanHeader(const string & body, headerInfo & headerData);
bool fetchEntity(headerInfo & headerData, string & filename, int sockfd);
void escapize(string & str);
bool illegalChar(const char &ch);

#endif //WEBCLIENT_H
