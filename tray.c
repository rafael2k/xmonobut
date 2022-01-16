#include "tray.h"

static Atom Atoms[10];
static Window Dock;
static Bool   Is_docked;
static Bool   Found_tray;

int
tray_init(Display* dpy, Window win)
{
   Atoms[ATOM_SYSTEM_TRAY]
      = XInternAtom(dpy, "_NET_SYSTEM_TRAY_S0", False);

   Atoms[ATOM_SYSTEM_TRAY_OPCODE]
      =  XInternAtom (dpy, "_NET_SYSTEM_TRAY_OPCODE", False);
   
   Atoms[ATOM_XEMBED_INFO]
      = XInternAtom(dpy, "_XEMBED_INFO", False);

   Atoms[ATOM_XEMBED]
      = XInternAtom(dpy, "_XEMBED", False);

   Atoms[ATOM_MANAGER]
      = XInternAtom(dpy, "MANAGER", False);

   /* !!! This could mess events up !!! */
   XSelectInput(dpy, RootWindow(dpy, DefaultScreen(dpy)), StructureNotifyMask);
		
   Is_docked = False;
		
   /* Find the dock */
   Dock = XGetSelectionOwner(dpy, Atoms[ATOM_SYSTEM_TRAY]);
   if (Dock)
   {
      TRAYDBG("found dock, sending opcode\n");
      tray_unmap_window (dpy, win);
      tray_send_opcode(dpy, Dock, SYSTEM_TRAY_REQUEST_DOCK, win, 0, 0);
      /*
	next request map or have this already set ?
	or wait for XEMBED_EMBEDDED_NOTIFY ? 
      */
      Found_tray = True;
      return 1;
   }
   /* no dock we dont map till dock appears  */

   Found_tray = False;
   return 0;
}

void tray_init_session_info(Display *d, Window win, char **argv, int argc)
{
   XSetCommand(d, win, argv, argc);
}

void
tray_handle_event(Display *dpy, Window win, XEvent *an_event)
{
   /*
     tray should handle -
     ReparentNotify, if parent != Dock then not docked anymore - hide!
     ClientMessage, XEMBED_EMBEDDED_NOTIFY to set truly docked flag
                    MANAGER on root -> selection appeared ?
   */

   //tray_map_window (dpy, win);
   
   switch (an_event->type)
   {
      case ClientMessage:
	 TRAYDBG("message recieved\n");

	 if (an_event->xclient.message_type
	     == Atoms[ATOM_XEMBED])
	 {
	    switch (an_event->xclient.data.l[1])
	    {
	       case XEMBED_EMBEDDED_NOTIFY:
	       case XEMBED_WINDOW_ACTIVATE:
		  tray_map_window (dpy, win);
		  break;
	    }
	 }
	 if (an_event->xclient.message_type
	     == Atoms[ATOM_MANAGER] && !Found_tray)
	 {   /* dock has appeared  */
	    if (an_event->xclient.data.l[1] == Atoms[ATOM_SYSTEM_TRAY])
	    {
	       Dock = XGetSelectionOwner(dpy, Atoms[ATOM_SYSTEM_TRAY]);
	       TRAYDBG("got manager message on root\n");
	       if (Dock)
	       {
		  tray_unmap_window (dpy, win);
		  tray_send_opcode(dpy, Dock, SYSTEM_TRAY_REQUEST_DOCK,
				   win, 0, 0);
		  Found_tray = True;
	       }
	    }
	 }

	 break;
      case ReparentNotify:
	 TRAYDBG("ouch got reparented\n");
	 break;
   }

}

void
tray_set_xembed_info (Display* dpy, Window win, int flags)
{
   CARD32 list[2];
   
   list[0] = MAX_SUPPORTED_XEMBED_VERSION;
   list[1] = flags;
   XChangeProperty (dpy, win, Atoms[ATOM_XEMBED_INFO], XA_CARDINAL, 32,
		    PropModeReplace, (unsigned char *) list, 2);
   
}

void
tray_map_window (Display* dpy, Window win)
{
   tray_set_xembed_info (dpy, win, XEMBED_MAPPED);
}

void
tray_unmap_window (Display* dpy, Window win)
{
   tray_set_xembed_info (dpy, win, 0);
}

void
tray_send_opcode(
		      Display* dpy,
		      Window w,    
		      long message,
		      long data1,  
		      long data2,  
		      long data3   
		      ){
   XEvent ev;
   
   memset(&ev, 0, sizeof(ev));
   ev.xclient.type = ClientMessage;
   ev.xclient.window = w;
   ev.xclient.message_type = Atoms[ATOM_SYSTEM_TRAY_OPCODE];
   ev.xclient.format = 32;
   ev.xclient.data.l[0] = CurrentTime;
   ev.xclient.data.l[1] = message;
   ev.xclient.data.l[2] = data1;
   ev.xclient.data.l[3] = data2;
   ev.xclient.data.l[4] = data3;
   
   //trap_errors();
   XSendEvent(dpy, w, False, NoEventMask, &ev);
   XSync(dpy, False);
   //if (untrap_errors()) {
      /* Handle failure */
   //}
}
