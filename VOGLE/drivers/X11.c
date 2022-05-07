/*
 * VOGL/VOGLE driver for X11.
 * 
 * Define VOGLE if this driver is really for the VOGLE Libarary.
 *
 */
#define VOGLE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#ifdef VOGLE

#include "vogle.h"
#include "voglex.h"

static	char	*me = "vogle";
#define LARGEFONT       "-adobe-courier-medium-r-normal--24-240-75-75-m-150-iso8859-1"
#define SMALLFONT       "-adobe-courier-medium-r-normal--10-100-75-75-m-60-iso8859-1"

#else

#include "vogl.h"
static	char	*me = "vogl";
#define LARGEFONT	"9x15bold"
#define SMALLFONT	"6x13bold"

#endif

static	char	*xdisp = (char *)NULL;

#define MIN(x,y)	((x) < (y) ? (x) : (y))
#define MAX(x,y)	((x) > (y) ? (x) : (y))
#define	CMAPSIZE	256
#define EVMASK	StructureNotifyMask|ExposureMask|KeyPressMask|ButtonPressMask|ButtonReleaseMask|KeyReleaseMask|PointerMotionMask
#define	MAXPNTS		256

/*
 * This defines an instance of an X11 device.
 */
typedef struct {
	Window		winder;		/* .. local id, must be first */
	int		id;		/* Vogle's window id */
	Display		*display;
	int		screen;
	unsigned long	carray[CMAPSIZE];
	Colormap	colormap;
	Drawable	theDrawable;
	GC		theGC;
	XGCValues	theGCvalues;
	Pixmap		bbuf;		/* Back buffer pixmap */
	XFontStruct	*font_id;
	unsigned long	colour;
	unsigned int	h, w;
	char		*smallf, *largef;
	char		*title;
	char		back_used;
	char		privatecmap;
	char		depth;
	char		initialized;
} X11_rec;

static X11_rec		*x11;
static char		errbuf[BUFSIZ];
static XEvent		event;

/*
 * Functions
 */
static int X11_draw(int x, int y);
static int X11_pnt(int x, int y);
static int X11_getkey(void);
static int X11_checkkey(void);
static int X11_locator(int *wx, int *wy);
static int X11_clear(void);
static int X11_color(int ind);
static int X11_mapcolor(int i, int r, int g, int b);
static int X11_font(const char *fontfile);
static int X11_char(char c);
static int X11_string(const char *s);
static int X11_fill(int n, int x[], int y[]);
static int X11_backbuf(void *p, int old, int bw, int bh);
static int X11_swapbuf(void);
static int X11_frontbuf(void);
static void X11_sync(void);
static int X11_setlw(int w);
static int X11_setls(int lss);
static void * X11_winopen(const char *dev, const char *title, int id);
static int X11_winset(void *devwin);
static int X11_winclose(void *devwin);
static int X11_winraise(void *devwin);
static int X11_windel(void *devwin);
static unsigned long X11_getevent(Vevent *vev, int noblock);
void _X11_devcpy(char *dev);

/*
 * Tell vogle to use an existing X11 window. To be called once for
 * each toolkit window that vogle is to use.
 *
 * Returns an integer for referencing the window from within vogle
 */
int
vo_xt_window(Display *dis, Window win, int xw, int xh)
{
   int	ind, i;

   x11 = (X11_rec *)vallocate(sizeof(X11_rec));
   if (x11 == NULL)
   {
      return(0);
   }
   
   memset(x11, 0, sizeof(X11_rec));

   x11->id		= 10000;
   x11->display		= dis;
   x11->winder		= win;
   x11->screen		= DefaultScreen(x11->display);
   x11->colormap	= DefaultColormap(x11->display, x11->screen);
   x11->depth		= vdevice.dev.depth = DefaultDepth(x11->display, x11->screen);
   x11->theDrawable	= x11->winder;

   x11->w 		= xw;
   x11->h 		= xh;

   _X11_devcpy("X11");
   ind 				= _vgetfreewindowslot();
   vdevice.wins[ind].id 	= ind;
   vdevice.wins[ind].devwin 	= x11;
   vdevice.wins[ind].dev 	= vdevice.dev;
   vdevice.wins[ind].localid 	= win;
   vdevice.wins[ind].havebackb 	= 0;
   vdevice.curwin 		= ind;

   /*
    * Set our standard colors...
    */
   if (vdevice.dev.depth == 1)
   {
      /*
       * Black and white - anything that's not black is white.
       */
      x11->carray[0] = BlackPixel(x11->display, x11->screen);
      for (i = 1; i < CMAPSIZE; i++)
      {
         x11->carray[i] = WhitePixel(x11->display, x11->screen);
      }
   }
   else
   {
      /*
       * Color, try to get our colors close to what's in the
       * default colormap.
       */
      X11_mapcolor(0, 0, 0, 0);
      X11_mapcolor(1, 255, 0, 0);
      X11_mapcolor(2, 0, 255, 0);
      X11_mapcolor(3, 255, 255, 0);
      X11_mapcolor(4, 0, 0, 255);
      X11_mapcolor(5, 255, 0, 255);
      X11_mapcolor(6, 0, 255, 255);
      X11_mapcolor(7, 255, 255, 255);
   }

   if ((x11->smallf = XGetDefault(x11->display, me, "smallfont")) == (char *)NULL)
   {
      x11->smallf = SMALLFONT;
   }
   
   if ((x11->largef = XGetDefault(x11->display, me, "largefont")) == (char *)NULL)
   {
      x11->largef = LARGEFONT;
   }
   
   /*
    * Create Graphics Context and Drawable
    */
   x11->theGC = XDefaultGC(x11->display, x11->screen);
   x11->theGCvalues.graphics_exposures = False;
   x11->theGCvalues.cap_style = CapButt;
   XChangeGC(x11->display, x11->theGC, GCGraphicsExposures|GCCapStyle, &x11->theGCvalues);
   X11_color(0);

   vdevice.dev.sizeX  = vdevice.dev.sizeY = MIN(xh, xw);
   vdevice.dev.sizeSx = xw;
   vdevice.dev.sizeSy = xh;
   vdevice.wins[ind].dev.sizeSx = xw;
   vdevice.wins[ind].dev.sizeSy = xh;
   vdevice.wins[ind].dev.sizeX  = vdevice.wins[ind].dev.sizeY = MIN(xh, xw);

   /* XAV */
   prefsize(xw, xh);
   /* XAV */
   
   if (!vdevice.initialised)
   {
      _vdovogleinit();
   }
   
   return(ind);
}

