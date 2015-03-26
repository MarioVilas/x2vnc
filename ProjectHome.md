# Who wrote it #

The x2vnc original code was written by **Fredrik Hubinette** and can be found here:

> http://fredrik.hubbe.net/x2vnc.html

This is an **unofficial** branch with some simple improvements.

# What it does #

This program will let you use two screens on two different computers as if they were connected to the same computer. Even if one of the computers runs Windows 95/98/NT and the other one runs X-windows. If they are both running Windows, you probably want to use Win2VNC instead.

# How it works #

The program will open a small (one pixel wide) window on the edge of your screen. Moving the pointer into this window will trigger the program to take over your mouse and send mouse movements and keystrokes though the RFB protocol to a VNC server running on another machine. When the pointer is moved back towards the opposite edge on the other screen, the mouse is then released again. The operation itself is almost identical to x2x, but most of the code was actually borrowed from the program vncviewer.

As the name x2vnc implies, x2vnc can only send events from an X-windows based display to any VNC server. VNC servers can run on Microsoft Windows 95/98/NT/2000/XP. x2vnc will not run without X-windows. Please note that the normal VNC server for X windows does not control the mouse on the screen itself, but creates a virtual server in memory instead. If you wish to control an X11 display with x2vnc, you need to use x11vnc, but it's probably easier to just use x2x instead.