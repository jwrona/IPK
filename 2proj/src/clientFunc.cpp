#include "client.h"

using namespace std;

clArgs::clArgs()
{
  port = -1;
  searchCrit = none;
  for (int i = 0; i < COUNT; i++)
    prArgs[i] = 0;

  flagCount = 1;
}

string clArgs::getHostname()
{
  if (hostname.empty())
  {
    cerr << "Missing hostname" << endl;
    exit(EXIT_FAILURE);
  }
  else
    return hostname;
}

bool clArgs::setPort(const char *charPort)
{
  char *endptr;
  long int longPort;
  errno = 0;

  longPort = strtol(charPort, &endptr, 10);

  //error check during strtol()
  if (errno == ERANGE || (errno != 0 && longPort == 0))
  {				//result out of rande
    cerr << "Port out of range." << endl;
    return false;
  }
  if (endptr == charPort)
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

  port = static_cast < int >(longPort);
  return true;
}

int clArgs::getPort()
{
  if (port == -1)
  {
    cerr << "Missing port number" << endl;
    exit(EXIT_FAILURE);
  }
  else
    return port;
}

string clArgs::getPortStr()
{
  if (port == -1)
  {
    cerr << "Missing port number" << endl;
    exit(EXIT_FAILURE);
  }

  ostringstream ss;
  ss << port;
  return ss.str();
}

bool clArgs::searchByLogin()
{
  if (searchCrit == none || searchCrit == uid)
    return false;
  else
    return true;
}

bool clArgs::searchByUID()
{
  if (searchCrit == none || searchCrit == login)
    return false;
  else
    return true;
}

void clArgs::pushArg(const char *arg)
{
  searchArgs.push_back(arg);
}

string clArgs::popArg()
{
  if (searchArgs.empty())
  {
    errPrint("INTERNAL ERR: searchArgs is empty");
    exit(EXIT_FAILURE);
  }

  string ret(searchArgs.front());
  searchArgs.erase(searchArgs.begin());
  return ret;
}

void clArgs::setLogin()
{
  if (prArgs[LOGIN] == 0)
    prArgs[LOGIN] = flagCount++;
}

void clArgs::setUID()
{
  if (prArgs[UID] == 0)
    prArgs[UID] = flagCount++;
}

void clArgs::setGID()
{
  if (prArgs[GID] == 0)
    prArgs[GID] = flagCount++;
}

void clArgs::setName()
{
  if (prArgs[NAME] == 0)
    prArgs[NAME] = flagCount++;
}

void clArgs::setHome()
{
  if (prArgs[HOME] == 0)
    prArgs[HOME] = flagCount++;
}

void clArgs::setShell()
{
  if (prArgs[SHELL] == 0)
    prArgs[SHELL] = flagCount++;
}

string clArgs::getPrArgs()
{
  ostringstream ss;
  for (int i = 0; i < COUNT; i++)
    ss << prArgs[i];

  return ss.str();
}

void clArgs::print()
{
  if (hostname.empty())
    cout << "hostname empty" << endl;
  else
    PR(hostname);

  PR(port);

  if (searchByLogin())
    cout << "search criterion: login" << endl;
  if (searchByUID())
    cout << "search criterion: UID" << endl;
  cout << "searchArgs: ";
  for (unsigned int i = 0; i < searchArgs.size(); i++)
    cout << searchArgs[i] << ' ';
  cout << endl;

  cout << "prArgs = " << getPrArgs() << endl;
  cout << endl;
}

//vicenasobne zadani stejneho argumentu je OK, pocita se ten posledni
bool makeOpt(const int argc, char *argv[], clArgs & args)
{
  int opt;
  while (true)
  {
    while ((opt = getopt(argc, argv, "h:p:luLUGNHS")) != -1)
    {
      switch (opt)
      {
	case 'h':
	  args.setHostname(optarg);
	  break;
	case 'p':
	  if (!args.setPort(optarg))
	    return false;
	  break;
	case 'l':
	  args.clearArg();
	  args.setSearchByLogin();
	  break;
	case 'u':
	  args.clearArg();
	  args.setSearchByUID();
	  break;

	case 'L':
	  args.setLogin();
	  break;
	case 'U':
	  args.setUID();
	  break;
	case 'G':
	  args.setGID();
	  break;
	case 'N':
	  args.setName();
	  break;
	case 'H':
	  args.setHome();
	  break;
	case 'S':
	  args.setShell();
	  break;

	case '?':
	  return false;
	default:
	  cerr << "Unknown getopt() error!" << endl;
	  return false;
      }
    }

    if (optind < argc)
    {
      if (optopt == 'l' || optopt == 'u')
      {
	while (optind < argc)
	{
	  if (argv[optind][0] == '-')
	    break;
	  else
	    args.pushArg(argv[optind++]);
	}
      }
      else
      {
	cerr << "Unknown parameter: " << argv[optind] << endl;
	return false;
      }
    }

    if (optind == argc)
      break;
  }

  if (args.getHostname().empty())
    return false;

  if (args.getPort() == -1)
    return false;

  if (!(args.searchByLogin() || args.searchByUID()))
  {
    cerr << "Missing search criterion" << endl;
    return false;
  }

  return true;
}

addrinfo *resolveURI(const string uri, const string service, const int family,
		     const int socktype, const int protocol, const int flags)
{
  addrinfo hints;
  memset(&hints, 0, sizeof(addrinfo));
  addrinfo *res = 0;

  hints.ai_family = family;
  hints.ai_socktype = socktype;
  hints.ai_protocol = protocol;
  hints.ai_flags = flags;

  int errcode = getaddrinfo(uri.c_str(), service.c_str(), &hints, &res);
  if (errcode)
  {
    cerr << "getaddrinfo(): " << gai_strerror(errcode) << endl;
    return 0;
  }

  return res;
}

int sockAndConn(addrinfo * res)
{
  addrinfo *loopres;
  int sockfd = -1;

  for (loopres = res; loopres != 0; loopres = loopres->ai_next)
  {
    if ((sockfd = socket(loopres->ai_family, loopres->ai_socktype, loopres->ai_protocol)) == -1)
      continue;

    if (connect(sockfd, loopres->ai_addr, loopres->ai_addrlen) == -1)
      close(sockfd);
    else
      break;
  }

  if (sockfd == -1)
  {
    errPrint("could not create socket");
    return -1;
  }

  if (loopres == 0)
  {
    errPrint("could not connect");
    perror("connect");
    return -1;
  }

  return sockfd;
}

inline void errPrint(const char *message)
{
  cerr << message << endl;
}

string generateRequest(clArgs & args)
{
  ostringstream request;
  if (args.searchByLogin())
    request << "criterion=login\n";
  else if (args.searchByUID())
    request << "criterion=uid\n";
  request << "arguments=";

  while (!args.emptyArg())
    request << args.popArg() << ',';
  request << '\n';

  request << "prArgs=" << args.getPrArgs() << '\n';

  ostringstream length;
  length << hex << request.tellp();

  string msg("msglen=" + length.str() + '\n' + request.str());
  return msg;
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

  const int buffsize = 512;
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