/*
 *	vo_xt_win_size
 *
 * If the X toolkit has changed the window size, then
 * you might wish to call this routine to tell vogl/vogle about it.
 */
int
vo_xt_win_size(int w, int xw, int xh)
{
   X11_rec	*x11win;
   char	backb;

   if (w < 0 || w >= vdevice.nwin)
      return(-1);

   if (vdevice.wins[w].id == -1)
      return(-1);

   x11win = (X11_rec *)vdevice.wins[w].devwin;

   backb = (x11win->theDrawable == x11win->bbuf);

   if (x11win->back_used)
   {

      X11_backbuf(x11win, 1, xw, xh);

      if (backb)
      {
         x11win->theDrawable = (Drawable)x11win->bbuf;
      }
   }

   x11win->w = xw;
   x11win->h = xh;
   vdevice.wins[w].dev.sizeSx = xw;
   vdevice.wins[w].dev.sizeSy = xh;
   vdevice.wins[w].dev.sizeX  = vdevice.wins[w].dev.sizeY = MIN(xh, xw);
   if (w == vdevice.curwin)
   {
      vdevice.dev.sizeX = vdevice.dev.sizeY = MIN(xh, xw);
      vdevice.dev.sizeSx = xw;
      vdevice.dev.sizeSy = xh;
   }

   /* XAV */
   prefsize(xw, xh);
   /* XAV */
   
   return(1);
}

/*
 * return the X display in use.
 */
Display *
vo_xt_get_display(int w)
{
   X11_rec	*x11win;

   if (w < 0 || w >= vdevice.nwin)
      return(NULL);

   if (vdevice.wins[w].id == -1)
      return(NULL);

   x11win = (X11_rec *)vdevice.wins[w].devwin;

   return(x11win->display);
}

/*
 * return the X Window in use.
 */
Window
vo_xt_get_window(int w)
{
   X11_rec	*x11win;

   if (w < 0 || w >= vdevice.nwin)
      return(0);

   if (vdevice.wins[w].id == -1)
      return(0);

   x11win = (X11_rec *)vdevice.wins[w].devwin;

   return(x11win->winder);
}

/*
 * return the Windows back Pixmap (if there is one).
 */
Pixmap
vo_xt_get_backbuf(int w)
{
   X11_rec	*x11win;

   if (w < 0 || w >= vdevice.nwin)
      return(0);

   if (vdevice.wins[w].id == -1)
      return(0);

   x11win = (X11_rec *)vdevice.wins[w].devwin;

   return(x11win->bbuf);
}

/*
 * return the Graphics Context in use.
 */
GC
vo_xt_get_GC(int w)
{
   X11_rec	*x11win;

   if (w < 0 || w >= vdevice.nwin)
      return(0);

   if (vdevice.wins[w].id == -1)
      return(0);

   x11win = (X11_rec *)vdevice.wins[w].devwin;

   return(x11win->theGC);
}

/*
 * Set the Graphics Context to use.
 */
void
vo_xt_set_GC(int w, GC gc)
{
   X11_rec	*x11win;

   if (w < 0 || w >= vdevice.nwin)
      return;

   if (vdevice.wins[w].id == -1)
      return;

   x11win = (X11_rec *)vdevice.wins[w].devwin;

   x11win->theGC = gc;
}


