#include "server.h"
using namespace std;

int sockfd = -1;

int main(int argc, char *argv[])
{
  if (!mySigaction())
    return EXIT_FAILURE;

  const int maxPortLen = 5;
  char port[maxPortLen + 1] = {'\0'};
  if (!makeOpt(argc, argv, port))
    return EXIT_FAILURE;

  addrinfo *res = getSockAddr(port, PF_INET, SOCK_STREAM, 0, AI_PASSIVE);
  if (res == 0)
    return EXIT_FAILURE;

  sockfd = sockAndBind(res);
  freeaddrinfo(res);

  if (sockfd == -1)
    return EXIT_FAILURE;

  const int backlog = 2;
  if (listen(sockfd, backlog) == -1)
  {
    perror("listen");
    close(sockfd);
    return EXIT_FAILURE;
  }

  while (true)
  {
    int clientfd = accept(sockfd, 0, 0);
    if (clientfd == -1)
    {
      perror("accept");
      close(sockfd);
      return EXIT_FAILURE;
    }

    //fork////////////////////////////////////////

    pid_t childPid = fork();	//child process creation
    if (childPid == -1)
    {				//unsuccessfull fork()
      perror("fork");
      close(clientfd);
      close(sockfd);
      return EXIT_FAILURE;
    }

    if (childPid == 0)
    {				//code executed by child process(es) only
      string rcvdMsg = fetchMsg(clientfd);
      if (rcvdMsg == "0")
        return EXIT_FAILURE;

      clientArgs args;
      if (!parseMsg(rcvdMsg, args))
	  return EXIT_FAILURE;

      string message;
      generateMsg(args, message);

      if (send(clientfd, message.data(), message.size(), 0) == -1)
      {
        perror("send");
        return EXIT_FAILURE;
      }

      if (close(clientfd) == -1)
      {
	perror("close(clientfd)");
	return EXIT_FAILURE;
      }
      return EXIT_SUCCESS;
    }
    else
    { //code executed by parent process only
      if (close(clientfd) == -1)
      {
	perror("close(clientfd)");
	return EXIT_FAILURE;
      }
    }

    //fork end////////////////////////////////////
  }				//while(true)

  return EXIT_SUCCESS;
}
