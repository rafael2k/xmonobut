/* XMonobut - Modify mouse button behaviour with keyboard.


   Copyright 2002 Matthew Allum <mallum@handhelds.org>

   Thanks to Dave Capella and Patrick Hill for patches.

   Originally loosly based on ideas found in xrmouse

   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.


*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xmd.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>
#include <X11/xpm.h>
#include <X11/Xresource.h>
#include <X11/extensions/shape.h>

#include "tray.h"
#include "xmonobut.xpm"

int get_keycode_from_str(char *keycode_str)
{
   int keycode = 0;
   if(!strcmp(keycode_str,"calendar")) { keycode = 130; }
   else if(!strcmp(keycode_str,"phone")) { keycode = 131; }
   else if(!strcmp(keycode_str,"mail")) { keycode = 132; }
   else if(!strcmp(keycode_str,"start")) { keycode = 133; }
   else if(!strcmp(keycode_str,"record")) { keycode = 128; }
   else { keycode = atoi(keycode_str); }
   if(!keycode) {
      fprintf(stderr,"invalid keycode\n");
      exit(1);
   }
   return keycode;
}

void usage()
{
   printf("usage: xmonobut [options...] \n");
   printf("where options are;\n");
   printf("\t-d <display>\n");
   printf("\t-k <keycode> keycode to grab. Tap once for right mouse button\n");
   printf("\t             , quickly tap twice for middle button ( disabled\n");
   printf("\t             if middle button keycode selected ).\n");
   printf("\t-t <value>   Time in milliseconds to catch double clicks\n");
   printf("\t             Defauts to 200.\n");
   printf("\t-m <keycode> Keycode to grab for middle mouse button.\n");
   printf("\t-n           Do not display dock window.\n" );
   printf("\t-c           Do not change cursor.\n" );
   printf("\t-r           Align window right in dock ( matchbox only )\n");
   exit(0);
}

int main(int argc, char **argv)
{
   Display *dpy;
   char *dpy_name = NULL;
   Window root, dock_win = 0;
   GC gc = 0;
   
   Atom window_type_atom;
   Atom window_type_dock_atom;
   Pixmap icon, mask;
   XpmAttributes xpm_attr;	

   Atom mb_align_atom;
   Atom mb_align_east_atom;
   int align_dock_right = False;
   
   XGCValues gv;
   
   XEvent ev;
   Cursor right_curs, middle_curs, orig_curs;
   Time lasttime = 0;

   unsigned char map[6];

   int i, buts, rst;
   
   /* Defaults */
   int keycode  = 0;
   int keycode2 = 0;
   int use_dock = 1;
   int dbl_click_time = 200;
   int align_right = 0;
   int active_button_num = 1;
   int last_button_num = 1;
   int cursor_disabled = False;
   
   for (i=1; argv[i]; i++) {
      char *arg = argv[i];
      if (*arg=='-') {
	 switch (arg[1]) {
	    case 'd' : /* display */
	       dpy_name = argv[i+1];
	       i++;
	       break;
	    case 'r' :
	       align_right = 1;
	       break;
	    case 'n' :
	       use_dock = 0;
	       break;
	    case 'k' :
	       keycode = get_keycode_from_str(argv[i+1]);
	       i++;
	       break;
	    case 'm' :
	       keycode2 = get_keycode_from_str(argv[i+1]);
	       i++;
	       break;
	    case 't' :
	       dbl_click_time = atoi(argv[i+1]);
	       i++;
	       break;
	    case 'c' :
	       cursor_disabled = True;
	       i++;
	       break;

	    default:
	       usage();
	       exit(0);
	       break;
      }
    }
  }
   
   if ((dpy = XOpenDisplay(dpy_name)) == NULL) {
      fprintf(stderr, "can't open display! check your DISPLAY variable.\n");
      exit(1);
   }

   /* see http://tronche.com/gui/x/xlib/appendix/b/ for more cursors */
   right_curs  = XCreateFontCursor(dpy, XC_mouse);
   middle_curs = XCreateFontCursor(dpy, XC_gumby);    /* :) */
   orig_curs   = XCreateFontCursor(dpy, XC_left_ptr);

   if (use_dock)
   {

      /* -- Dock Window --  */

      window_type_atom = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE" , False);
      window_type_dock_atom = XInternAtom(dpy,
					  "_NET_WM_WINDOW_TYPE_DOCK",False);
      mb_align_atom = XInternAtom(dpy, "_MB_DOCK_ALIGN", False);
      mb_align_east_atom = XInternAtom(dpy, "_MB_DOCK_ALIGN_EAST",False);

      dock_win = XCreateSimpleWindow(dpy,
				     RootWindow(dpy, DefaultScreen(dpy)),
				     0, 0,
				     10, 14,
				     0, BlackPixel(dpy, DefaultScreen(dpy)),
				     WhitePixel(dpy, DefaultScreen(dpy)));

      gv.graphics_exposures = False;
      gv.function           = GXcopy;
      gv.foreground         = BlackPixel(dpy, DefaultScreen(dpy));
      gc = XCreateGC(dpy, dock_win,
		     GCGraphicsExposures|GCFunction|GCForeground, &gv);
      
      xpm_attr.valuemask = 0;
      if (XpmCreatePixmapFromData( dpy, dock_win, xmonobut_xpm,
				   &icon, &mask, &xpm_attr) != XpmSuccess )
      {
	 fprintf(stderr, "failed to get icon image\n");
	 exit(1);
      }
       
      XCopyArea(dpy, icon, dock_win, gc, 0, 0,
		xpm_attr.width, xpm_attr.height, 0, 0);
      
      XShapeCombineMask (dpy, dock_win, ShapeBounding, 0, 0, mask, ShapeSet);

      XStoreName(dpy, dock_win,"Mouse Modifier");      

      if (align_dock_right)
	 XChangeProperty(dpy, dock_win, mb_align_atom, XA_ATOM, 32, 
			 PropModeReplace,
			 (unsigned char *) &mb_align_east_atom, 1);
   
      tray_init_session_info(dpy, dock_win, argv, argc);
      tray_init(dpy, dock_win);
      
      XSelectInput(dpy, dock_win, ExposureMask|ButtonPressMask ); 
   }

   root = RootWindow(dpy, DefaultScreen(dpy));
   
   if (keycode)
   {
      XGrabKey(dpy, keycode, 0, root, True, GrabModeAsync, GrabModeAsync);
      if (keycode2)
	 XGrabKey(dpy, keycode2, 0, root, True, GrabModeAsync, GrabModeAsync);
      XSelectInput(dpy, root, KeyPressMask | KeyReleaseMask );
   }
   
   for (;;) {
      XNextEvent(dpy, &ev);
      switch (ev.type) {
	 case KeyPress:
	    if (ev.xkey.keycode != keycode && !keycode2) break;
	    if ( (ev.xkey.keycode == keycode2)
		 || (!keycode2 && lasttime
		     && (ev.xkey.time-lasttime < dbl_click_time) ))
	       active_button_num = 2;
	    else
	       active_button_num = 3;
	    XAutoRepeatOff(dpy);
	    lasttime = ev.xkey.time;
	    break;
	    
	 case KeyRelease:
	    if ( (ev.xkey.keycode == keycode)
		 || ( keycode2 && ev.xkey.keycode == keycode2) )
	    {
	       active_button_num = 1;
	       XAutoRepeatOn(dpy);
	    }
	    break;

	 case ButtonPress:
	    if (++active_button_num > 3) active_button_num = 1;
	    break;

	 case Expose:
	    if (use_dock)
	    {
	       XCopyArea(dpy, icon, dock_win, gc, 0, 0,
			 xpm_attr.width, xpm_attr.height, 0, 0);
	       switch (active_button_num)
	       {
		  case 1:
		     break;
		  case 2:
		     XFillRectangle(dpy, dock_win, gc, 4, 1, 2, 3) ;
		     break;
		  case 3:
		     XFillRectangle(dpy, dock_win, gc, 6, 1, 2, 3);
		     break;
	       }
	       break;
	    }
      }
      
      if (active_button_num != last_button_num)
      {
	 buts = XGetPointerMapping(dpy, map, 5);
	 switch (active_button_num)
	 {
	    case 1:
	       map[0] = (unsigned char) 1;
	       map[1] = (unsigned char) 2;
	       map[2] = (unsigned char) 3;
	       if (!cursor_disabled)
		  XDefineCursor(dpy, root, orig_curs);
	       if (use_dock)
	       {
		  XCopyArea(dpy, icon, dock_win, gc, 0, 0,
			    xpm_attr.width, xpm_attr.height, 0, 0);
	       }
	       break;
	    case 2:
	       map[0] = (unsigned char) 2;
	       map[1] = (unsigned char) 3;
	       map[2] = (unsigned char) 1;
	       if (!cursor_disabled)
		  XDefineCursor(dpy, root, middle_curs);
	       if (use_dock)
	       {
		  XCopyArea(dpy, icon, dock_win, gc, 0, 0,
			    xpm_attr.width, xpm_attr.height, 0, 0);
		  XFillRectangle(dpy, dock_win, gc, 4, 1, 2, 3) ;
	       }
	       break;
	    case 3:
	       map[0] = (unsigned char) 3;
	       map[1] = (unsigned char) 2;
	       map[2] = (unsigned char) 1;
	       if (!cursor_disabled)
		  XDefineCursor(dpy, root, right_curs);
	       if (use_dock)
	       {
		  XCopyArea(dpy, icon, dock_win, gc, 0, 0,
			    xpm_attr.width, xpm_attr.height, 0, 0);
		  XFillRectangle(dpy, dock_win, gc, 6, 1, 2, 3) ;
	       }
	       break;
	 }
	 while((rst = XSetPointerMapping(dpy, map, buts)) == MappingBusy);
	 last_button_num = active_button_num;
      }
      if (use_dock) tray_handle_event(dpy, dock_win, &ev); 
    }

   if (keycode) XUngrabKey(dpy, keycode, AnyModifier, root);
   if (keycode2) XUngrabKey(dpy, keycode2, AnyModifier, root);
      
   XCloseDisplay(dpy);
}

   