/*
 * X11_winopen
 *
 *	open and/or initialises X11 display.
 */
static void *
X11_winopen(const char *devname, const char *title, int id)
{
   int		i;
   int		x, y, prefx, prefy, prefxs, prefys;
   unsigned int	w, h;
   unsigned int	bw, depth, mask;
   Window		rootw, childw;
   char		*av[2], *geom;
   static Display	*display = NULL;

   XSetWindowAttributes    theWindowAttributes;
   XWindowAttributes	retWindowAttributes;
   XSizeHints              theSizeHints;
   unsigned long           theWindowMask;
   XWMHints                theWMHints;


   x11 = (X11_rec *)vallocate(sizeof(X11_rec));
   if (x11 == NULL)
      return(NULL);

   memset(x11, 0, sizeof(X11_rec));


   av[0] = me;
   av[1] = (char *)NULL;

   if (!display)
   {
      if ((display = XOpenDisplay(xdisp)) == (Display *)NULL)
      {
         sprintf(errbuf, "X11_init: can't connect to X server (%s)\n", xdisp ? xdisp : "(from env)");
         if (verror(VERR_DRIVER, errbuf) == -1)
         {
            return(NULL);
         }
      }
   }
   x11->display = display;

   x11->screen = DefaultScreen(x11->display);
   x11->winder = RootWindow(x11->display, x11->screen);
   x11->id = id;
   if (getenv("USEOWNCMAP"))
   {
      x11->colormap = XCreateColormap(x11->display, x11->winder, DefaultVisual(x11->display, x11->screen), AllocAll);
      x11->privatecmap = 1;
   }
   else
   {
      x11->colormap = DefaultColormap(x11->display, x11->screen);
   }

   x11->depth = vdevice.dev.depth = DefaultDepth(x11->display, x11->screen);

   /*
    * Set our standard colors...
    */
   if (vdevice.dev.depth == 1)
   {
      /*
       * Black and white - anything that's not black is white.
       */
      x11->carray[0] = BlackPixel(x11->display, x11->screen);
      for (i = 1; i < CMAPSIZE; i++)
      {
         x11->carray[i] = WhitePixel(x11->display, x11->screen);
      }
   }
   else
   {
      /*
       * Color, try to get our colors close to what's in the
       * default colormap.
       */
      X11_mapcolor(0, 0, 0, 0);
      X11_mapcolor(1, 255, 0, 0);
      X11_mapcolor(2, 0, 255, 0);
      X11_mapcolor(3, 255, 255, 0);
      X11_mapcolor(4, 0, 0, 255);
      X11_mapcolor(5, 255, 0, 255);
      X11_mapcolor(6, 0, 255, 255);
      X11_mapcolor(7, 255, 255, 255);
   }

   getprefposandsize(&prefx, &prefy, &prefxs, &prefys);

   /*
    * NEED TO USE XGRABPOINTER here???
    */
   XQueryPointer(x11->display, x11->winder, &rootw, &childw, &x, &y, &x, &y, &mask);

   if (childw == None)
   {
      childw = rootw;
   }
   
   XGetGeometry(x11->display, childw, &rootw, &x, &y, &w, &h, &bw, &depth);


   theWindowAttributes.backing_store = WhenMapped;
   theWindowAttributes.save_under    = True;
   theWindowAttributes.border_pixel  = x11->carray[1];


   /*
    * See if there is something in the .Xdefaults file regarding
    * VOGL/VOGLE.
    */

   if ((x11->smallf = XGetDefault(x11->display, me, "smallfont")) == (char *)NULL)
      x11->smallf = SMALLFONT;

   if ((x11->largef = XGetDefault(x11->display, me, "largefont")) == (char *)NULL)
      x11->largef = LARGEFONT;


   geom = XGetDefault(display, me, "Geometry");

   theSizeHints.flags = 0;
   w = h = 600;
   if (geom != (char *)NULL)
   {
      theSizeHints.flags = 0;
      mask = XParseGeometry(geom, &x, &y, &w, &h);

      if (mask & XValue)
         theSizeHints.flags |= USPosition;

      if (mask & YValue)
         theSizeHints.flags |= USPosition;

      if (mask & WidthValue)
         theSizeHints.flags |= USSize;

      if (mask & HeightValue)
         theSizeHints.flags |= USSize;

      if (mask & XNegative)
         x = DisplayWidth(x11->display, x11->screen) - 2*bw - w + x;

      if (mask & YNegative)
         y = DisplayHeight(x11->display, x11->screen) - 2*bw - h + y;

   }
   
   if (prefx > -1)
   {
      x = prefx;
      y = prefy;
      theSizeHints.flags |= USPosition;
   }

   if (prefxs > -1)
   {
      w = prefxs;
      h = prefys;
      theSizeHints.flags |= USSize;
   }

   if (bw == 0)
   {
      bw = 4;
   }
   
   x -= bw;
   y -= bw;

   if (x <= 0)
      x = 0;

   if (y <= 0)
      y = 0;

   w -= 4 * bw;
   h -= 4 * bw;

   theWindowMask = CWBorderPixel|CWBackingStore;

   x11->winder = XCreateWindow(x11->display,
                               x11->winder,
                               x, y,
                               w, h,
                               bw,
                               (int)vdevice.dev.depth,
                               InputOutput,
                               CopyFromParent,
                               theWindowMask,
                               &theWindowAttributes
                               );

   theSizeHints.x = x;
   theSizeHints.y = y;
   theSizeHints.width = w;
   theSizeHints.height = h;

   i = strlen(title);
   
   if (i == 0)
   {
      x11->title = (char *)vallocate(6);
      strcpy(x11->title, "Vogle");
   }
   else
   {
      x11->title = (char *)vallocate(i + 1);
      strcpy(x11->title, title);
   }

   XSetStandardProperties(x11->display,
                          x11->winder,
                          x11->title,
                          x11->title,
                          None,
                          av,
                          1,
                          &theSizeHints
                          );

   theWMHints.initial_state = NormalState;
   theWMHints.input         = True;
   theWMHints.flags         = StateHint | InputHint;
   XSetWMHints(x11->display, x11->winder, &theWMHints);

   XSelectInput(x11->display, x11->winder, EVMASK);

   x11->theDrawable = (Drawable)x11->winder;

   /*
    * Create Graphics Context and Drawable
    */
   x11->theGC                          = XDefaultGC(x11->display, x11->screen);
   x11->theGCvalues.graphics_exposures = False;
   x11->theGCvalues.cap_style          = CapButt;
   XChangeGC(x11->display, x11->theGC, GCGraphicsExposures|GCCapStyle, &x11->theGCvalues);
   x11->theDrawable                    = (Drawable)x11->winder;
   X11_color(0);

   XMapRaised(x11->display, x11->winder);
   XFlush(x11->display);

   /*
    * Wait for Exposure event.
    */
   do
   {
      XNextEvent(x11->display, &event);
   } while (event.type != Expose);

   /*
    *  Let VOGL/VOGLE know about the window size.
    *  (We may have been resized..... )
    */
   if (XGetWindowAttributes(x11->display, x11->winder, &retWindowAttributes))
   {
      x = retWindowAttributes.x;
      y = retWindowAttributes.y;
      w = retWindowAttributes.width;
      h = retWindowAttributes.height;
   }

   XTranslateCoordinates(x11->display,
                         x11->winder, retWindowAttributes.root,
                         0, 0,
                         &x, &y,
                         &rootw
                         );
   
   vdevice.dev.sizeX = vdevice.dev.sizeY = MIN(h, w);
   
   x11->w = vdevice.dev.sizeSx = w;
   x11->h = vdevice.dev.sizeSy = h;

   x11->initialized = 1;

   return((void *)x11);
}

