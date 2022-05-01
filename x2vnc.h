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
 * vncviewer.h
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <X11/Xlib.h>
#include <X11/Xmd.h>
#include <rfbproto.h>


extern int endianTest;

#define Swap16IfLE(s) \
    (*(char *)&endianTest ? ((((s) & 0xff) << 8) | (((s) >> 8) & 0xff)) : (s))

#define Swap32IfLE(l) \
    (*(char *)&endianTest ? ((((l) & 0xff000000) >> 24) | \
			     (((l) & 0x00ff0000) >> 8)  | \
			     (((l) & 0x0000ff00) << 8)  | \
			     (((l) & 0x000000ff) << 24))  : (l))

#define MAX_ENCODINGS 10


/* args.c */

extern char *programName;
extern char hostname[];
extern int port;
extern Bool listenSpecified;
extern int listenPort, flashPort;
extern char *displayname;
extern Bool shareDesktop;
extern Bool viewOnly;
extern CARD32 explicitEncodings[];
extern int nExplicitEncodings;
extern Bool addCopyRect;
extern Bool addRRE;
extern Bool addCoRRE;
extern Bool addHextile;
extern Bool useBGR233;
extern Bool forceOwnCmap;
extern Bool forceTruecolour;
extern int requestedDepth;
extern char *geometry;
extern int wmDecorationWidth;
extern int wmDecorationHeight;
extern char *passwdFile;
extern int updateRequestPeriodms;
extern int updateRequestX;
extern int updateRequestY;
extern int updateRequestW;
extern int updateRequestH;
extern int rawDelay;
extern int copyRectDelay;
extern Bool debug;
extern Bool trimsel;
extern Bool resurface;
extern Bool reconnect;
extern int temp_file_fd;

extern int noblank;
extern int no_wakeup_delay;
extern int last_event_time;
extern float acceleration;

extern void processArgs(int argc, char **argv);
extern void usage();


/* rfbproto.c */

extern int rfbsock;
extern Bool canUseCoRRE;
extern Bool canUseHextile;
extern char *desktopName;
extern rfbPixelFormat myFormat;
extern rfbServerInitMsg si;
extern struct timeval updateRequestTime;
extern Bool sendUpdateRequest;

extern Bool ConnectToRFBServer(const char *hostname, int port);
extern Bool InitialiseRFBConnection();
extern Bool SetFormatAndEncodings();
extern Bool SendIncrementalFramebufferUpdateRequest();
extern Bool SendFramebufferUpdateRequest(int x, int y, int w, int h,
					 Bool incremental);
extern Bool SendPointerEvent(int x, int y, int buttonMask);
extern Bool SendKeyEvent(CARD32 key, Bool down);
extern Bool SendClientCutText(char *str, int len);
extern Bool HandleRFBServerMessage();


/* x.c */

enum edge_enum
{
  EDGE_EAST,
  EDGE_WEST,
  EDGE_NORTH,
  EDGE_SOUTH
};

extern enum edge_enum edge;
extern Display *dpy;
extern unsigned long BGR233ToPixel[];

extern Bool CreateXWindow();
extern Bool HandleXEvents();
extern Bool AllXEventsPredicate(Display *dpy, XEvent *ev, char *arg);


/* sockets.c */

extern Bool errorMessageFromReadExact;

extern Bool ReadExact(int sock, char *buf, int n);
extern Bool WriteExact(int sock, char *buf, int n);
extern int ListenAtTcpPort(int port);
extern int getFreePort();
extern int ConnectToTcpAddr(const char *host, int port);
extern int AcceptTcpConnection(int listenSock);
extern int StringToIPAddr(const char *str, unsigned int *addr);
extern Bool SameMachine(int sock);
extern Bool useSSHTunnel;
extern char *useSSHGateway;
extern char *sshUser;
extern int sshPort;


/* listen.c */

extern void listenForIncomingConnections();
