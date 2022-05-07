/*
 * VOGL/VOGLE driver for MACOSX-CARBON.
 *
 * Note: basic gui utilize the "old" carbon events (see carbon_winopen and carbon_getevent)
 *       gui with vo_cb_window utilize the "new" carbon events (full-feature gui and callbacks)
 *
 */

#ifdef CARBON

#include <Carbon/Carbon.h>

/*
 * set VOGLE if this is really for the VOGLE library.
 */

#define VOGLE 1

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>

#ifdef VOGLE
#include "vogle.h"
#else
#include "vogl.h"
#endif

#define LARGEFONT       "Times-Bold"
#define SMALLFONT       "Times-Bold"


#define MIN(x,y)	((x) < (y) ? (x) : (y))
#define MAX(x,y)	((x) > (y) ? (x) : (y))

#define	CMAPSIZE	256   /* perhaps we'll go RGBmode someday. */

#define WINPOSX  50
#define WINPOSY  50

#define WINSIZEX 500
#define WINSIZEY 500


char *carbonEventDescription[] = /* for event debugging */
{ 
   "nullEvent" ,
   "mouseDown" ,
   "mouseUp" ,
   "keyDown" ,
   "keyUp" ,
   "autoKey" ,
   "updateEvt" ,
   "diskEvt" ,
   "activateEvt" ,
   "" ,
   "" ,
   "" ,
   "" ,
   "" ,
   "" ,
   "osEvt" ,
   "" ,
   "" ,
   "" ,
   "" ,
   "" ,
   "" ,
   "" ,
   "kHighLevelEvent" ,
   "" ,
   "" ,
   "" ,
   "" ,
   "" ,
   "" ,
   "" ,
   "" ,
   "" ,
   "" ,
   "" ,
   "" ,
   "" ,
   "" ,
   "" ,
   "" ,
};

typedef struct RGB {
   float r, g, b;
} RGB;
   
typedef struct {

   char *title;
   
   WindowRef window;
   CGContextRef context;
   
   int id;  /* vogle id */
   
   RGB colormap[CMAPSIZE];
   
   unsigned int w, h;
   
   int depth;
   int initialized;
   
} CARBON_rec;


static CARBON_rec *carbon;


/*
 * Functions 
 */
static int carbon_draw(int x, int y);
static int carbon_pnt(int x, int y);
static int carbon_getkey(void);
static int carbon_checkkey(void);
static int carbon_locator(int* wx, int* wy);
static int carbon_clear(void);
static int carbon_color(int ind);
static int carbon_mapcolor(int ind, int r, int g, int b);
static int carbon_font(const char *fontname);
static int carbon_char(char c);
static int carbon_string(const char *s);
static int carbon_fill(int n, int [], int y[]);
static int carbon_backbuf(void *p, int old, int bw, int bh);
static int carbon_swapbuf(void);
static int carbon_frontbuf(void);
static void carbon_sync(void);
static int carbon_setlw(int w);
static int carbon_setls(int lss);
static void * carbon_winopen(const char *dev, const char *title, int id);
static int carbon_winset(void * devwin);
static int carbon_winclose(void * devwin);
static int carbon_winraise(void * devwin);
static int carbon_windel(void * devwin);
static unsigned long carbon_getevent(Vevent *vev, int noblock);

static int _carbon_fillrect(Rect rect);

void _CARBON_devcpy(char *dev);

/*
 * Tell vogle to use an existing CARBON window. To be called once for
 * each toolkit window that vogle is to use.
 *
 * Returns an integer for referencing the window from within vogle
 */
 
