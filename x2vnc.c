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
 *
 *  Seriously modified by Fredrik Hübinette <hubbe@hubbe.net>
 */

/*
 * x2vnc - control VNC displays without showing them
 */

#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <x2vnc.h>

int temp_file_fd=0;

int main(int argc, char **argv)
{
  fd_set fds;
  struct timeval tv, *tvp;
  int msWait;
  
  processArgs(argc, argv);
  
  if (listenSpecified) {
    
    listenForIncomingConnections();
    /* returns only with a succesful connection */
    
  }
  else
  {
    if(reconnect)
    {
      long last_fork=0;
      char *tmpdir="/tmp";
      char tmpfile[1024];
      if(getenv("TMPDIR") && strlen(tmpdir) < 900)
	tmpdir=getenv("TMPDIR");

      sprintf(tmpfile, "%s/x2vnc-%d-%d",
	      tmpdir,
	      getpid(),
	      time(0));

      temp_file_fd=open(tmpfile, O_RDWR | O_CREAT | O_EXCL, 0600);
      unlink(tmpfile);

      while(1)
      {
	int status, pid;

	/* limit how often we restart */
	if(time(0) - last_fork < 1) sleep(2);

	last_fork=time(0);
	switch (pid=fork())
	{
	  case -1: 
	    perror("fork"); 
	    exit(1);
	    
	  case 0:
	    break;
	    
	  default:
	    while(waitpid(pid, &status, 0) < 0 && errno==EINTR);
            if(debug)
              fprintf(stderr,"Child exited with status %d\n",status);
	    continue;
	}
	break;
      }
    }

    if (!ConnectToRFBServer(hostname, port)) exit(1);
  }
  
  if (!InitialiseRFBConnection(rfbsock)) exit(1);
  
  if (!CreateXWindow()) exit(1);
  
  if (!SetFormatAndEncodings()) {
    exit(1);
  }
  
  
  while (1)
  {
    /*
     * Always handle all X events before doing select.  This is the
     * simplest way of ensuring that we don't block in select while
     * Xlib has some events on its queue.
     */
    
    if (!HandleXEvents()) {
      exit(1);
    }
    
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    
    FD_ZERO(&fds);
    FD_SET(ConnectionNumber(dpy),&fds);
    FD_SET(rfbsock,&fds);
    
    if (select(FD_SETSIZE, &fds, NULL, NULL, &tv) < 0) {
      perror("select");
      exit(1);
    }
    
    if (FD_ISSET(rfbsock, &fds)) {
      if (!HandleRFBServerMessage()) {
		exit(1);
	    }
	}
    }

    return 0;
}
