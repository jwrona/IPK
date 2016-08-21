/*
 * webclient.c
 * dependent on webclFunc.c
 * Jan Wrona, xwrona00@stud.fit.vutbr.cz
 * project #1, IPK
 */

#include "webclient.h"
using namespace std;

int main(int argc, char *argv[])
{
  string uri, port, host, path, filename;
  int redirCount = 5;

  if (!makeParams(argc, argv, uri))
    return EXIT_FAILURE;	//params
  do
  {
    if (!parseURI(uri, port, host, path, filename))
      return EXIT_FAILURE;
    escapize(path);

    addrinfo *res = resolveURI(host, port, AF_UNSPEC, SOCK_STREAM);	//addrinfo
    if (res == 0)
      return EXIT_FAILURE;

    int sockfd = sockAndConn(res);	//socket create + connection
    freeaddrinfo(res);
    if (sockfd == -1)
      return EXIT_FAILURE;

    //request////////////////////////////////////////////////////////////////////
    string getmsg("GET " + path + " HTTP/1.1\r\n" +
		  "Connection: close\r\n" + "Host: " + host + "\r\n" +
		  "User-Agent: webclient/1.0 http://www.fit.vutbr.cz/" + "\r\n" + "\r\n");

    if (send(sockfd, getmsg.data(), getmsg.length(), 0) == -1)
    {
      perror("send");
      return EXIT_FAILURE;
    }

    //recive/////////////////////////////////////////////////////////////////////
    //header
    headerInfo headerData;
    string headerBody;
    if (!fetchHeader(headerBody, sockfd))
      return false;
    if (!scanHeader(headerBody, headerData))
      return false;
    //entity
    switch (headerData.showAction())
    {
      case ok:
	if (!fetchEntity(headerData, filename, sockfd))
	  return EXIT_FAILURE;
	close(sockfd);
	return EXIT_SUCCESS;
      case redirect:
	uri = headerData.showURI();
	break;
      case error:
      default:
	close(sockfd);
	return EXIT_FAILURE;
    }
    close(sockfd);
  } while (redirCount-- > 0);

  errPrint("number of redirections is higher than 5");
  return EXIT_FAILURE;
}
