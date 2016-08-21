#include "server.h"

using namespace std;

clientArgs::clientArgs()
{
  searchCrit = none;
  searchArgs.clear();
  for (unsigned int i = 0; i < (sizeof(prArgs) / sizeof(prArgs[0])); i++)
    prArgs[i] = 0;
}

bool clientArgs::searchByLogin()
{
  if (searchCrit == none || searchCrit == uid)
    return false;
  else
    return true;
}

bool clientArgs::searchByUID()
{
  if (searchCrit == none || searchCrit == login)
    return false;
  else
    return true;
}

void clientArgs::pushArg(string arg)
{
  searchArgs.push_back(arg);
}

string clientArgs::popArg()
{
  if (searchArgs.empty())
  {
    cerr << "INTERNAL ERR: searchArgs is empty" << endl;
    exit(EXIT_FAILURE);
  }

  string ret(searchArgs.front());
  searchArgs.erase(searchArgs.begin());
  return ret;
}


//vicenasobne zadani stejneho argumentu je OK, pocita se ten posledni
bool makeOpt(const int argc, char *argv[], char *retPort)
{
  if (argc < 2)
  {
    cerr << "Missing port number." << endl;
    return false;
  }
  int opt = 0;
  string port;
  while ((opt = getopt(argc, argv, "p:")) != -1)
  {
    switch (opt)
    {
      case 'p':
	port = optarg;
	break;
      case '?':
	return false;
      default:
	cerr << "Unknown getopt() error!" << endl;
	return false;
    }
  }

  char *endptr;
  long int longPort;
  errno = 0;

  longPort = strtol(port.c_str(), &endptr, 10);

  if (errno == ERANGE || (errno != 0 && longPort == 0))
  {				//result out of rande
    cerr << "Port out of range." << endl;
    return false;
  }
  if (endptr == port.c_str())
  {				//no digits found
    cerr << "Wrong port number." << endl;
    return false;
  }
  if (*endptr != '\0')
  {				//digits + aditional characters
    cerr << "Wrong port number." << endl;
    return false;
  }

  if (longPort < 0 || longPort > 65535)
  {				//strtol OK but invalid port number
    cerr << "Wrong port number." << endl;
    return false;
  }
  strcpy(retPort, port.c_str());
  return true;
}

addrinfo *getSockAddr(const char *service, const int family, const int socktype,
		      const int protocol, const int flags)
{
  addrinfo hints;
  memset(&hints, 0, sizeof(addrinfo));
  addrinfo *res = 0;

  hints.ai_family = family;
  hints.ai_socktype = socktype;
  hints.ai_protocol = protocol;
  hints.ai_flags = flags;

  int errcode = getaddrinfo(0, service, &hints, &res);
  if (errcode)
  {
    cerr << "getaddrinfo(): " << gai_strerror(errcode) << endl;
    return 0;
  }

  return res;
}

int sockAndBind(addrinfo * res)
{
  addrinfo *loopres;
  int sockfd = -1;

  for (loopres = res; loopres != 0; loopres = loopres->ai_next)
  {
    if ((sockfd = socket(loopres->ai_family, loopres->ai_socktype, loopres->ai_protocol)) == -1)
      continue;

    if (bind(sockfd, loopres->ai_addr, loopres->ai_addrlen) == 0)
      break;

    close(sockfd);
  }

  if (sockfd == -1)
  {
    perror("could not create socket");
    return -1;
  }

  if (loopres == 0)
  {
    perror("could not bind");
    return -1;
  }

  return sockfd;
}

bool mySigaction()
{
  struct sigaction act;

  memset(&act, 0, sizeof(act));
  act.sa_handler = sigchldHandler;
  act.sa_flags = SA_RESTART;	//tady vyzkouset jeste jine flags

  if (sigaction(SIGCHLD, &act, 0) == -1)
  {
    perror("sigaction");
    return false;
  }

  memset(&act, 0, sizeof(act));
  act.sa_handler = sigintHandler;
  act.sa_flags = SA_RESTART;

  if (sigaction(SIGINT, &act, 0) == -1)
  {
    perror("sigaction");
    return false;
  }
  return true;
  //TODO: jeste SIGTERM
}

void sigchldHandler(int)
{
  //TODO: chce to poresit exit statusy childs pres waitpid
  waitpid(-1, 0, 0);
}

void sigintHandler(int)
{
  extern int sockfd;
  close(sockfd);
  exit(EXIT_SUCCESS);
}

string fetchMsg(const int sockfd)
{
  char rcvdByte = 0;
  string sizeStr, message;
  int rcvdNow = 0;

  while ((rcvdNow = recv(sockfd, (char *) &rcvdByte, sizeof(rcvdByte), 0)) != -1)
  {
    if (rcvdNow == 0)
      continue;
    else if (rcvdByte == '\n')
      break;
    else
      sizeStr += rcvdByte;
  }

  unsigned int length = 0;
  if ((sscanf(sizeStr.c_str(), "msglen=%x", &length)) != 1)
  {
    perror("sscanf");
    return "0";
  }

  const int buffsize = 128;
  unsigned int rcvd = 0;
  char rcvmsg[buffsize] = { '\0' };
  while ((rcvdNow = recv(sockfd, rcvmsg, sizeof(rcvmsg) / sizeof(rcvmsg[0]), 0)) != -1)
  {
    message.append(rcvmsg, rcvdNow);
    if ((rcvd += rcvdNow) >= length)
      break;
  }
  return message;
}

