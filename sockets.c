/*
 *  Copyright (C) 1997, 1998 Olivetti & Oracle Research Laboratory
 *
 *  This is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this software; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
 *  USA.
 */

/*
 * sockets.c - functions to deal with sockets.
 */

#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <x2vnc.h>

void PrintInHex(char *buf, int len);

Bool errorMessageFromReadExact = True;
Bool useSSHTunnel;
char *useSSHGateway;
char *sshUser;
int sshPort = 22;

/*
 * Read an exact number of bytes, and don't return until you've got them.
 */

Bool
ReadExact(int sock, char *buf, int n)
{
    int i = 0;
    int j;

    while (i < n) {
	j = read(sock, buf + i, (n - i));
	if (j <= 0) {
	    if (j < 0) {
		fprintf(stderr,"%s",programName);
		perror(": read");
	    } else {
		if (errorMessageFromReadExact) {
		    fprintf(stderr,"%s: read failed\n",programName);
		}
	    }
	    return False;
	}
	i += j;
    }
    if (debug)
	PrintInHex(buf,n);
    return True;
}


/*
 * Write an exact number of bytes, and don't return until you've sent them.
 */

Bool
WriteExact(int sock, char *buf, int n)
{
    int i = 0;
    int j;

    while (i < n) {
	j = write(sock, buf + i, (n - i));
	if (j <= 0) {
	    if (j < 0) {
		fprintf(stderr,"%s",programName);
		perror(": write");
	    } else {
		fprintf(stderr,"%s: write failed\n",programName);
	    }
	    return False;
	}
	i += j;
    }
    return True;
}


/*
 * ConnectToTcpAddr connects to the given TCP port.
 */

int
ConnectToTcpAddr(const char *hostname, int port)
{
  int sock = -1;
  int one = 1;
  
  struct addrinfo hints;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_NUMERICSERV;

  char portstr[3*sizeof(int)+1];
  if(useSSHTunnel)
  {
    const char *remote, *gateway;
    if(useSSHGateway)
    {
      gateway=useSSHGateway;
      remote=hostname;
    }else{
      gateway=hostname;
      remote="localhost";
    }
    sprintf(portstr, "%i", tunnel(gateway, remote, port));
    hostname = 0; /* Request loopback address from getaddrinfo() */
  } else {
    sprintf(portstr, "%i", port);
  }

  struct addrinfo *res;
  int eai = getaddrinfo(hostname, portstr, &hints, &res);
  if (eai) {
      if (eai == EAI_SYSTEM) {
	  fprintf(stderr,"%s",programName);
	  perror(": ConnectToTcpAddr: getaddrinfo");
      } else {
	  fprintf(stderr, "%s: ConnectToTcpAddr: getaddrinfo: %s\n", programName, gai_strerror(eai));
	  return -1;
      }
  }
  struct addrinfo *ai;
  for (ai = res; ai; ai = ai->ai_next) {
    sock = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
    if (sock < 0) {
      fprintf(stderr,"%s",programName);
      perror(": ConnectToTcpAddr: socket");
      continue;
    }
  
    if (connect(sock, ai->ai_addr, ai->ai_addrlen) < 0) {
	fprintf(stderr,"%s",programName);
	perror(": ConnectToTcpAddr: connect");
	close(sock);
	sock = -1;
    } else {
	break;
    }
  }
  freeaddrinfo(res);
  if (sock < 0) {
    fprintf(stderr,"%s: Could not connect to any address\n", programName);
    return -1;
  }
  
  if (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *)&one, sizeof(one)) < 0) {
    fprintf(stderr,"%s",programName);
    perror(": ConnectToTcpAddr: setsockopt");
    close(sock);
    return -1;
  }
  
  return sock;
}



/*
 * ListenAtTcpPort starts listening at the given TCP port.
 */