static int
X11_winset(void *p)
{
   X11_rec	*x11win = (X11_rec *)p;

   if (!x11win)
      return(-1);

   x11 = x11win;
   return(x11->id);
}

static int
X11_winclose(void *p)
{
   X11_rec	*x11win = (X11_rec *)p;
   /* Make it an icon */
   XIconifyWindow(
                  x11win->display,
                  x11win->winder,
                  x11win->screen
                  );
   return(1);
}

static int
X11_winraise(void *p)
{
   X11_rec	*x11win = (X11_rec *)p;

   /* Un-make it an Icon */

   /*
   * From the Xlib manual...
    * "To change from iconic state to normal state, the client needs
    *  only to map the window--it need not reset the property"
    */
   XMapWindow(x11win->display, x11win->winder);

   return(1);
}

/*
 * X11_windel
 *
 *	cleans up before returning the window to normal.
 */
static int
X11_windel(void *p)
{
   X11_rec	*x11win = (X11_rec *)p;

   if (x11win->back_used && x11win->bbuf)
   {
      XFreePixmap(x11win->display, x11win->bbuf);
   }
   
   if (x11win->font_id != (XFontStruct *)NULL)
   {
      XFreeFont(x11win->display, x11win->font_id);
   }
   
   x11win->font_id = (XFontStruct *)NULL;

   /* Free the GC */


   XDestroyWindow(x11win->display, x11win->winder);

   XSync(x11win->display, 0);

   /*XCloseDisplay(x11win->display);*/

   if (x11win->title)
   {
      free(x11win->title);
   }
   
   /* if (x11 != x11win)	\* Problem: Don't free the one we are using yet */
   free(x11win);

   return(1);
}

/*
 * X11_draw
 *
 *	draws a line from the current graphics position to (x, y).
 *
 * Note: (0, 0) is defined as the top left of the window in X (easy
                                                               * to forget).
 */
