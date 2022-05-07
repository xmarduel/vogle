/*
 * VOGL/VOGLE driver for COCOA.
 *
 */

#ifdef COCOA

#import <AppKit/AppKit.h> 

/*
 * set VOGLE if this is really for the VOGLE library.
 */

#define VOGLE 1

#include <stdio.h>
#include <math.h>
#include <strings.h>
#include <time.h>

#ifdef VOGLE
#include "vogle.h"
#else
#include "vogl.h"
#endif

#define CMAPSIZE	4096   /* perhaps we'll go RGBmode someday. */

#define WINPOSX  100
#define WINPOSY  100

#define WINSIZEX 500
#define WINSIZEY 500

#pragma mark Events

@interface CView : NSView {
@public
	int nbMouseClicks;
	NSPoint currentMousePosition;
}

- (BOOL)acceptsFirstResponder;
- (void)windowWillClose:(NSNotification *)notification;

@end

@implementation CView

- (id)initWithFrame:(NSRect)frame
{
	NSLog(@"initWithFrame");
	self = [super initWithFrame:frame];
	if (self)
	{
		// Initialization code here.
		BOOL rc = [self becomeFirstResponder];
	}
	return self;
}

- (BOOL)acceptsFirstResponder
{
	NSLog(@"acceptsFirstResponder:");
	return YES;
}

- (void)windowWillClose:(NSNotification *)notification
{
	[NSApp terminate:self];
}

@end


typedef struct {
	
	NSWindow		*window;		/* .. local id, must be first */
	
	char *title;
   int id;  /* vogle id */
	
	NSApplication *application;
	
	CView   *view;
	NSImage *backbuf;
	
	id  drawable; /* draw in the drawable, the view or (for double buffering) the image */
	
	NSFont *font;
	NSMutableDictionary *fontAttributes;
	
	NSColor **colormap;
   
   unsigned int w, h;
	
	int depth;
	int back_used;
   
} COCOA_rec;

static COCOA_rec *cocoa;

/*
 * Functions 
 */
static int cocoa_draw(int x, int y);
static int cocoa_pnt(int x, int y);
static int cocoa_getkey(void);
static int cocoa_checkkey(void);
static int cocoa_locator(int* wx, int* wy);
static int cocoa_clear(void);
static int cocoa_color(int ind);
static int cocoa_mapcolor(int ind, int r, int g, int b);
static int cocoa_font(const char *fontname);
static int cocoa_char(char c);
static int cocoa_string(const char *s);
static int cocoa_fill(int n, int x[], int y[]);
static int cocoa_backbuf(void *p, int old, int bw, int bh);
static int cocoa_swapbuf(void);
static int cocoa_frontbuf(void);
static void cocoa_sync(void);
static int cocoa_setlw(int w);
#ifndef VOGLE
static int cocoa_setls(int lss);
#endif
static void * cocoa_winopen(const char *dev, const char *title, int id);
static int cocoa_winset(void * devwin);
static int cocoa_winclose(void * devwin);
static int cocoa_winraise(void * devwin);
static int cocoa_windel(void * devwin);
static unsigned long cocoa_getevent(Vevent *vev, int noblock);

void _COCOA_devcpy(char *dev);

/*
 * Tell vogle to use an existing COCOA window. To be called once for
 * each toolkit window that vogle is to use.
 *
 * Returns an integer for referencing the window from within vogle
 */

void cocoa_run_app()
{
	[NSApp run];
}