bool parseMsg(string & msg, clientArgs & args)
{
  const int maxCrLen = 6;
  char criterion[maxCrLen] = { '\0' };
  if ((sscanf(msg.c_str(), "criterion=%s\n", criterion)) != 1)
  {
    cerr << "Parsing error." << endl;
    return false;
  }

  if (strcmp(criterion, "login") == 0)
    args.setSearchByLogin();
  else if (strcmp(criterion, "uid") == 0)
    args.setSearchByUID();
  else
  {
    cerr << "Parsing error." << endl;
    return false;
  }

  msg.erase(0, msg.find('\n') + 1);
  if (msg.find("arguments=") != 0)
  {
    cerr << "Parsing error." << endl;
    return false;
  }
  msg.erase(0, strlen("arguments="));
  size_t pos = 0;
  while ((pos = msg.find(',')) != msg.npos)
  {
    args.pushArg(msg.substr(0, pos));
    msg.erase(0, pos + 1);
  }
  msg.erase(0, 1);		// '\n'

  const int prCount = 6;
  const int prLen = prCount + 1;	//because of '\0'
  char prArr[prLen] = { '\0' };
  if ((sscanf(msg.c_str(), "prArgs=%s\n", prArr)) != 1)
  {
    cerr << "Parsing error." << endl;
    return false;
  }

  for (int i = 0; i < prCount; i++)
  {
    int tmp = prArr[i] - '0';
    args.prArgs[i] = tmp;
  }

  return true;
}

bool generateMsg(clientArgs & args, string & message)
{
  for (unsigned int i = 0; i < args.searchArgs.size(); i++)
  {
    struct passwd pw, *pwRes = 0;
    const int buffLen = 4096;
    char buf[buffLen];
    bool found = false;
    setpwent();

    if (args.searchByLogin())
    {
      while (true)
      {
	int ret = getpwent_r(&pw, buf, buffLen, &pwRes);	//mem leak
	//cout << "ret = " << ret << "\tpwRes = " << (double*)pwRes << endl;
	if ((ret == 0 || ret == ENOENT) && pwRes == 0)
	{
	  if (!found)
	    message += string("Error: unknown login '") + args.searchArgs[i] + string("'\n");
	  break;
	}
	else if (ret == ERANGE)
	{
	  cerr << "getpwent_r: nou inaf bafr" << endl;
	  return false;
	}
	else if (args.searchArgs[i] == pwRes->pw_name)
	{
	  found = true;
	  message += stringizePwEnt(args, pwRes);
	  if (!message.empty())
	    message.replace(message.length() - 1, 1, 1, '\n'); //replaces last space by NL
	}
      }
    }
    else if (args.searchByUID())
    {
      uid_t uid = static_cast < uid_t > (atoi(args.searchArgs[i].c_str()));
      while (true)
      {
	int ret = getpwent_r(&pw, buf, buffLen, &pwRes);	//mem leak
	if ((ret == 0 || ret == ENOENT) && pwRes == 0)
	{
	  if (!found)
	    message += string("Error: unknown UID '") + args.searchArgs[i] + string("'\n");
	  break;
	}
	else if (ret == ERANGE)
	{
	  cerr << "getpwent_r: nou inaf bafr" << endl;
	  return false;
	}
	else if (uid == pwRes->pw_uid)
	{
	  found = true;
	  message += stringizePwEnt(args, pwRes);
	  if (!message.empty())
	    message.replace(message.length() - 1, 1, 1, '\n');
	}
      }
    }
  }
  endpwent();

  ostringstream length;
  length << hex << message.size();
  message = string("msglen=") + length.str() + '\n' + message;
  return true;
}

string stringizePwEnt(const clientArgs & args, const struct passwd *pwRes)
{
  ostringstream ret;
  int pos;

  for (int value = 1; value < 7; value++)
  {
    for (pos = 0; pos < args.prCount; pos++)
    {
      if (args.prArgs[pos] == value)
      {
	switch (pos)
	{
	  case 0:
	    ret << pwRes->pw_name << ' ';
	    break;
	  case 1:
	    ret << pwRes->pw_uid << ' ';
	    break;
	  case 2:
	    ret << pwRes->pw_gid << ' ';
	    break;
	  case 3:
	    ret << pwRes->pw_gecos << ' ';
	    break;
	  case 4:
	    ret << pwRes->pw_dir << ' ';
	    break;
	  case 5:
	    ret << pwRes->pw_shell << ' ';
	    break;
	}
      }
    }
  }
  return ret.str();
}
