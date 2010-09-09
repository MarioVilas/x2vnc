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
 * args.c - argument processing.
 */

#include <sys/utsname.h>
#include <x2vnc.h>

#define FLASHPORT  (5400)    /* Offset to listen for `flash' commands */
#define CLIENTPORT (5500)    /* Offset to listen for reverse connections */
#define SERVERPORT (5900)    /* Offset to server for regular connections */

char *programName;

char hostname[256];
int port;

Bool listenSpecified = False;
int listenPort = 0, flashPort = 0;

char *displayname = NULL;

Bool shareDesktop = False;

CARD32 explicitEncodings[MAX_ENCODINGS];
int nExplicitEncodings = 0;
Bool addCopyRect = True;
Bool addRRE = True;
Bool addCoRRE = True;
Bool addHextile = True;

Bool useBGR233 = False;
Bool forceOwnCmap = False;
Bool forceTruecolour = False;
Bool resurface = False;
Bool reconnect = True;
int requestedDepth = 0;
float acceleration = 1.0;

char *geometry = NULL;

int wmDecorationWidth = 4;
int wmDecorationHeight = 24;

char *passwdFile = NULL;

int updateRequestPeriodms = 0;

int updateRequestX = 0;
int updateRequestY = 0;
int updateRequestW = 0;
int updateRequestH = 0;

int rawDelay = 0;
int copyRectDelay = 0;

int debug=0;
Bool trimsel = False;

Bool noblank = False;
int no_wakeup_delay = 0x7fffffff;

void usage()
{
  fprintf(stderr,
          "x2vnc version " VERSION ", Copyright (C) 2000 Fredrik Hubinette\n"
          "Based on vncviewer which is copyright by AT&T\n"
          "x2vnc comes with ABSOLUTELY NO WARRANTY. This is free software,\n"
          "and you are welcome to redistribute it under certain conditions.\n"
          "See the file COPYING in the x2vnc source for details.\n"
          "\n"
          "usage: %s [<options>] <host>:<display#>\n"
          "       %s [<options>] -listen [<display#>]\n"
          "\n"
          "<options> are:\n"
          "              [-display <display>]\n"
          "              [-version]\n"
          "              [-shared]\n"
          "              [-north] [-south] [-east] [-west]\n"
          "              [-hotkey key]\n"
          "              [-passwdfile <passwd-file>]\n"
          "              [-resurface]\n"
          "              [-edgewidth width]\n"
          "              [-restingx 0|-1]\n"
          "              [-restingy 0|-1]\n"
          "              [-desktop desktop]\n"
          "              [-timeout seconds]\n"
          "              [-wheelhack]\n"
          "              [-navhack]\n"
          "              [-reversewheel]\n"
          "              [-scrolllines lines]\n"
          "              [-mac]\n"
          "              [-trimsel]\n"
          "              [-noblank]\n"
          "              [-lockdelay seconds]\n"
          "              [-debug]\n"
          "              [-accel multiplier]\n"
          "              [-noreconnect]\n"
          "              [-tunnel]\n"
          "              [-via <host>]\n"
          " Known extensions:"
#ifdef HAVE_XINERAMA
          " Xinerama"
#endif
#if HAVE_MIT_SCREENSAVER
          " MIT-Screensaver"
#endif
#ifdef HAVE_XIDLE
          " Xidle"
#endif
#ifdef HAVE_XRANDR
          " randr"
#endif
#ifdef HAVE_XF86DGA
          " XFree86-DGA"
#endif
          "\n"
          ,programName,programName);
  exit(1);
}