int
ListenAtTcpPort(int port)
{
    int sock = -1;
    int one = 1, zero = 0;
    
    struct sockaddr_storage addr;
    socklen_t addrlen;

    for (addr.ss_family = AF_INET6; sock < 0 && addr.ss_family;
	 addr.ss_family = (addr.ss_family == AF_INET6 ? AF_INET : 0)) {

	sock = socket(addr.ss_family, SOCK_STREAM, 0);
	if (sock < 0) {
	    fprintf(stderr,"%s",programName);
	    perror(": ListenAtTcpPort: socket");
	    continue;
	}

	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
		       (const char *)&one, sizeof(one)) < 0) {
	    fprintf(stderr,"%s",programName);
	    perror(": ListenAtTcpPort: setsockopt");
	    close(sock); sock = -1;
	    continue;
	}

	if (addr.ss_family == AF_INET6) {
	    struct sockaddr_in6 *addr6 = (struct sockaddr_in6*)&addr;
	    addr6->sin6_port = htons(port);
	    memcpy(addr6->sin6_addr.s6_addr, &in6addr_any, sizeof(in6addr_any));
	    addrlen = sizeof(struct sockaddr_in6);
	    if (setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY,
			   &zero, sizeof(zero)) < 0) {
		fprintf(stderr,"%s",programName);
		perror(": Warning: ListenAtTcpPort: setsockopt");
	    }
	} else {
	    struct sockaddr_in *addr4 = (struct sockaddr_in *)&addr;
	    addr4->sin_port = htons(port);
	    addr4->sin_addr.s_addr = INADDR_ANY;
	    addrlen = sizeof(struct sockaddr_in);
	}

	if (bind(sock, (struct sockaddr *)&addr, addrlen) < 0
	    && (addr.ss_family != AF_INET6
		|| setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY,
			      &one, sizeof(one)) < 0
		|| bind(sock, (struct sockaddr *)&addr, addrlen) < 0)) {
	    fprintf(stderr,"%s",programName);
	    perror(": ListenAtTcpPort: bind");
	    close(sock); sock = -1;
	    continue;
	}

	if (listen(sock, 5) < 0) {
	    fprintf(stderr,"%s",programName);
	    perror(": ListenAtTcpPort: listen");
	    close(sock); sock = -1;
	    continue;
	}
    }

    return sock;
}

int getFreePort(void)
{
  int sock=-1;
  struct sockaddr_in addr;
  int one = 1;
  int x;
  static int last;

  for(x=0;x<100;x++)
  {
    int port = 5500 + last % 100;
    char portstr[3*sizeof(int)+1];
    sprintf(portstr, "%i", port);
    last+=4711;

    struct addrinfo *res = 0;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;
  
    int eai = getaddrinfo(0, portstr, &hints, &res);
    if (eai) {
	fprintf(stderr, "%s: getaddrinfo() failed when finding a free port: %s\n",
		programName, gai_strerror(eai));
	return -1;
    }
    for (struct addrinfo *ai = res; ai; ai = ai->ai_next) {
	sock = socket(ai->ai_family, SOCK_STREAM, 0);
	if (sock < 0 ||
	    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
		       &one, sizeof(one)) < 0 ||
	    (ai->ai_family == AF_INET6 && setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY,
						     &one, sizeof(one)) < 0) ||
	    bind(sock, ai->ai_addr, ai->ai_addrlen) < 0) {
	    close(sock);
	    sock = -1;
	    freeaddrinfo(res);
	    break;
	} else {
	    close(sock);
	}
    }
    if (sock > 0) {
	return port;
    }
  }
  return -1;
}


/*
 * AcceptTcpConnection accepts a TCP connection.
 */

int
AcceptTcpConnection(int listenSock)
{
    int sock;
    struct sockaddr_storage addr;
    int addrlen = sizeof(addr);
    int one = 1;

    sock = accept(listenSock, (struct sockaddr *) &addr, &addrlen);
    if (sock < 0) {
	fprintf(stderr,"%s",programName);
	perror(": AcceptTcpConnection: accept");
	return -1;
    }

    if (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY,
		   (char *)&one, sizeof(one)) < 0) {
	fprintf(stderr,"%s",programName);
	perror(": AcceptTcpConnection: setsockopt");
	close(sock);
	return -1;
    }

    return sock;
}