static int
X11_draw(int x, int y)
{
   XDrawLine(x11->display,
             x11->theDrawable,
             x11->theGC,
             vdevice.cpVx, vdevice.dev.sizeSy - vdevice.cpVy,
             x, vdevice.dev.sizeSy - y
             );

   if (vdevice.sync)
      XSync(x11->display, 0);

   return(1);
}

static int
X11_pnt(int x, int y)
{
   XDrawPoint(x11->display,
              x11->theDrawable,
              x11->theGC,
              x, vdevice.dev.sizeSy - y
              );

   if (vdevice.sync)
      XSync(x11->display, 0);

   return(1);
}

/*
 * X11_getkey
 *
 *	grab a character from the keyboard - blocks until one is there.
 */
static int
X11_getkey(void)
{
   char	c;

   do
   {
      XNextEvent(x11->display, &event);
      if (event.type == KeyPress)
      {
         if (XLookupString((XKeyEvent *)&event, &c, 1, NULL, NULL) > 0)
         {
            return((int)c);
         }
         else
         {
            return(0);
         }
      }
      
   } while (event.type != KeyPress);

   return(0);
}

/*
 * X11_checkkey
 *
 *	Check if there has been a keyboard key pressed.
 *	and return it if there is.
 */
static int
X11_checkkey(void)
{
   char	c;

   if (!XCheckWindowEvent(x11->display, x11->winder, Expose|KeyPressMask, &event))
   {
      return(0);
   }
   
   if (event.type == KeyPress)
   {
      if (XLookupString((XKeyEvent *)&event, &c, 1, NULL, NULL) > 0)
      {
         return((int)c);
      }
   }
   return(0);
}


static unsigned long
X11_getevent(Vevent *vev, int block)
{
   XButtonEvent	   *be;
   XConfigureEvent *ce;
   XMotionEvent	   *me;
   XEvent	   nextevent;
   char		   c;

   vev->type = 0;
   vev->data = 0;

   if (block)
   {
      XNextEvent(x11->display, &event);
   }
   else
   {
      if (!XCheckMaskEvent(x11->display, EVMASK, &event))
      {
         return(0L);
      }
   }

   be = (XButtonEvent *)&event;
   ce = (XConfigureEvent *)&event;
   me = (XMotionEvent *)&event;

   switch(event.type)
   {
      case KeyPress:
         vev->type = VKEYPRESS;
         if (XLookupString((XKeyEvent *)&event, &c, 1, NULL, NULL) > 0)
         {
            vev->data = c;
         }
         break;

      case KeyRelease:
         vev->type = VKEYRELEASE;
         if (XLookupString((XKeyEvent *)&event, &c, 1, NULL, NULL) > 0)
         {
            vev->data = c;
         }
         break;

      case ButtonPress:
         vev->type = VBUTTONPRESS;
         vev->data = be->button;
         vev->x    = be->x;
         vev->y    = be->y;
         break;

      case ButtonRelease:
         vev->type = VBUTTONRELEASE;
         vev->data = be->button;
         vev->x    = be->x;
         vev->y    = be->y;
      break;

      case MotionNotify:
         vev->type = VMOTION;
         vev->data = me->state;	/* What buttons are active */
         vev->x    = me->x;
         vev->y    = me->y;
         break;
         
      case Expose:
         /* Should compress these a bit... will this work?
         * .. we don't want to return a dodgey event that
         * vogle  doesn't understand.
         */
         while (XCheckTypedEvent(x11->display, Expose, &nextevent))
         {
            event = nextevent;
#ifdef DEBUG
            fprintf(stderr, "Eat an Expose event\n");
#endif
         }

         if (event.xexpose.count == 0)
         {
            vev->type = VREDRAW;
            vev->data = 0;
         }
         break;
         
      case ConfigureNotify:
         /* Should compress these a bit... */
         /* Should check if it's really a resize and not just
         * a move
         */
         vev->type = VRESIZE;
         vev->data = 0;
         vev->x    = ce->width;
         vev->y    = ce->height;
         break;
         
      default:
         vev->type = 0;
         vev->data = 0;
   }

   return((unsigned long)be->window);
}

/*
 * X11_locator
 *
 *	return the window location of the cursor, plus which mouse button,
 * if any, is been pressed.
 */
static int
X11_locator(int *wx, int *wy)
{
   Window		rootw, childw;
   int		x, y;
   unsigned int	mask;

   XQueryPointer(x11->display, x11->winder, &rootw, &childw, &x, &y, wx, wy, &mask);

   *wy = (int)vdevice.dev.sizeSy - *wy;

   return(mask >> 8);
}

#ifdef VOGLE
/*
 * X11_clear
 *
 * Clear the screen (or current buffer )to current colour
 */
static int
X11_clear(void)
{
   XSetBackground(x11->display, x11->theGC, x11->colour);
   XFillRectangle(x11->display,
                  x11->theDrawable,
                  x11->theGC,
                  0,
                  0,
                  vdevice.dev.sizeSx, vdevice.dev.sizeSy
                  );

   if (vdevice.sync)
      XFlush(x11->display);

   return(1);
}