int vo_cb_window(WindowRef win, int xw, int xh)
{
   int	ind;

   carbon = (CARBON_rec *)vallocate(sizeof(CARBON_rec));
   if (carbon == NULL)
   {
      return(0);
   }
   
   memset(carbon, 0, sizeof(CARBON_rec));

   carbon->id		= 10000;
   carbon->window		= win;

   carbon->w 		= xw;
   carbon->h 		= xh;

   _CARBON_devcpy("CARBON");
   ind 				= _vgetfreewindowslot();
   vdevice.wins[ind].id 	      = ind;
   vdevice.wins[ind].devwin 	   = carbon;
   vdevice.wins[ind].dev 	      = vdevice.dev;
   vdevice.wins[ind].localid 	   = (int)win;
   vdevice.wins[ind].havebackb 	= 0;
   vdevice.curwin 		         = ind;

   /*
    * Set our standard colors...
    */
   carbon->depth = 24;
   //
   carbon_mapcolor(BLACK  ,   0,   0,   0);
   carbon_mapcolor(RED    , 255,   0,   0);
   carbon_mapcolor(GREEN  ,   0, 255,   0);
   carbon_mapcolor(YELLOW , 255, 255,   0);
   carbon_mapcolor(BLUE   ,   0,   0, 255);
   carbon_mapcolor(MAGENTA, 255,   0, 255);
   carbon_mapcolor(CYAN   ,   0, 255, 255);
   carbon_mapcolor(WHITE  , 255, 255, 255);
   

   /*
   if ((x11->smallf = XGetDefault(x11->display, me, "smallfont")) == (char *)NULL)
   {
      x11->smallf = SMALLFONT;
   }
   
   if ((x11->largef = XGetDefault(x11->display, me, "largefont")) == (char *)NULL)
   {
      x11->largef = LARGEFONT;
   }
   */
   
   /*
    * Create Graphics Context
    */
   SetPortWindowPort (carbon->window); 
   QDBeginCGContext (GetWindowPort (carbon->window), &(carbon->context));
   
   carbon_color(0);

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

int vo_cb_win_size(int w, int xw, int xh)
{
   CARBON_rec	*carbonwin;

   if (w < 0 || w >= vdevice.nwin)
      return(-1);

   if (vdevice.wins[w].id == -1)
      return(-1);

   carbonwin = (CARBON_rec *)vdevice.wins[w].devwin;

   carbonwin->w = xw;
   carbonwin->h = xh;
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

WindowRef vo_cb_get_window(int w)
{
   CARBON_rec	*carbonwin;

   if (w < 0 || w >= vdevice.nwin)
      return(0);

   if (vdevice.wins[w].id == -1)
      return(0);

   carbonwin = (CARBON_rec *)vdevice.wins[w].devwin;

   return(carbonwin->window);
}

/*
 * carbon_winopen()
 * initializes the carbon display.
 */
static void * carbon_winopen(const char *devname, const char *title, int id)
{
   int i;
   int prefx, prefy, prefxs, prefys;
   unsigned int  x, y, w, h;
   
   Rect windowBounds; 
   WindowAttributes windowAttr;
                  
   carbon = (CARBON_rec *)vallocate(sizeof(CARBON_rec));
   if (carbon == NULL)
   {
      return(0);
   }

   getprefposandsize(&prefx,&prefy,&prefxs,&prefys);

   if (prefx != -1)
   {
      x = prefx;
      y = prefy;
   }
   else
   {
      x = WINPOSX;
      y = WINPOSY;
   }
   
   if (prefxs > -1)
   {
      w = prefxs;
      h = prefys;
   }
   else
   {
      w = WINSIZEX;
      h = WINSIZEY;
   }   
     
     
   /*
   typedef UInt32 WindowAttributes;
enum {
   kWindowNoAttributes = 0,
   kWindowCloseBoxAttribute = (1L << 0),
   kWindowHorizontalZoomAttribute = (1L << 1),
   kWindowVerticalZoomAttribute = (1L << 2),
   kWindowFullZoomAttribute = (kWindowVerticalZoomAttribute | kWindowHorizontalZoomAttribute),
   kWindowCollapseBoxAttribute = (1L << 3),
   kWindowResizableAttribute = (1L << 4),
   kWindowSideTitlebarAttribute = (1L << 5),
   kWindowToolbarButtonAttribute = (1L << 6),
   kWindowMetalAttribute = (1L << 8),
   kWindowNoUpdatesAttribute = (1L << 16),
   kWindowNoActivatesAttribute = (1L << 17),
   kWindowOpaqueForEventsAttribute = (1L << 18),
   kWindowCompositingAttribute = (1L << 19),
   kWindowNoShadowAttribute = (1L << 21),
   kWindowHideOnSuspendAttribute = (1L << 24),
   kWindowStandardHandlerAttribute = (1L << 25),
   kWindowHideOnFullScreenAttribute = (1L << 26),
   kWindowInWindowMenuAttribute = (1L << 27),
   kWindowLiveResizeAttribute = (1L << 28),
   kWindowIgnoreClicksAttribute = (1L << 29),
   kWindowNoConstrainAttribute = (1L << 31),
   kWindowStandardDocumentAttributes = (kWindowCloseBoxAttribute | kWindowFullZoomAttribute | kWindowCollapseBoxAttribute | kWindowResizableAttribute),
   kWindowStandardFloatingAttributes = (kWindowCloseBoxAttribute | kWindowCollapseBoxAttribute)
};
   */
     
 windowAttr = kWindowStandardDocumentAttributes | // standard window 
	             kWindowStandardFloatingAttributes /*|
                kWindowStandardHandlerAttribute */   // standard window event handler
	             ;
  
 windowAttr = kWindowStandardDocumentAttributes | // standard window 
	             kWindowCompositingAttribute  |
                kWindowLiveResizeAttribute 
	             ;


   SetRect(&windowBounds, x, y, w+x, h+y); // bounds for the new window 
    
   CreateNewWindow(kDocumentWindowClass, windowAttr, &windowBounds, &carbon->window);
    
   i = strlen(title);
  
   if (i==0)
   {
      carbon->title = (char *)vallocate(6);
      strcpy(carbon->title, "carbon vogle");
   }
   else
   {
      carbon->title =  (char *)vallocate(i+1);
      strcpy(carbon->title, title);
   }
  
   
   /*
   enum CFStringBuiltInEncodings {
   
   kCFStringEncodingMacRoman = 0,
   kCFStringEncodingWindowsLatin1 = 0x0500,
   kCFStringEncodingISOLatin1 = 0x0201,
   kCFStringEncodingNextStepLatin = 0x0B01,
   kCFStringEncodingASCII = 0x0600,
   kCFStringEncodingUnicode = 0x0100,
   kCFStringEncodingUTF8 = 0x08000100,
   kCFStringEncodingNonLossyASCII = 0x0BFF,

   // The following constants are available
   // only on Mac OS X v10.4 and later,
   kCFStringEncodingUTF16 = 0x0100,
   kCFStringEncodingUTF16BE = 0x10000100,
   kCFStringEncodingUTF16LE = 0x14000100,
   kCFStringEncodingUTF32 = 0x0c000100,
   kCFStringEncodingUTF32BE = 0x18000100,
   kCFStringEncodingUTF32LE = 0x1c000100
   };

   */
   
   SetWindowTitleWithCFString(carbon->window, CFStringCreateWithCString(kCFAllocatorDefault, carbon->title, kCFStringEncodingASCII));
   //
   carbon->depth = 24;
   //
   carbon_mapcolor(BLACK  ,   0,   0,   0);
   carbon_mapcolor(RED    , 255,   0,   0);
   carbon_mapcolor(GREEN  ,   0, 255,   0);
   carbon_mapcolor(YELLOW , 255, 255,   0);
   carbon_mapcolor(BLUE   ,   0,   0, 255);
   carbon_mapcolor(MAGENTA, 255,   0, 255);
   carbon_mapcolor(CYAN   ,   0, 255, 255);
   carbon_mapcolor(WHITE  , 255, 255, 255);
   //
   // The window was created hidden so show it.
   ShowWindow(carbon->window);
   //
   vdevice.dev.sizeX = vdevice.dev.sizeY = MIN(w,h) ;
   carbon->w = vdevice.dev.sizeSx = w;
   carbon->h = vdevice.dev.sizeSy = h;
   //
   SetPortWindowPort (carbon->window); 
   QDBeginCGContext (GetWindowPort (carbon->window), &(carbon->context)); 
   //
   carbon->initialized = 1;
   //
   return (void*)carbon;
}


/*
 * carbon_winset
 */
static int carbon_winset(void *p)
{
   CARBON_rec *acarbonwin = (CARBON_rec*)p;
   
   if (!acarbonwin)
      return (-1);
      
   SelectWindow(acarbonwin->window);
   
   carbon = acarbonwin;
   return (carbon->id);
}

/*
 * carbon_winclose
 */
static int carbon_winclose(void *p)
{
   CARBON_rec *acarbonwin = (CARBON_rec*)p;
   
   /* make it an icon */
   CollapseWindow(acarbonwin->window, TRUE);
   
   return (1);
}

/*
 * carbon_winraise
 */
static int carbon_winraise(void *p)
{
   CARBON_rec *acarbonwin = (CARBON_rec*)p;
   
   /* un-make it an icon */
   CollapseWindow(acarbonwin->window, FALSE);
   
   return (1);
}

/*
 * carbon_windel
 */
static int carbon_windel(void *p)
{
   CARBON_rec *acarbonwin = (CARBON_rec*)p;
   
   DisposeWindow(acarbonwin->window);
   
   return (1);
}

/*
enum {
   mDownMask = 1 << mouseDown,
   mUpMask = 1 << mouseUp,
   keyDownMask = 1 << keyDown,
   keyUpMask = 1 << keyUp,
   autoKeyMask = 1 << autoKey,
   updateMask = 1 << updateEvt,
   diskMask = 1 << diskEvt,
   activMask = 1 << activateEvt,
   highLevelEventMask = 0x0400,
   osMask = 1 << osEvt,
   everyEvent = 0xFFFF
};
typedef UInt16 EventMask;
*/
   
/*
enum {
   nullEvent = 0,
   mouseDown = 1,
   mouseUp = 2,
   keyDown = 3,
   keyUp = 4,
   autoKey = 5,
   updateEvt = 6,
   diskEvt = 7,
   activateEvt = 8,
   osEvt = 15,
   kHighLevelEvent = 23
};
*/

/*
OSStatus ReceiveNextEvent (
   UInt32 inNumTypes,
   const EventTypeSpec * inList,
   EventTimeout inTimeout,
   Boolean inPullEvent,
   EventRef * outEvent
);
*/
  
static int old_event_type = -1; // available in carbon_getevent and carbon_locator !!!

static unsigned long carbon_getevent(Vevent *vev, int noblock)
{
  // static int old_event_type = -1;
   
   EventMask   theMask = everyEvent;
   EventRecord theEvent;
   int sleep   = -1;
   int nosleep =  0;
   RgnHandle   mouseRgn = NULL;
   
   Boolean res;
   
   if (noblock)
   {
      res = WaitNextEvent(theMask, &theEvent, nosleep, mouseRgn);
   }
   else
   {
      res = WaitNextEvent(theMask, &theEvent, sleep, mouseRgn);
   }

   //printf("carbon_getevent (block=%d): theEvent.what = %d - %s\n", block, theEvent.what, carbonEventDescription[theEvent.what]);
   
   switch (theEvent.what)
   {
      case nullEvent:
         if (old_event_type == VBUTTONPRESS)
         {
            vev->type = VMOTION;
            vev->data = 1;
            vev->x    = theEvent.where.v;
            vev->y    = theEvent.where.h;
         }
         else
         {
            vev->data = 0;
         }
         break;
       
      case mouseDown:
         vev->type = VBUTTONPRESS;
         vev->data = 1;
         vev->x    = theEvent.where.v;
         vev->y    = theEvent.where.h;
         //
         old_event_type = VBUTTONPRESS;
         //
         break;
   
      case mouseUp:
         vev->type = VBUTTONRELEASE;
         vev->data = 1;
         vev->x    = theEvent.where.v;
         vev->y    = theEvent.where.h;
         //
         old_event_type = VBUTTONRELEASE;
         //
         break;
   
      case keyDown:
         vev->type = VKEYPRESS;
         vev->data = theEvent.message & charCodeMask;
         vev->x    = theEvent.where.v;
         vev->y    = theEvent.where.h;
         //
         old_event_type = VKEYPRESS;
         //
         break;
   
      case keyUp:
         vev->type = VKEYRELEASE;
         vev->data = theEvent.message & charCodeMask;
         vev->x    = theEvent.where.v;
         vev->y    = theEvent.where.h;
         //
         old_event_type = VKEYRELEASE;
         //
         break;
      
      case autoKey:
         if (old_event_type == VKEYPRESS)
         {
            vev->type = VKEYPRESS;
            vev->data = theEvent.message & charCodeMask;
            vev->x    = theEvent.where.v;
            vev->y    = theEvent.where.h;
         }
         else
         {
            vev->data =  0;
            vev->x    = -1;
            vev->y    = -1;
         }
         break;
      
      case updateEvt:
         vev->type =  0;
         vev->x    = -1;
         vev->y    = -1;
         break;
       
      case diskEvt:  
         vev->type = 0;
         vev->x    = -1;
         vev->y    = -1;
         break;
       
      case activateEvt:
         vev->type =  0;
         vev->x    = -1;
         vev->y    = -1;
         break;
       
      case osEvt:
         vev->type =  0;
         vev->x    = -1;
         vev->y    = -1;
         break;
       
      case kHighLevelEvent:
         vev->type =  0;
         vev->x    = -1;
         vev->y    = -1;
         break;
       
      default:
         vev->type =  0;
         vev->x    = -1;
         vev->y    = -1;
         break;
   }

    return (1);
}

/*
 * carbon_getkey
 *
 *	grab a character from the keyboard - blocks until one is there.
 */
static int carbon_getkey(void)
{
   char	c;

   EventMask theMask = everyEvent;
   EventRecord theEvent;
   int sleep = -1;
   RgnHandle mouseRgn = NULL;
   
   Boolean res;
   
   do
   {
      res = WaitNextEvent(theMask, &theEvent, sleep, mouseRgn);
      
      if (theEvent.what == keyDown)
      {
         c = theEvent.message & charCodeMask ;
         
         if (c)
         {
            return((int)c);
         }
         else
         {
            return(0);
         }
      }
      
   } while (theEvent.what != keyDown);

   return(0);

}
 
/*
 * carbon_checkkey
 *
 *	Check if there has been a keyboard key pressed.
 *	and return it if so.
 */
static int carbon_checkkey(void)
{
   char	c;
   
   EventMask theMask = everyEvent;
   EventRecord theEvent;
   int nosleep = 1;
   RgnHandle mouseRgn = NULL;
   
   Boolean res;
   
   res = WaitNextEvent(theMask, &theEvent, nosleep, mouseRgn);
      
   if (theEvent.what == keyDown)
   {
      c = theEvent.message & charCodeMask ;
         
      if (c)
      {
         return((int)c);
      }
      else
      {
         return(-1);
      }
   }
   
   return -1;
}

/*
 * carbon_locator
 *
 * return the window location of the cursor, plus which mouse button,
 * if any, is been pressed.
 * 
 * NOT YET IMPLEMENTED : find with button (with any modifiers) is pressed...
 */
static int carbon_locator(int *wx, int *wy)
{
   OSStatus res;

   GrafPtr inPort       = GetWindowPort (carbon->window);
   OptionBits inOptions = kTrackMouseLocationOptionDontConsumeMouseUp;
   Point                outPt;
   UInt32               outModifiers;
   MouseTrackingResult  outResult;
   
   res =  TrackMouseLocationWithOptions (
      /*GrafPtr inPort*/ inPort,
      /*OptionBits inOptions*/ inOptions,
      /*EventTimeout inTimeout*/ 0,
      /*Point * outPt*/ &outPt,
      /*UInt32 * outModifiers*/ &outModifiers,
      /*MouseTrackingResult * outResult*/ &outResult
   );

   printf("carbon_locator : pos = (%04d - %04d)  (mouse info = %d)\n", outPt.h, outPt.v, outResult);

   *wx = outPt.h;
   *wy = (int)vdevice.dev.sizeSy - outPt.v;
   
   /*
   typedef UInt16 MouseTrackingResult;
   enum 
   {
      kMouseTrackingMouseDown = 1,
      kMouseTrackingMouseUp = 2,
      kMouseTrackingMouseExited = 3,
      kMouseTrackingMouseEntered = 4,
      kMouseTrackingMouseDragged = 5,
      kMouseTrackingKeyModifiersChanged = 6,
      kMouseTrackingUserCancelled = 7,
      kMouseTrackingTimedOut = 8,
      kMouseTrackingMouseMoved = 9
   };
   */

   if (outResult == kMouseTrackingMouseDown) // outResult does not work ???
   {
      return 1; // the button number which is pressed ... (1, 2 or 4)
   }
   else
   {
      return 0; // no button pressed ... 
   }
}

/*
 * carbon_clear
 *
 * Clear the screen (or current buffer )to current colour
 */
static int carbon_clear(void)
{
   Rect rect;
   GetWindowBounds(carbon->window, kWindowGlobalPortRgn, &rect);
   
   _carbon_fillrect(rect);

   return (1);
}

/*
 * carbon_color
 *
 *	set the current drawing color index.
 */
static int carbon_color(int ind)
{
   float r = carbon->colormap[ind].r;
   float g = carbon->colormap[ind].g;
   float b = carbon->colormap[ind].b;
   
   CGContextSetRGBFillColor(carbon->context, r, g, b, 1.0);
   CGContextSetRGBStrokeColor(carbon->context, r, g, b, 1.0);
   
   return (1);
}

/*
 *	change index i in the color map to the appropriate r, g, b, value.
 */
static int carbon_mapcolor(int i, int r, int g, int b)
{
   if (i >= CMAPSIZE) return(-1);

   carbon->colormap[i].r = r/255.0;
   carbon->colormap[i].g = g/255.0;
   carbon->colormap[i].b = b/255.0;

   return (1);
}

/*
 * carbon_font
 *
 *   Set up a hardware font. Return 1 on success 0 otherwise.
 *
 * This is system-dependent.  I assume that the fontfile parameter
 * has the font family name followed by a blank, followed by the size 
 * in points, e.g. "Ohlfs 384.7", "Helvetica-BoldOblique 1.0".
 * Note that the size can be floating-point.
 *
 */

  
static int carbon_font(const char *fontfile)
{
   /*
   enum CGTextEncoding
   {
      kCGEncodingFontSpecific,
      kCGEncodingMacRoman
   };
   */
   CGContextSelectFont(carbon->context, fontfile, 1.0, kCGEncodingFontSpecific);
   
   return (1);
}

static int carbon_setlw(int w)
{
   CGContextSetLineWidth(carbon->context, (float)w);
   
   return (1);
}

#ifndef VOGLE
static int carbon_setls(int w)
{
   CGContextSetLineWidth(carbon->context, (float)w);
   
   return (1);
}
#endif

/*
 * carbon_pnt
 *
 *	draws a point
 *
 */
static int carbon_pnt(int x, int y)
{
   CGContextBeginPath(carbon->context);
   CGContextMoveToPoint(carbon->context, x, y); /* move to current point */
   CGContextAddLineToPoint(carbon->context, x, y);
   CGContextStrokePath(carbon->context);
   
   return (1);
}

/*
 * carbon_draw
 *
 *	draws a line from the current graphics position to (x, y).
 *
 */
static int carbon_draw(int x, int y)
{
   CGContextBeginPath(carbon->context);
   CGContextMoveToPoint(carbon->context, vdevice.cpVx, vdevice.cpVy); /* move to current point */
   CGContextAddLineToPoint(carbon->context, x, y);
   CGContextStrokePath(carbon->context);
  
   return (1);
}

/* 
 * carbon_char (outputs one char)
 */
static int carbon_char(char c)
{
   CGContextShowTextAtPoint(carbon->context, vdevice.cpVx, vdevice.dev.sizeSy - vdevice.cpVy, &c, 1);
   
   return (1);
}

/*
 * carbon_string
 *
 *	Display a string at the current drawing position.
 */
static int carbon_string(const char *s)
{
   CGContextShowTextAtPoint (carbon->context, vdevice.cpVx, vdevice.dev.sizeSy - vdevice.cpVy, s, strlen(s));
   
   return (1);
}

/*
 * carbon_fill
 *
 *	fill a polygon
 */
static int carbon_fill(int n, int x[], int y[])
{
   int k;
   
   CGPoint points[n];
   
   for (k=0; k<n; k++)
   {
      points[k].x = x[k];
      points[k].y = y[k];
   }
   
   CGContextBeginPath(carbon->context);
   CGContextAddLines(carbon->context, points, n);
   CGContextFillPath(carbon->context);
	
   return(1);
}

/*
 * _carbon_fillrect
 *
 *	fill a rectangle
 */
static int _carbon_fillrect(Rect rect)
{
   CGContextFillRect (carbon->context, CGRectMake (0, 0, rect.right - rect.left, rect.bottom - rect.top));   
   
   return(1);
}

/*
 * -
 *
 *	Set up double buffering by allocating the back buffer and
 *	setting drawing into it.
 */
static int carbon_backbuf(void *p, int old, int bw, int bh)
{
   return (1);
}

/*
 * carbon_swapbuf
 *
 *	Swap the back and front buffers. (Really, just copy the
 *	back buffer to the screen).
 */
static int carbon_swapbuf(void)
{
    carbon_sync();
    return (1);
}

/*
 * carbon_frontbuf
 *
 *	Make sure we draw to the screen.
 */
static int carbon_frontbuf(void)
{
   return (1);
}

/*
 * Syncronise the display with what we think has been sent to it...
 */
static void carbon_sync()
{
   CGContextFlush(carbon->context);
}

/*
 * the device entry
 */
static DevEntry carbondev = {
	"carbon",
	"small",
	"large",
   8,
   WINSIZEX, WINSIZEY,
   WINSIZEX, WINSIZEY,
	carbon_backbuf,
	carbon_char,
	carbon_checkkey,
	carbon_clear,
	carbon_color,
	carbon_draw,
	carbon_fill,
	carbon_font,
	carbon_frontbuf,
	carbon_getkey,
   carbon_getevent,
	carbon_winopen,
   carbon_winset,
   carbon_winclose,
   carbon_winraise,
   carbon_windel,
	carbon_locator,
	carbon_mapcolor,
   carbon_pnt,
#ifndef VOGLE
	carbon_setls,
#endif
	carbon_setlw,
	carbon_string,
	carbon_swapbuf, 
	carbon_sync
};

/*
 * _carbon_devcpy
 *
 *	copy the carbon device into vdevice.dev.
 */
void _CARBON_devcpy(char *devname)
{
	vdevice.dev = carbondev;
}

/* -------------------------------------------------------------- */
/* -------------------------------------------------------------- */

/*
      IBNibRef 		nibRef;
      
      // Create a Nib reference passing the name of the nib file (without the .nib extension)
      // CreateNibReference only searches into the application bundle.
      err = CreateNibReference(CFSTR("main"), &nibRef);
      require_noerr( err, CantGetNibRef );
    
      // Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
      // object. This name is set in InterfaceBuilder when the nib is created.
      err = SetMenuBarFromNib(nibRef, CFSTR("MenuBar"));
      require_noerr( err, CantSetMenuBar );
    
      // Then create a window. "MainWindow" is the name of the window object. This name is set in 
      // InterfaceBuilder when the nib is created.
      err = CreateWindowFromNib(nibRef, CFSTR("MainWindow"), &carbon->window);
      require_noerr( err, CantCreateWindow );

      // We don't need the nib reference anymore.
      DisposeNibReference(nibRef);
      
      //err = SetWindowBounds(carbon->window, kWindowGlobalPortRgn,  &windowBounds);
      //require_noerr( err, CantSetWindowSize );
*/
   
#endif // #ifdef CARBON