int vo_cocoa_window(NSApplication *app, int xw, int xh)
{
   int i;
	int ind;
	
   cocoa = (COCOA_rec *)vallocate(sizeof(COCOA_rec));
   if (cocoa == NULL)
   {
      return(0);
   }
   
   memset(cocoa, 0, sizeof(COCOA_rec));
	
   cocoa->id		= 10000;
	
	cocoa->application = app;
	
   cocoa->w 		= xw;
   cocoa->h 		= xh;
	
   _COCOA_devcpy("COCOA");
	
	cocoa->window = [ [NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, xw, xh)
									styleMask:NSResizableWindowMask|NSMiniaturizableWindowMask|NSClosableWindowMask|NSTitledWindowMask
									backing:NSBackingStoreBuffered
									defer:NO ];
	
	ind 				= _vgetfreewindowslot();
   vdevice.wins[ind].id 	      = ind;
   vdevice.wins[ind].devwin 	   = cocoa;
   vdevice.wins[ind].dev 	      = vdevice.dev;
   vdevice.wins[ind].localid 	   = (int)(cocoa->window);
   vdevice.wins[ind].havebackb 	= 0;
   vdevice.curwin 		         = ind;
	
	//cocoa->colormap = malloc(CMAPSIZE* sizeof(NSObject));
   cocoa->colormap = malloc(CMAPSIZE* sizeof(32));
	for (i=0; i<CMAPSIZE; i++)
	{
		cocoa->colormap[i] = nil;
	}
	cocoa->colormap[0] = [NSColor blackColor];
	cocoa->colormap[1] = [NSColor redColor];
	cocoa->colormap[2] = [NSColor greenColor];
	cocoa->colormap[3] = [NSColor yellowColor];
	cocoa->colormap[4] = [NSColor blueColor];
	cocoa->colormap[5] = [NSColor magentaColor];
	cocoa->colormap[6] = [NSColor cyanColor];
	cocoa->colormap[7] = [NSColor whiteColor];
	
	[cocoa->window setTitle:@"Tiny Application Window"];
	
	/* Create the CView for the Window.  */
	cocoa->view = [ [CView alloc] init];
	[cocoa->window setContentView:cocoa->view ];
	[cocoa->window setDelegate:cocoa->view ];
	[cocoa->window makeKeyAndOrderFront: nil];
	
	cocoa->font = nil;
	
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
   
   
   cocoa_color(0);
	
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
 * cocoa_winopen()
 * initializes the cocoa application/display.
 */

static void * cocoa_winopen(const char *devname, const char *title, int ind)
{
	int i;
	int prefx, prefy, prefxs, prefys;
	unsigned int  x, y, w, h;
	
	cocoa = (COCOA_rec *)vallocate(sizeof(COCOA_rec));
   
	if (cocoa == NULL)
   {
      return(0);
   }

	NSAutoreleasePool * pool    = [[NSAutoreleasePool alloc] init];
	NSApp = cocoa->application = [NSApplication sharedApplication];
	
	//MyApplicationDelegate * appDelegate = [[[[MyApplicationDelegate]alloc] init] autorelease];
	//[NSApp setDelegate:appDelegate];

	getprefposandsize(&prefx,&prefy,&prefxs,&prefys);
	cocoa->depth = 24;
	
	
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
	
	cocoa->id = ind;
	cocoa->window = [ [NSWindow alloc] initWithContentRect:NSMakeRect(x, y, w, h)
	                        styleMask:NSMiniaturizableWindowMask|NSClosableWindowMask|NSTitledWindowMask|NSResizableWindowMask 
									backing:NSBackingStoreBuffered
									defer:NO ];
	
	//cocoa->colormap = malloc(CMAPSIZE* sizeof(NSObject));
   cocoa->colormap = malloc(CMAPSIZE* sizeof(32));
	for (i=0; i<CMAPSIZE; i++)
	{
		cocoa->colormap[i] = nil;
	}
	cocoa->colormap[0] = [NSColor blackColor];
	cocoa->colormap[1] = [NSColor redColor];
	cocoa->colormap[2] = [NSColor greenColor];
	cocoa->colormap[3] = [NSColor yellowColor];
	cocoa->colormap[4] = [NSColor blueColor];
	cocoa->colormap[5] = [NSColor magentaColor];
	cocoa->colormap[6] = [NSColor cyanColor];
	cocoa->colormap[7] = [NSColor whiteColor];
	
	i = strlen(title);
	
   if (i==0)
   {
      cocoa->title = (char *)vallocate(6);
      strcpy(cocoa->title, "Vogle");
   }
   else
   {
      cocoa->title =  (char *)vallocate(i+1);
      strcpy(cocoa->title, title);
   }

	[cocoa->window setTitle:[NSString stringWithUTF8String:cocoa->title]];
	
	 // Create the CView for the Window.
	cocoa->view = [[CView alloc] init];
	
	[cocoa->window setContentView:cocoa->view];
	[cocoa->window setDelegate:cocoa->view];
	
	// draw op. on the view...
	[cocoa->view lockFocus];
	
	cocoa->drawable = cocoa->view; // well, for the double buffering if any...
	
	cocoa->font = nil;
	cocoa->fontAttributes = [[NSMutableDictionary alloc] init];
	
	vdevice.dev.sizeX = vdevice.dev.sizeY = MIN(w,h) - 1;
	cocoa->w = vdevice.dev.sizeSx = w;
   cocoa->h = vdevice.dev.sizeSy = h;
	cocoa->back_used = 0;
		
	
	[cocoa->window setInitialFirstResponder:cocoa->view];
	[cocoa->window makeFirstResponder:cocoa->view];
	[cocoa->window makeKeyAndOrderFront:nil];
	[cocoa->window makeKeyWindow];
	[cocoa->window becomeKeyWindow];
	[cocoa->window setAcceptsMouseMovedEvents:YES];
	
	/*
	if ([cocoa->window acceptsMouseMovedEvents])
	{
		NSLog(@"window now acceptsMouseMovedEvents");
   }
   else
   {
	   NSLog(@"FAILED window now acceptsMouseMovedEvents");
   }*/
	
	return (void*)cocoa ;
}

/*
 * cocoa_winset
 */
static int cocoa_winset(void *p)
{
   COCOA_rec *cocoa_rec = (COCOA_rec*)p;
   
   if (!cocoa_rec)
      return (-1);
	
   [cocoa_rec->window makeKeyWindow];

   return (cocoa_rec->id);
}

/*
 * cocoa_winclose
 */
static int cocoa_winclose(void *p)
{
   COCOA_rec *cocoa_rec = (COCOA_rec*)p;
   
   /* make it an icon */
   [cocoa_rec->window miniaturize:nil];
   
   return (1);
}

/*
 * cocoa_winraise
 */
static int cocoa_winraise(void *p)
{
   COCOA_rec *cocoa_rec = (COCOA_rec*)p;

   /* un-make it an icon */
   [cocoa_rec->window  deminiaturize:nil];

   return (1);
}

/*
 * cocoa_windel
 */
static int cocoa_windel(void *p)  // BUGGY !!! test with "lotsofwindows"
{
   COCOA_rec *cocoa_rec = (COCOA_rec*)p;
   
   [cocoa_rec->window close];
	if (cocoa_rec->backbuf) [cocoa_rec->backbuf release];
   
   return (1);
}

/*
 * cocoa_clear
 *
 * Clear the screen (or current buffer ) to current colour
 */
static int cocoa_clear()
{
	NSRect bounds = [cocoa->view bounds];
	[NSBezierPath fillRect:bounds];
	
	if (vdevice.sync)
		cocoa_sync();
	
	return (1);
}

/*
 * cocoa_pnt
 *
 *	draws a point
 *
 */
static int cocoa_pnt(int x, int y)
{
	// draw a rect if drawing to a view
	NSRect pt = NSMakeRect(x-0.4, y-0.4, 0.8, 0.8);
	[NSBezierPath fillRect:pt];
	
   return (1);
}

/*
 * cocoa_draw
 *
 *	draws a line from the current graphics position to (x, y).
 *
 */
static int cocoa_draw(int x, int y)
{	
	int isPoint = FALSE;
	if (fabs(x-vdevice.cpVx) < 1.0e-8 && fabs(y-vdevice.cpVy) < 1.0e-8)
	{
		isPoint = TRUE;
	}
	
	if (!isPoint)
	{
		NSBezierPath *path = [[NSBezierPath alloc] init];
		NSPoint startPoint = {vdevice.cpVx, vdevice.cpVy};
		NSPoint endPoint   = {x, y};
		
		[path moveToPoint:startPoint];
		[path lineToPoint:endPoint];
		[path stroke];
		[path release];
	}
	else
	{
		if (!cocoa->back_used)
		{
			// draw a rect if drawing to a view
			NSRect pt = NSMakeRect(x-0.4, y-0.4, 0.8, 0.8);
		   [NSBezierPath fillRect:pt];
		}
		else
		{
			// draw a pixel if drawing to an image
			NSRect pt = NSMakeRect(x-0.4, y-0.4, 0.8, 0.8);
		   [NSBezierPath fillRect:pt];
		}
	}
	
	if (vdevice.sync)
		cocoa_sync();
	
   return (1);
}

/*
 * cocoa_fill
 *
 *	fill a polygon
 */
static int cocoa_fill(int n, int x[], int y[])
{
	int k;
   
	NSBezierPath *path = [[NSBezierPath alloc] init];
	
	NSPoint points[n];
	
	for (k=0; k<n; k++)
	{
		points[k].x = x[k];
		points[k].y = y[k];
	}
   
	[path moveToPoint:points[0]];
	
   for (k=1; k<n; k++)
   {
		[path lineToPoint:points[k]];
   }
	
	[path closePath];	
	[path fill];	
	[path release];
	
	if (vdevice.sync)
		cocoa_sync();
   
	return(1);
}

static int cocoa_setlw(int w)
{
   if (w != 0) // TINY or THICK!
	{
		w = 3;
	}
	
	[NSBezierPath setDefaultLineWidth:(float)w];
   
   return (1);
}

/*
 * cocoa_color
 *
 *	set the current drawing color index.
 */
static int cocoa_color(int ind)
{
	[cocoa->colormap[ind] set];
	
	return (1);
}

/*
 * cocoa_mapcolor
 *
 *	change index i in the color map to the appropriate r, g, b, value.
 */
static int cocoa_mapcolor(int i, int r, int g, int b)
{
	if (i >= CMAPSIZE) return(-1);
	
	cocoa->colormap[i] = [NSColor colorWithDeviceRed:r/255.0 green:g/255.0 blue:b/255.0 alpha:1.0];
	
	return (1);
}

/*
 * cocoa_font
 *
 *   Set up a hardware font. Return 1 on success 0 otherwise.
 *
 * This is system-dependent.  I assume that the fontfile parameter
 * has the font family name followed by a blank, followed by the size 
 * in points, e.g. "Ohlfs 384.7", "Helvetica-BoldOblique 1.0".
 * Note that the size can be floating-point.
 *
 */

static int cocoa_font(const char *fontspec)
{
	NSString *fontName = [NSString stringWithUTF8String:fontspec];
	float fontSize     = 5.0;
	
   cocoa->font = [NSFont fontWithName:fontName size:fontSize];
	[cocoa->font set];
	
	[cocoa->fontAttributes setObject:[NSFont fontWithName:fontName size:fontSize] forKey:NSFontAttributeName];
	[cocoa->fontAttributes setObject:[NSColor redColor] forKey:NSForegroundColorAttributeName];
   
   return (1);
}

/* 
 * cocoa_char (outputs one char)
 */
static int cocoa_char(char c)
{
   char s[2] = "?";
	s[0] = c;
	
	NSString *ss = [NSString stringWithUTF8String:s];
	NSPoint  point = {vdevice.cpVx, vdevice.dev.sizeSy - vdevice.cpVy};
	
	[ss drawAtPoint:point withAttributes:cocoa->fontAttributes];
   
	if (vdevice.sync)
		cocoa_sync();
   
	return (1);	
}

/*
 * cocoa_string
 *
 *	Display a string at the current drawing position.
 */
static int cocoa_string(const char *s)
{
	NSString *ss = [NSString stringWithUTF8String:s];
	NSPoint  point = {vdevice.cpVx, vdevice.dev.sizeSy - vdevice.cpVy};
	
	[ss drawAtPoint:point withAttributes:cocoa->fontAttributes];
	
	if (vdevice.sync)
		cocoa_sync();
   
	return (1);
}

/*
 * cocoa_backbuf
 *
 *	Set up double buffering by allocating the back buffer and
 *	setting drawing into it.
 */
static int cocoa_backbuf(void *p, int old, int bw, int bh)
{	
	COCOA_rec *cocoa = (COCOA_rec*)p;
	
	[cocoa->drawable unlockFocus];
	
	NSRect bounds = [cocoa->view bounds];
	cocoa->backbuf = [[NSImage alloc] initWithSize:bounds.size];
	
		//if ( ![cocoa->backbuf useCacheWithDepth:NS_TwentyFourBitRGBDepth] )
		//{
		//   fprintf(stderr,"couldn't create backing buffer.\n");
		//   return 0;
	   //};

	cocoa->drawable = cocoa->backbuf;
	[cocoa->drawable lockFocus];
	
	cocoa->back_used = 1;
	
	return(1);
}

/*
 * cocoa_swapbuf
 *
 *	Swap the back and front buffers. (Really, just copy the
 *	back buffer to the screen).
 */
static int cocoa_swapbuf()
{
	[cocoa->drawable unlockFocus];
	[cocoa->view lockFocus];
	
	NSRect imageRect;
	imageRect.origin = NSZeroPoint;
	imageRect.size   = [cocoa->backbuf size];
	
	[cocoa->backbuf drawInRect:[cocoa->view bounds]
				fromRect:imageRect
			   operation:NSCompositeSourceOver
				fraction:1.0];
	
	[cocoa->window flushWindow];
	[cocoa->view unlockFocus];
	[cocoa->drawable lockFocus];
	
	return (1);
}

/*
 * cocoa_frontbuf
 *
 *	Make sure we draw to the screen.
 */
static int cocoa_frontbuf()
{
	[cocoa->drawable unlockFocus];
	cocoa->drawable = cocoa->view;
	[cocoa->drawable lockFocus];
	
	return (1);
}

/*
 * Syncronise the display with what we think has been sent to it...
 */
static void cocoa_sync()
{
	[cocoa->window flushWindow];
}

/*
 * cocoa_getevent
 *
 *	grab a character from the keyboard - blocks until one is there.
 *
 */
static unsigned long cocoa_getevent(Vevent *vev, int noblock)
{
	vev->type = 0;
	
	/*
	 enum {
	 NSLeftMouseDownMask = 1,
	 NSLeftMouseUpMask = 2,
	 NSRightMouseDownMask = 4,
	 NSRightMouseUpMask = 8,
	 NSMouseMovedMask = 16,
	 NSLeftMouseDraggedMask = 32,
	 NSRightMouseDraggedMask = 64,
	 NSMouseEnteredMask = 128,
	 NSMouseExitedMask = 256,
	 NSKeyDownMask = 512,
	 NSKeyUpMask = 1024,
	 NSFlagsChangedMask = 2048,
	 NSPeriodicMask = 4096,
	 NSCursorUpdateMask = 8192,
	 // Note that NSAnyEventMask is an OR-combination of all other event masks
	 NSAnyEventMask = 16383
	 };
	 */
	
	NSEvent* event = [NSApp nextEventMatchingMask:NSAnyEventMask
	   untilDate:[NSDate distantPast]
	   inMode:NSDefaultRunLoopMode /* NSDefaultRunLoopMode NSEventTrackingRunLoopMode NSModalPanelRunLoopMode NSConnectionReplyMode */
	   dequeue:YES];
	 
	if (event != nil)
	{
		/*
		 enum {
		 NSLeftMouseDown      = 1,
		 NSLeftMouseUp        = 2,
		 NSRightMouseDown     = 3,
		 NSRightMouseUp       = 4,
		 NSMouseMoved         = 5,
		 NSLeftMouseDragged   = 6,
		 NSRightMouseDragged  = 7,
		 NSMouseEntered       = 8,
		 NSMouseExited        = 9,
		 NSKeyDown            = 10,
		 NSKeyUp              = 11,
		 NSFlagsChanged       = 12,
		 NSAppKitDefined      = 13,
		 NSSystemDefined      = 14,
		 NSApplicationDefined = 15,
		 NSPeriodic           = 16,
		 NSCursorUpdate       = 17,
		 NSScrollWheel        = 22,
		 NSTabletPoint        = 23,
		 NSTabletProximity    = 24,
		 NSOtherMouseDown     = 25,
		 NSOtherMouseUp       = 26,
		 NSOtherMouseDragged  = 27
		 NSEventTypeGesture   = 29,
		 NSEventTypeMagnify   = 30,
		 NSEventTypeSwipe     = 31,
		 NSEventTypeRotate    = 18,
		 NSEventTypeBeginGesture = 19,
		 NSEventTypeEndGesture   = 20
		 };
		*/
		if ([event type] == NSLeftMouseDown)
		{
			vev->type = VBUTTONPRESS;
			vev->data = 1;
			vev->x = [event locationInWindow].x;
			vev->y = [event locationInWindow].y;
			
			NSLog(@"VBUTTONPRESS (LEFT)");
			
			return (unsigned long)(cocoa->window);
		}
		else
		if ([event type] == NSLeftMouseUp)
		{
			vev->type = VBUTTONRELEASE;
			vev->data = 1;
			vev->x = [event locationInWindow].x;
			vev->y = [event locationInWindow].y;
				
			NSLog(@"VBUTTONRELEASE (LEFT)");
				
			return (unsigned long)(cocoa->window);
		}
		else
		if ([event type] == NSRightMouseDown)
		{
			vev->type = VBUTTONPRESS;
			vev->data = 2;
			vev->x = [event locationInWindow].x;
			vev->y = [event locationInWindow].y;
			
			NSLog(@"VBUTTONPRESS (RIGHT)");
			
			return (unsigned long)(cocoa->window);
		}
		else
		if ([event type] == NSRightMouseUp)
		{
			vev->type = VBUTTONRELEASE;
			vev->data = 1;
			vev->x = [event locationInWindow].x;
			vev->y = [event locationInWindow].y;
			
			NSLog(@"VBUTTONRELEASE (RIGHT)");
			
			return (unsigned long)(cocoa->window);
		}
		else
		if ([event type] == NSKeyDown)
		{
			vev->type = VKEYPRESS;
			vev->data = [ [event characters] characterAtIndex:0];
			
			NSLog(@"VKEYPRESS");
			
			return (unsigned long)(cocoa->window);
		}
		else
		if ([event type] == NSKeyUp)
		{
			vev->type = VKEYRELEASE;
			vev->data = [ [event characters] characterAtIndex:0];
			
			NSLog(@"VKEYRELEASE");
			
			return (unsigned long)(cocoa->window);
		}
		else
		if ([event type] == NSMouseMoved)
		{
			vev->type = VMOTION;
			vev->data = 0;
			vev->x = [event locationInWindow].x;
			vev->y = [event locationInWindow].y;
				
			NSLog(@"VMOUSEMOVED");
				
			return 1;
		}
		else
		if ([event type] == NSLeftMouseDragged)
		{
			vev->type = VMOTION;
			vev->data = 1;
			vev->x = [event locationInWindow].x;
			vev->y = [event locationInWindow].y;
				
			NSLog(@"VMOUSEMOVED - LEFT BUTTON");
				
			return 1;
		}
		else
		if ([event type] == NSRightMouseDragged)
		{
			vev->type = VMOTION;
			vev->data = 2;
			vev->x = [event locationInWindow].x;
			vev->y = [event locationInWindow].y;
					
			NSLog(@"VMOUSEMOVED - RIGHTBUTTON");
					
			return 1;
		}
		if ([event type] == NSScrollWheel)
		{
			
			//vev->type = VSROLLWHEEL;
			vev->data = 0;
			
			NSLog(@"VSROLLWHEEL");
			
			return 0;
		}
		else 
		{
			NSLog(@"???? : %d" , [event type]);
			return 0;
		}
	}
	else
	{
		return (0);
	}
}

/*
 * cocoa_getkey
 *
 *	grab a character from the keyboard - blocks until one is there.
 *
 * do
 * {
 *   (void)(*vdevice.dev.Vgetevent)(&vev, 1);
 * } while (vev.type != -1 && vev.type != VKEYPRESS);
 *
 */
static int cocoa_getkey()
{
	Vevent vev;
	
	do
	{
		cocoa_getevent(&vev, 0);
	} 
	while (vev.type != -1 && vev.type != VKEYPRESS);
	
	return (1);
}

/*
 * cocoa_checkkey
 *
 *	Check if there has been a keyboard key pressed.
 *	and return it if so.
 */
static int cocoa_checkkey()
{
	Vevent vev;

	cocoa_getevent(&vev, 0);
	 
	if (vev.type == VKEYPRESS)
	{
		return vev.data;
	}
	else
	{
		return 0;
	}

}

/*
 * cocoa_locator
 *
 * return the window location of the cursor, plus which mouse button,
 * if any, is been pressed.
 * 
 */
static int cocoa_locator(int *wx, int *wy)
{	
		//[cocoa->application run]; // for tests...
	
	NSPoint loc = [cocoa->window mouseLocationOutsideOfEventStream];
	
	*wx = (int)loc.x;
	*wy = (int)loc.y;
	
	/*
	 is it possible as with X11 to get the current button pressed, 'outside events' ?
	 */
	
	return (0);
}

/*
 * the device entry
 */
static DevEntry COCOAdev = {
	"cocoa",
	"small",
	"large",
   8,
   WINSIZEX, WINSIZEY,
   WINSIZEX, WINSIZEY,
	cocoa_backbuf,
	cocoa_char,
	cocoa_checkkey,
	cocoa_clear,
	cocoa_color,
	cocoa_draw,
	cocoa_fill,
	cocoa_font,
	cocoa_frontbuf,
	cocoa_getkey,
   cocoa_getevent,
	cocoa_winopen,
   cocoa_winset,
   cocoa_winclose,
   cocoa_winraise,
   cocoa_windel,
	cocoa_locator,
	cocoa_mapcolor,
   cocoa_pnt,
#ifndef VOGLE
	cocoa_setls,
#endif
	cocoa_setlw,
	cocoa_string,
	cocoa_swapbuf, 
	cocoa_sync
};


/*
 * _COCOA_devcpy
 *
 *	copy the COCOA device into vdevice.dev.
 */
void _COCOA_devcpy(char *dev)
{
	vdevice.dev = COCOAdev; // set the current "device" to the global variable vdevice. the created window will also
									// have a pointer to this device
}

#endif /* COCOA */