/*
 * StringToIPAddr - convert a host string to an IP address.
 */

int
StringToIPAddr(const char *str, unsigned int *addr)
{
    struct hostent *hp;

    if ((*addr = inet_addr(str)) == -1)
    {
	if (!(hp = gethostbyname(str)))
	    return 0;

	*addr = *(unsigned int *)hp->h_addr;
    }

    return 1;
}


/*
 * Test if the other end of a socket is on the same machine.
 */

Bool
SameMachine(int sock)
{
    struct sockaddr_in peeraddr, myaddr;
    int addrlen = sizeof(struct sockaddr_in);

    getpeername(sock, (struct sockaddr *)&peeraddr, &addrlen);
    getsockname(sock, (struct sockaddr *)&myaddr, &addrlen);

    return (peeraddr.sin_addr.s_addr == myaddr.sin_addr.s_addr);
}


/*
 * Print out the contents of a packet for debugging.
 */

void
PrintInHex(char *buf, int len)
{
    int i, j;
    char c, str[17];

    str[16] = 0;

    fprintf(stderr,"ReadExact: ");

    for (i = 0; i < len; i++)
    {
	if ((i % 16 == 0) && (i != 0)) {
	    fprintf(stderr,"           ");
	}
	c = buf[i];
	str[i % 16] = (((c > 31) && (c < 127)) ? c : '.');
	fprintf(stderr,"%02x ",(unsigned char)c);
	if ((i % 4) == 3)
	    fprintf(stderr," ");
	if ((i % 16) == 15)
	{
	    fprintf(stderr,"%s\n",str);
	}
    }
    if ((i % 16) != 0)
    {
	for (j = i % 16; j < 16; j++)
	{
	    fprintf(stderr,"   ");
	    if ((j % 4) == 3) fprintf(stderr," ");
	}
	str[i % 16] = 0;
	fprintf(stderr,"%s\n",str);
    }

    fflush(stderr);
}


int tunnel(char *gatewayhost, char *remotehost, int remoteport)
{
  char *space, *cmd;
  int port=getFreePort();

  space=malloc( (gatewayhost ? strlen(gatewayhost) : strlen(remotehost)) + strlen(remotehost) + 1024);
  if(!space)
  {
    fprintf(stderr,"Failed to malloc environment buffer!\n");
    exit(1);
  }

  sprintf(space,"LOCALPORT=%d",port);
  putenv(space);
  space+=strlen(space)+1;

  sprintf(space,"REMOTEHOST=%s",remotehost);
  putenv(space);
  space+=strlen(space)+1;
  
  sprintf(space,"REMOTEPORT=%d",remoteport);
  putenv(space);
  space+=strlen(space)+1;
  
  if(gatewayhost)
  {
      if(sshUser)
      {
        sprintf(space,"GATEWAYHOST=%s@%s",sshUser,gatewayhost);
      }
      else
      {
        sprintf(space,"GATEWAYHOST=%s",gatewayhost);
      }
  }
  else
  {
    sprintf(space,"GATEWAYHOST=%s",remotehost);
  }
  putenv(space);
  space+=strlen(space)+1;

  sprintf(space,"GATEWAYPORT=%d",sshPort);
  putenv(space);
  space+=strlen(space)+1;

  cmd=getenv("X2VNC_SSH_CMD");
  /* Using -X tells ssh to turn off the NAGLE algorithm, which is required for any form of speed */
  if(!cmd) cmd="ssh -A -X -f -L \"$LOCALPORT:$REMOTEHOST:$REMOTEPORT\" \"$GATEWAYHOST\" -p $GATEWAYPORT 'ssh-add;sleep 60'";

  if(system(cmd) < 0)
  {
    perror("tunnel:system");
    exit(1);
  }
  return port;
}