#else

/*
 * X11_clear
 *
 * Clear the screen (or current buffer )to current colour
 */
static int
X11_clear(void)
{
   unsigned int	w = vdevice.maxVx - vdevice.minVx;
   unsigned int	h = vdevice.maxVy - vdevice.minVy;

   XSetBackground(x11->display, x11->theGC, x11->colour);

   XFillRectangle(x11->display,
                  x11->theDrawable,
                  x11->theGC,
                  vdevice.minVx,
                  vdevice.dev.sizeSy - vdevice.maxVy,
                  w,
                  h
                  );

   if (vdevice.sync)
      XFlush(x11->display);

   return(1);
}
#endif

/*
 * X11_color
 *
 *	set the current drawing color index.
 */
static int
X11_color(int ind)
{
   x11->colour = x11->carray[ind];
   XSetForeground(x11->display, x11->theGC, x11->colour);

   return(1);
}

int
RGBcolor(short r, short g, short b)
{
   x11->colour = (r << 16) + (g << 8) + b;
   XSetForeground(x11->display, x11->theGC, x11->colour);

   return(1);
}

/*
 * X11_mapcolor
 *
 *	change index i in the color map to the appropriate r, g, b, value.
 */
static int
X11_mapcolor(int i, int r, int g, int b)
{
   int	stat;
   XColor	tmp;

   if (i >= CMAPSIZE)
   {
      sprintf(errbuf, "X11_mapcolor: index too large [%d]", i);
      return(verror(VERR_DRIVER, errbuf));
   }


   /*
    * For Black and White.
    * If the index is 0 and r,g,b != 0 then we are remapping black.
    * If the index != 0 and r,g,b == 0 then we make it black.
    */
   if (vdevice.dev.depth == 1)
   {
      if (i == 0 && (r != 0 || g != 0 || b != 0))
      {
         x11->carray[i] = WhitePixel(x11->display, x11->screen);
      }
      else
      if (i != 0 && r == 0 && g == 0 && b == 0)
      {
         x11->carray[i] = BlackPixel(x11->display, x11->screen);
      }
   }
   else
   {
      tmp.red = (unsigned short)(r / 255.0 * 65535);
      tmp.green = (unsigned short)(g / 255.0 * 65535);
      tmp.blue = (unsigned short)(b / 255.0 * 65535);
      tmp.flags = 0;
      tmp.pixel = (unsigned long)i;

      if (x11->privatecmap)
      {
         XStoreColor(x11->display, x11->colormap, &tmp);
      }
      else
      if ((stat = XAllocColor(x11->display, x11->colormap, &tmp)) == 0)
      {
         sprintf(errbuf, "XAllocColor failed, try setenv USEOWNCMAP 1'\n");
         return(verror(VERR_DRIVER, errbuf));
      }
      x11->carray[i] = tmp.pixel;
   }

   XFlush(x11->display);
   return(1);
}

/*
 * X11_font
 *
 *   Set up a hardware font. Return 1 on success 0 otherwise.
 *
 */
static int
X11_font(const char *fontfile)
{
   XGCValues	xgcvals;
   char	*name = (char *)fontfile;

   if (x11->font_id != (XFontStruct *)NULL)
   {
      XFreeFont(x11->display, x11->font_id);
   }
   
   if (strcmp(fontfile, "small") == 0)
   {
      if ((x11->font_id = XLoadQueryFont(x11->display, x11->smallf)) == (XFontStruct *)NULL)
      {
         sprintf(errbuf, "X11_font: couldn't open small font '%s'", x11->smallf);
         return(verror(VERR_DRIVER, errbuf));
      }
      else
      {
         name = x11->smallf;
      }
   }
   else
   if (strcmp(fontfile, "large") == 0)
   {
      if ((x11->font_id = XLoadQueryFont(x11->display, x11->largef)) == (XFontStruct *)NULL)
      {
         sprintf(errbuf, "X11_font: couldn't open large font '%s'", x11->smallf);
         return(verror(VERR_DRIVER, errbuf));
      }
      name = x11->largef;
   }
   else
   {
      if ((x11->font_id = XLoadQueryFont(x11->display, fontfile)) == (XFontStruct *)NULL)
      {
         sprintf(errbuf, "X11_font: couldn't open fontfile '%s'\n", fontfile);
         return(verror(VERR_DRIVER, errbuf));
      }
   }

   vdevice.hheight = x11->font_id->max_bounds.ascent + x11->font_id->max_bounds.descent;
   vdevice.hwidth = x11->font_id->max_bounds.width;

   xgcvals.font = XLoadFont(x11->display, name);
   XChangeGC(x11->display, x11->theGC, GCFont, &xgcvals);

   return(1);
}

/*
* X11_char
 *
 *	 outputs one char - is more complicated for other devices
 */