void processArgs(int argc, char **argv)
{
  int i;
  Bool argumentSpecified = False;
  
  programName = argv[0];
  
  for (i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
        if (strcmp(argv[i],"-display") == 0) {
          if (++i >= argc) usage();
          displayname = argv[i];
        } else if (strcmp(argv[i],"-east") == 0) {
          edge=EDGE_EAST;
        } else if (strcmp(argv[i],"-west") == 0) {
          edge=EDGE_WEST;
        } else if (strcmp(argv[i],"-north") == 0) {
          edge=EDGE_NORTH;
        } else if (strcmp(argv[i],"-south") == 0) {
          edge=EDGE_SOUTH;
        } else if (strcmp(argv[i],"-mac") == 0) {
          extern int mac_mode;
          mac_mode=1;
        } else if (strcmp(argv[i],"-nowheel") == 0) {
          extern int emulate_wheel;
          emulate_wheel=0;
        } else if (strcmp(argv[i],"-wheelhack") == 0) {
          extern int emulate_wheel;
          emulate_wheel=1;
        } else if (strcmp(argv[i],"-navhack") == 0) {
          extern int emulate_nav;
          emulate_nav=1;
        } else if (strcmp(argv[i],"-reversewheel") == 0) {
          extern int wheel_button_up;
          wheel_button_up=5;
        } else if (strcmp(argv[i],"-resurface") == 0) {
          resurface=True;
        } else if (strcmp(argv[i],"-noreconnect") == 0) {
          reconnect=False;
        } else if (strcmp(argv[i],"-shared") == 0) {
          shareDesktop = True;
        } else if (strcmp(argv[i],"-edgewidth") == 0) {
          extern int edge_width;
          if (++i >= argc) usage();
          edge_width = atoi(argv[i]);
          if(edge_width < 0)
          {
            fprintf(stderr,"x2vnc: -edgewidth cannot be less than 0\n");
            exit(1);
          }
        } else if (strcmp(argv[i],"-restingx") == 0) {
          extern int restingx;
          if (++i >= argc) usage();
          restingx = atoi(argv[i]);
          if(restingx != 0 && restingx != -1)
          {
            fprintf(stderr,"x2vnc: -restingx can only be 0 or -1\n");
            exit(1);
          }
        } else if (strcmp(argv[i],"-restingy") == 0) {
          extern int restingy;
          if (++i >= argc) usage();
          restingy = atoi(argv[i]);
          if(restingy != 0 && restingy != -1)
          {
            fprintf(stderr,"x2vnc: -restingy can only be 0 or -1\n");
            exit(1);
          }
        } else if (strcmp(argv[i],"-desktop") == 0) {
          extern int requested_desktop;
          if (++i >= argc) usage();
          requested_desktop = atoi(argv[i]);
        } else if (strcmp(argv[i],"-timeout") == 0) {
          extern long grab_timeout_delay;
          if (++i >= argc) usage();
          grab_timeout_delay = atoi(argv[i]);
          if(grab_timeout_delay < 0)
          {
            fprintf(stderr,"x2vnc: -timeout cannot be less than 0\n");
            exit(1);
          }
        } else if (strcmp(argv[i],"-lockdelay") == 0) {
          if (++i >= argc) usage();
          no_wakeup_delay = atoi(argv[i]);
        } else if (strcmp(argv[i],"-accel") == 0) {
          if (++i >= argc) usage();
          acceleration = atof(argv[i]);
        } else if (strcmp(argv[i],"-scrolllines") == 0) {
          extern int scroll_lines;
          if (++i >= argc) usage();
          scroll_lines = atoi(argv[i]);
          if(scroll_lines < 1)
          {
            fprintf(stderr,"x2vnc: -scrollines cannot be less than 1\n");
            exit(1);
          }
        } else if (strcmp(argv[i],"-hotkey") == 0) {
          extern void sethotkey(char *);
          if (++i >= argc) usage();
          sethotkey(argv[i]);
        } else if (strcmp(argv[i],"-passwd") == 0) {
          if (++i >= argc) usage();
          passwdFile = argv[i];
        } else if (strcmp(argv[i],"-passwdfile") == 0) {
          if (++i >= argc) usage();
          passwdFile = argv[i];
        } else if (strcmp(argv[i],"-debug") == 0) {
          debug++;
        } else if (strcmp(argv[i],"-trimsel") == 0) {
          trimsel = True;
        } else if (strcmp(argv[i],"-noblank") == 0) {
          noblank = True;
        } else if (strcmp(argv[i],"-tunnel") == 0) {
          useSSHTunnel=True;
        } else if (strcmp(argv[i],"-sshuser") == 0) {
          useSSHTunnel=True;
          if (++i >= argc) usage();
          sshUser = argv[i];
        } else if (strcmp(argv[i],"-sshport") == 0) {
          useSSHTunnel=True;
          if (++i >= argc) usage();
          sshPort = atoi(argv[i]);
          if(sshPort < 1 || sshPort > 65535)
          {
            fprintf(stderr,"x2vnc: -sshport must be between 1 and 65535\n");
            exit(1);
          }
        } else if (strcmp(argv[i],"-via") == 0) {
          if (++i >= argc) usage();
          useSSHGateway = argv[i];
          useSSHTunnel=True;
        } else if (strcmp(argv[i],"-listen") == 0) {
          if (argumentSpecified) usage();
          
          listenSpecified = True;
          if (++i < argc) {
            listenPort = CLIENTPORT+atoi(argv[i]);
            flashPort = FLASHPORT+atoi(argv[i]);
          }
        } else {
          usage();
        }
    
    } else {
      
      if (argumentSpecified || listenSpecified) usage();
      
      argumentSpecified = True;
      
      port = 0;
      if (sscanf(argv[i], "%[^:]:%d", hostname, &port) != 2) {
        strncpy(hostname, argv[i], sizeof(hostname));
        port = 0;
      }
      
      if (port < 0) {
          usage();
      } else if (port < 100) {
          port += SERVERPORT;
      } else if (port > 65535) {
          usage();
      }
    }
  }
  
  if (listenSpecified) {
    if (listenPort == 0) {
      char *display;
      char *colonPos;
      struct utsname hostinfo;
      
      display = XDisplayName(displayname);
      colonPos = strchr(display, ':');
      
      uname(&hostinfo);
      
      if (colonPos && ((colonPos == display) ||
                       (strncmp(hostinfo.nodename, display,
                                strlen(hostinfo.nodename)) == 0))) {
        
        listenPort = CLIENTPORT+atoi(colonPos+1);
        flashPort = FLASHPORT+atoi(colonPos+1);
        
      } else {
        fprintf(stderr,"%s: cannot work out which display number to "
                "listen on.\n", programName);
                fprintf(stderr,
                        "Please specify explicitly with -listen <num>\n");
                exit(1);
            }
        }

    } else if (!argumentSpecified) {

        usage();

    }
}
