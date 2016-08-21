/*
 * webclFunc.c
 * Functions implementation for weblicent.c
 * declarations in webclient.h
 * Jan Wrona, xwrona00@stud.fit.vutbr.cz
 * project #1, IPK
 */
#include "webclient.h"
using namespace std;

//debug
void printAddrinfo(addrinfo * res)
{
  for (addrinfo * addr = res; addr != 0; addr = addr->ai_next)
  {
    PR(addr->ai_flags);
    PR(addr->ai_family);
    PR(addr->ai_socktype);
    PR(addr->ai_protocol);
    PR(addr->ai_addrlen);
    PR(addr->ai_addr);
    PR(addr->ai_next);
    if (addr->ai_canonname != 0)
      PR(addr->ai_canonname);
    cout << endl;
  }
}

//implementation
headerInfo::headerInfo()
{
  action = codeNotSet;
  length = lengthNotSet;
  contentLength = 0;
}

void headerInfo::setAction(int code)
{
  switch (code)
  {
    case 200:
      action = ok;
      return;
    case 301:
    case 302:
      action = redirect;
      return;
    default:
      action = error;
      break;
  }
  errPrint("error or unimplemented status code");
  cerr << "code = " << code << endl;
}

codeType headerInfo::showAction()
{
  if (action != codeNotSet)
    return action;

  errPrint("showAction(): action was not set");
  return codeNotSet;
}

void headerInfo::setLengthType(lengthType passedLen)
{
  length = passedLen;
}

lengthType headerInfo::showLengthType()
{
  if (length != lengthNotSet)
    return length;

  errPrint("showLengthType(): length type was not set");
  return lengthNotSet;
}

void headerInfo::setLength(int passedLen)
{
  if (passedLen != 0)
    contentLength = passedLen;
  else
    errPrint("setLength(): passed zero length");
}

int headerInfo::showLength()
{
  if (contentLength != 0)
    return contentLength;

  errPrint("showLength(): length was not set");
  return 0;
}

void headerInfo::setURI(const string URI)
{
  if (URI.length() == 0)
  {
    errPrint("setURI(): passed URI is empty");
    return;
  }
  redirURI = URI;
}

string headerInfo::showURI()
{
  if (redirURI.length() == 0)
    errPrint("showURI(): URI was not set");
  return redirURI;
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

    if (sockfd == -1)
    {
      errPrint("could not create socket");
      return -1;
    }

    if (connect(sockfd, loopres->ai_addr, loopres->ai_addrlen) == -1)
      close(sockfd);
    else
      break;
  }

  if (loopres == 0)
  {
    errPrint("could not connect");
    perror("connect");
    return -1;
  }

  return sockfd;
}

bool makeParams(int argc, char *argv[], string & uri)
{
  if (argc != 2)
  {
    cerr << "Usage: " << argv[0] << " URL" << endl;
    return false;
  }

  uri = argv[1];
  return true;
}

bool parseURI(const string uri, string & port, string & host, string & path, string & filename)
{
  int regRes = 0;
  string
    regex("^([[:alnum:]]+)://(([[:alnum:]-]+\\.)+[[:alpha:]]+)(:([[:digit:]]{1,5}))?((/[^/]*)*)$");
  regex_t preg;

  regRes = regcomp(&preg, regex.c_str(), REG_EXTENDED);
  if (regRes != 0)
  {
    errPrint("regcomp() err");
    regfree(&preg);
    return false;
  }

  const int nmatch = 8;
  regmatch_t pmatch[nmatch];

  regRes = regexec(&preg, uri.c_str(), nmatch, pmatch, 0);
  regfree(&preg);
  if (regRes)
  {
    cerr << "wrong URI: " << uri << endl;
    return false;
  }

  int i = 0;
  //pmatch[0] -> whole string
  i++;				//1
  if (uri.substr(0, pmatch[i].rm_eo) == "http")
    port = "80";
  else
  {
    errPrint("protocol not suported");
    return false;
  }

  i++;				//2
  host = uri.substr(pmatch[i].rm_so, (pmatch[i].rm_eo - pmatch[i].rm_so));
  //pmatch[3] -> second-level domain
  //pmatch[4] -> port with colon
  i = 5;
  if (pmatch[i].rm_so != -1)
    port = uri.substr(pmatch[i].rm_so, (pmatch[i].rm_eo - pmatch[i].rm_so));

  i++;				//6
  if (pmatch[i + 1].rm_so == -1)
    path = "/";
  else
    path = uri.substr(pmatch[i].rm_so, (pmatch[i].rm_eo - pmatch[i].rm_so));

  i++;				//7
  if ((pmatch[i].rm_so == -1) || (pmatch[i].rm_eo - (pmatch[i].rm_so + 1)) == 0)
    filename = "index.html";
  else
    filename = uri.substr((pmatch[i].rm_so + 1), (pmatch[i].rm_eo - pmatch[i].rm_so));

  return true;
}