static int
X11_char(char c)
{
   char	s[2];

   s[0] = c;
   s[1] = '\0';

   XDrawString(x11->display, x11->theDrawable, x11->theGC, vdevice.cpVx, (int)(vdevice.dev.sizeSy - vdevice.cpVy), s, 1);

   if (vdevice.sync)
      XFlush(x11->display);

   return(1);
}

/*
 * X11_string
 *
 *	Display a string at the current drawing position.
 */
static int
X11_string(const char *s)
{
   XDrawString(x11->display, x11->theDrawable, x11->theGC, vdevice.cpVx, (int)(vdevice.dev.sizeSy - vdevice.cpVy), s, strlen(s));

   if (vdevice.sync)
      XFlush(x11->display);

   return(1);
}

/*
 * X11_fill
 *
 *	fill a polygon
 */
static int
X11_fill(int n, int x[], int y[])
{
   XPoint	plist[MAXPNTS];
   int	i;

   if (n > MAXPNTS)
   {
      return(-1);
   }

   for (i = 0; i < n; i++)
   {
      plist[i].x = x[i];
      plist[i].y = vdevice.dev.sizeSy - y[i];
   }

   XFillPolygon(x11->display, x11->theDrawable, x11->theGC, plist, n, Nonconvex, CoordModeOrigin);

   vdevice.cpVx = x[n-1];
   vdevice.cpVy = y[n-1];

   if (vdevice.sync)
      XFlush(x11->display);

   return(1);
}

/*
 * X11_backbuf
 *
 *	Set up double buffering by allocating the back buffer and
 *	setting drawing into it.
 */
static int
X11_backbuf(void *p, int old, int bw, int bh)
{
   X11_rec	*x11win = (X11_rec *)p;
   int	redo = 0;

   if (old && (bw > x11win->w || bh > x11win->h))
   {
      if (x11win->back_used && x11win->bbuf)
      {
         XFreePixmap(x11win->display, x11win->bbuf);
      }
      
      x11win->w = bw;
      x11win->h = bh;
      redo = 1;
   }
   else if (!old)
   {
      bw = x11win->w;
      bh = x11win->h;
      redo = 1;
   }

   if (redo)
   {
      x11win->bbuf = XCreatePixmap(x11win->display,
                                   (Drawable)x11win->winder,
                                   (unsigned int)bw,
                                   (unsigned int)bh,
                                   (unsigned int)x11win->depth
                                   );
   }
   
   x11win->theDrawable = (Drawable)x11win->bbuf;

   x11win->back_used = 1;

   return(1);
}

/*
 * X11_swapbuf
 *
 *	Swap the back and from buffers. (Really, just copy the
                                    *	back buffer to the screen).
 */
static int
X11_swapbuf(void)
{
   XCopyArea(x11->display,
             x11->theDrawable,
             x11->winder,
             x11->theGC,
             0, 0,
             (unsigned int)vdevice.dev.sizeSx,
             (unsigned int)vdevice.dev.sizeSy,
             0, 0
             );

   /*XFlush(x11->display);*/
   XSync(x11->display, 0); /* added by xm - YES , IT'S VERY NECCESSARY FOR TRANSIENT DRAWINS ! */

   return(1);
}

/*
 * X11_frontbuf
 *
 *	Make sure we draw to the screen.
 */
static int
X11_frontbuf(void)
{
   x11->theDrawable = (Drawable)x11->winder;

   return(1);
}

/*
 * Syncronise the display with what we think has been sent to it...
 */
static void
X11_sync(void)
{
   XSync(x11->display, 0);
}

#undef VORTDUMP
#ifdef VORTDUMP
/*
 * HACK
 * Dump the contents of the current buffer to a VORT file....
 * ONLY WORKS WITH 8Bit Drawables!
 */
#include "vort.h"

