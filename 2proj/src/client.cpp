#include "client.h"

int main(int argc, char *argv[])
{
  clArgs args;
  if (!makeOpt(argc, argv, args))
    return EXIT_FAILURE;

  //args.print();
  addrinfo *res = resolveURI(args.getHostname(), args.getPortStr(), PF_INET, SOCK_STREAM);
  if (res == 0) return EXIT_FAILURE;

  int sockfd = sockAndConn(res);
  freeaddrinfo(res);
  if (sockfd == -1) return EXIT_FAILURE;
  ///////////////////////////////////////////////////////////////////
  
  string request = generateRequest(args);
  if (send(sockfd, request.data(), request.length(), 0) == -1)
  {
    perror("send");
    return EXIT_FAILURE;
  }
  string response = fetchMsg(sockfd);
  if (response == "0")
    return EXIT_FAILURE;

  cout << response;

  ///////////////////////////////////////////////////////////////////
  close(sockfd);
  return EXIT_SUCCESS;
}