bool scanHeader(const string & body, headerInfo & headerData)
{
  int statCode, majorVer, minorVer, index = 0;
  if ((sscanf(body.c_str(), "HTTP/%d.%d %d", &majorVer, &minorVer, &statCode)) != 3)
  {
    errPrint("unknown header received");
    return false;
  }
  headerData.setAction(statCode);

  if (statCode == 301 || statCode == 302)
  {
    int index = 0;
    if ((index = body.find("Location: ")) == body.npos)
    {
      errPrint("redirection but location not set");
      return false;
    }
    index += strlen("Location: ");
    const int endIndex = (body.find("\r\n", index) - index);
    string line(body.substr(index, endIndex));
    headerData.setURI(line);
    return true;
  }

  if ((body.find("Transfer-Encoding: chunked")) != body.npos
      || (body.find("transfer-encoding: chunked")) != body.npos)
  {
    headerData.setLengthType(chunked);
    return true;
  }
  else if ((index = body.find("Content-Length: ")) != body.npos)
  {
    string line(body.substr(index));
    int length = 0;
    if (sscanf(line.c_str(), "Content-Length: %d", &length) != 1)
    {
      errPrint("unknown message length");
      return false;
    }
    headerData.setLengthType(contentLength);
    headerData.setLength(length);
    return true;
  }
  else
  {
    headerData.setLengthType(none);
    return true;
  }

  return false;
}

bool fetchHeader(string & headerBody, const int sockfd)
{
  char rcvdByte = 0;
  int rcvdNow = 0;

  while ((rcvdNow = recv(sockfd, (char *) &rcvdByte, sizeof(rcvdByte), 0)) != -1)
  {
    if (rcvdNow == 0)
      continue;

    headerBody += rcvdByte;
    if (headerBody.find("\r\n\r\n") != headerBody.npos)
      return true;
  }
  errPrint("header fetching not successfull");
  return false;
}

bool fetchEntity(headerInfo & headerData, string & filename, int sockfd)
{
  const int buffsize = 1024;
  int rcvd = 0, rcvdNow = 0;
  char rcvmsg[buffsize] = { 0 }, rcvdByte = 0;
  ofstream out(filename.c_str());

  switch (headerData.showLengthType())
  {
    case none:
      while ((rcvdNow = recv(sockfd, rcvmsg, sizeof(rcvmsg) / sizeof(rcvmsg[0]), 0)) > 0)
	out.write(rcvmsg, rcvdNow);
      break;
    case chunked:
      {
	string chunkStr;
	int chunkSize = 0;
	bool readChunkSize = true;

	while ((rcvdNow = recv(sockfd, (char *) &rcvdByte, sizeof(rcvdByte), 0)) != -1)
	{
	  if (rcvdNow == 0)
	    continue;

	  if (readChunkSize)
	  {
	    chunkStr += rcvdByte;
	    if (chunkStr.find("\r\n") == 0)
	      chunkStr.erase(0, 1);

	    if (chunkStr.find("\r\n") != chunkStr.npos)
	    {
	      if ((chunkSize = strtol(chunkStr.c_str(), 0, 16)) == 0)
		break;

	      chunkStr.erase();
	      rcvd = 0;
	      readChunkSize = false;
	    }
	  }
	  else
	  {
	    out.put(rcvdByte);
	    if ((rcvd += rcvdNow) == chunkSize)
	      readChunkSize = true;
	  }
	}
	break;
      }
    case contentLength:
      while ((rcvdNow = recv(sockfd, rcvmsg, sizeof(rcvmsg) / sizeof(rcvmsg[0]), 0)) != -1)
      {
	out.write(rcvmsg, rcvdNow);
	if ((rcvd += rcvdNow) >= headerData.showLength())
	  break;
      }
      break;
    default:
      return false;
  }
  return true;
}

void escapize(string & str)
{
  stringstream escapized;

  for (unsigned int i = 0; i < str.length(); i++)
  {
    if (illegalChar(str[i]))
      escapized << '%' << hex << (int) str[i];
    else
      escapized << str[i];
  }
  str = escapized.str();
}

bool illegalChar(const char &ch)
{
  if (((int) ch >= 0 && (int) ch <= 32) || (int) ch >= 127)
    return true;
  return false;
}

inline void errPrint(const char *message)
{
  cerr << message << endl;
}