void
X11_dump_pixmap(const char *filename, int dx, int dy, int dw, int dh)
{
   XImage	*ximage;
   image	*im;
   unsigned char	*line, *rm, *gm, *bm;
   XColor	*cols;
   int	i;

   if (dw > vdevice.dev.sizeSx || dw < 0)
      dw = vdevice.dev.sizeSx;
   if (dh > vdevice.dev.sizeSy || dh < 0)
      dh = vdevice.dev.sizeSy;

   if (dx > vdevice.dev.sizeSx || dx < 0)
      dx = 0;
   if (dy > vdevice.dev.sizeSy || dy < 0)
      dy = 0;

   ximage = XGetImage(x11->display,
                      x11->theDrawable,
                      dx, dy,
                      (unsigned int)dw,
                      (unsigned int)dh,
                      AllPlanes,
                      ZPixmap
                      );

   if (!ximage)
   {
      fprintf(stderr, "X11_dump_pixmap: can't do XGetImage\n");
      exit(1);
   }

   if ((im = openimage(filename, "w")) == (image *)NULL)
   {
      fprintf(stderr, "X11_dump_pixmap: can't open %s\n", filename);
      exit(1);
   }

   if (!(rm = (unsigned char *)malloc(256)))
   {
      fprintf(stderr, "X11_dump_pixmap: can't alloc rm\n");
      exit(1);
   }
   if (!(gm = (unsigned char *)malloc(256)))
   {
      fprintf(stderr, "X11_dump_pixmap: can't alloc gm\n");
      exit(1);
   }
   if (!(bm = (unsigned char *)malloc(256)))
   {
      fprintf(stderr, "X11_dump_pixmap: can't alloc bm\n");
      exit(1);
   }
   if (!(cols = (XColor *)malloc(256 * sizeof(XColor))))
   {
      fprintf(stderr, "X11_dump_pixmap: can't alloc cols\n");
      exit(1);
   }

   /*
    * Get our colormap...
    */
   for (i = 0; i < 256; i++)
   {
      cols[i].pixel = (unsigned long)i;
      cols[i].red = cols[i].green = cols[i].blue = 0;
      cols[i].flags = DoRed | DoGreen | DoBlue;
   }

   XQueryColors(x11->display, x11->colormap, cols, 256);

   for (i = 0; i < 256; i++)
   {
      rm[i] = (unsigned char)(cols[i].red >> 8);
      gm[i] = (unsigned char)(cols[i].green >> 8);
      bm[i] = (unsigned char)(cols[i].blue >> 8);
   }

   imagetype(im) = PIX_RLECMAP;
   imageheight(im) = dh;
   imagewidth(im) = dw;
   imagedate(im) = time(0);
   titlelength(im) = 0;
   setcmap(im, 256, rm, gm, bm);

   writeheader(im);

   line = (unsigned char *)ximage->data;
   for (i = 0; i < dh; i++)
   {
      writemappedline(im, line);
      line += ximage->bytes_per_line;
   }

   closeimage(im);

   free(rm);
   free(gm);
   free(bm);
   free(cols);
   XDestroyImage(ximage);
}

#endif

#ifndef VOGLE
/*
 * X11_setlw
 *
 *	Set the line width....
 */
static int
X11_setlw(int w)
{
   XGCValues vals;

   vals.line_width = w;
   XChangeGC(x11->display, x11->theGC, GCLineWidth, &vals);

   erturn(1);
}

/*
 * X11_setls
 *
 *	Set the line style....
 */

static int
X11_setls(int lss)
{
   unsigned ls = lss;
   char	dashes[16];
   int	i, n, a, b, offset;

   if (ls == 0xffff)
   {
      XSetLineAttributes(x11->display, x11->theGC, vdevice.attr->a.lw, LineSolid, CapButt, JoinMiter);
      return;
   }

   for (i = 0; i < 16; i++)
   {
      dashes[i] = 0;
   }
   
   for (i = 0; i < 16; i++)	/* Over 16 bits */
   {
      if ((ls & (1 << i)))
         break;
   }
   
   offset = i;

#define	ON	1
#define	OFF	0

   a = b = OFF;
   if (ls & (1 << 0))
      a = b = ON;

   n = 0;
   for (i = 0; i < 16; i++)
   {	/* Over 16 bits */
      if (ls & (1 << i))
         a = ON;
      else
         a = OFF;

      if (a != b)
      {
         b = a;
         n++;
      }
      dashes[n]++;
   }
   n++;

   XSetLineAttributes(x11->display, x11->theGC, vdevice.attr->a.lw, LineOnOffDash, CapButt, JoinMiter);
   XSetDashes(x11->display, x11->theGC, offset, dashes, n);

   return(1);
}

#else

/*
 * X11_setlw
 *
 *	Set the line width....
 */
static int
X11_setlw(int w)
{
   XGCValues vals;

   if (w != 0)
      w = 3;

   vals.line_width = w;
   XChangeGC(x11->display, x11->theGC, GCLineWidth, &vals);

   return(1);
}

#endif

/*
 * the device entry
 */
static DevEntry X11dev = {
   "X11",
   "large",
   "small",
   8,
   300, 300,
   300, 300,
   X11_backbuf,
   X11_char,
   X11_checkkey,
   X11_clear,
   X11_color,
   X11_draw,
   X11_fill,
   X11_font,
   X11_frontbuf,
   X11_getkey,
   X11_getevent,
   X11_winopen,
   X11_winset,
   X11_winclose,
   X11_winraise,
   X11_windel,
   X11_locator,
   X11_mapcolor,
   X11_pnt,
#ifndef VOGLE
   X11_setls,
#endif
   X11_setlw,
   X11_string,
   X11_swapbuf,
   X11_sync
};

/*
 * _X11_devcpy
 *
 *	copy the X11 device into vdevice.dev.
 */
void
_X11_devcpy(char *devname)
{
   /* Could do all sorts of things with the devname - like
   * have a different display.
   */
   vdevice.dev = X11dev;
}
