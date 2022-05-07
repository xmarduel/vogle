
/** \file vopl.c
 *
 */

#include "vopl.h"

#include <stdlib.h>
#include <float.h>
#include <string.h>

/*----------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------*/

vopldev *plotdev = NULL;

voplDevice  vopl_device =
{
   NULL, /* pointer to array of windows */
   0,    /* nwins  */
   -1    /* curwin */
};

/*----------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------*/

static char *vopl_version = "vopl-1.70";

static vopldev* vopldev_get();
static void vopldev_init (vopldev *dev);
static void vopldev_axis_init(axisdata *axis);

static int  _voplgetfreeplotdevslot(void);
static void _voplsetcurrentplotdev(vopldev *plotdev);

/*----------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------*/

char *vopl_get_version(void)
{
   return vopl_version;
}

/*----------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------*/

int vopl_winopen(const char *device, const char *title)
{
   static int count;
   char xtitle[128];
   int ind;

   /* increment window count */
   count = count + 1;
   
   if ( title == NULL )
   {
      snprintf(xtitle, 127, "VOPL-%d", count);
   }
   else
   {
      strncpy(xtitle, title, 127);
   }
   
   /* open a new window */
   ind = winopen(device, xtitle);

   /* allocate mem for vopl_device (same as allocation for the vogle window...) */
   vopl_wininit(ind);
   
   /* return id on the new window */
   return ind;
}

/*----------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------*/

int vopl_wininit(int win)
{
   /* called when the vogle window is created with a toolkit, not with vopl_wimopen */

   /* allocate mem for vopl_device */
   _voplgetfreeplotdevslot();

   vopl_device.curwin         = win;

   vopl_device.window[win].id = win;

   vopl_device.window[win].nx = 1; /* default: 1 plot per window */
   vopl_device.window[win].ny = 1; /* default: 1 plot per window */

   /* allocate 1 plotdev first as default */
   vopl_device.window[win].plotdev = vopldev_get();

   /* and set the default plotdev to this one (or the first of the array of plotdev owned by this window) */
   _voplsetcurrentplotdev(vopl_device.window[win].plotdev);

   return EXIT_SUCCESS;
}

/*----------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------*/

int vopl_winset(int win)
{
   if ( winset(win) > -1 )
   {
      vopl_device.curwin         = win;

      /* and set the default plotdev to this one (or the first of the array of plotdev owned by this window) */
      _voplsetcurrentplotdev(vopl_device.window[win].plotdev);

      return EXIT_SUCCESS;
   }
   else
   {
      return EXIT_FAILURE;
   }

   return EXIT_SUCCESS;
}

/*----------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------*/

int vopl_winclose(int win)
{
   return winclose(win);
}

/*----------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------*/

int vopl_winraise(int win)
{
   return winraise(win);
}

/*----------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------*/

int vopl_windel(int win)
{
   if (vopl_device.window[win].id != -1)
   {
      free(vopl_device.window[win].plotdev);
      vopl_device.window[win].plotdev = NULL;

      /*
       * Clear it, in case it get's reused later.
       */
      memset(&vopl_device.window[win], 0, sizeof(vopl_device.window));
      vopl_device.window[win].id = -1;
      
      if (vopl_device.curwin == win)
      {
         _voplsetcurrentplotdev(vopl_device.window[vopl_device.curwin].plotdev);
      }
   }

   return windel(win);
}

/*----------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------*/

int vopl_vgetwin(void)
{
   return vopl_device.curwin;
}

/*----------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------*/

int vopl_vwinidvalid(int win)
{
   if (win >= 0 && win < vopl_device.nwins)
   {
      if (vopl_device.window[win].id != -1)
         return(1);
   }

   return(0);
}

/*----------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------*/

void subp(int nx, int ny)
{
   int k;
   
   if ( vopl_device.curwin > -1 )
   {
      /**/
      vopl_device.window[vopl_device.curwin].nx = nx;
      vopl_device.window[vopl_device.curwin].ny = ny;
      
      /* allocate as many sub-plot(plotdev) as needed */
      free(vopl_device.window[vopl_device.curwin].plotdev);

      vopl_device.window[vopl_device.curwin].plotdev = (vopldev*)malloc( (nx*ny)*sizeof(vopldev) );
      /* init them too */
      for (k=0; k<nx*ny; k++)
      {
         vopldev_init(&(vopl_device.window[vopl_device.curwin].plotdev[k]));
      }

      /* and set the default plotdev to the first sub-plot */
      _voplsetcurrentplotdev(&vopl_device.window[vopl_device.curwin].plotdev[0] );
   }
}

/*----------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------*/

void  panel(int ix, int iy)
{
   double view_x_ini ;
   double view_x_end ;

   double view_y_ini ;
   double view_y_end ;

   int k;

   int nx = vopl_device.window[vopl_device.curwin].nx;
   int ny = vopl_device.window[vopl_device.curwin].ny;

   /* check */
   if ( !((0<ix)&&(ix<=nx)) ) vopl_error(VOPL_ERR_BAD_SUBP, "panel");
   if ( !((0<iy)&&(iy<=ny)) ) vopl_error(VOPL_ERR_BAD_SUBP, "panel");

   k = (ix-1)*ny + (iy-1);
   /* "activate" the right plotdev */
   _voplsetcurrentplotdev(&vopl_device.window[vopl_device.curwin].plotdev[k] );

   /* adapt viewport to this subplot */
   view_x_ini = -1.0 - 2.0/ny + 2.0*(iy+0)/ny;
   view_x_end = -1.0 - 2.0/ny + 2.0*(iy+1)/ny;

   view_y_ini =  1.0 - 2.0*(ix-0)/nx;
   view_y_end =  1.0 - 2.0*(ix-1)/nx;

   viewport(view_x_ini,view_x_end, view_y_ini,view_y_end);
}

/*----------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------*/

void  clearpanel(int color1)
{
   pushattributes();

      /* fill the area in black */
      polyfill(1);
      color(color1);
      rect(-1.0,-1.0, 1.0,1.0); /* paint the area now */

   popattributes();
}

/*----------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------*/

int  nxsubp()
{
   return vopl_device.window[vopl_device.curwin].nx;
}

int  nysubp()
{
   return vopl_device.window[vopl_device.curwin].ny;
}

/*----------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------*/

void plotmarker(double posx, double posy, char* symbol)
{
   double tx,ty;
   double left, right, bottom, top;

   
   /* coordonates in world positions -> set them in viewport pos. */
   pushmatrix();
   pushviewport();
   pushattributes();

      font("markers");

      getviewport(&left, &right, &bottom, &top);

      viewport(left   + (1.0 + XMIN) / 2.0 * (right - left),
               right  - (1.0 - XMAX) / 2.0 * (right - left),
               bottom + (1.0 + YMIN) / 2.0 * (top - bottom),
               top    - (1.0 - YMAX) / 2.0 * (top - bottom));

      ortho2(WhatX(plotdev->axes[XIND].min),
             WhatX(plotdev->axes[XIND].max),
             WhatY(plotdev->axes[YIND].min),
             WhatY(plotdev->axes[YIND].max));

      centertext(1);
      /*clipping(0);*/
      tx = plotdev->markerscale * TEXTWIDTH  * WhatX((plotdev->axes[XIND].max - plotdev->axes[XIND].min));
      ty = plotdev->markerscale * TEXTHEIGHT * WhatY((plotdev->axes[YIND].max - plotdev->axes[YIND].min));

      textsize(1.5*tx, 1.5*ty);

      linestyle("");

      move2(posx, posy + 0.15 * ty);
      drawstr(symbol);
      
   popmatrix();
   popviewport();
   popattributes();
}

/*----------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------*/

static void vopldev_axis_init(axisdata *axis)
{
   axis->min =  FLT_MAX;
   axis->max = -FLT_MAX;
   axis->div = 1.0;
   
   axis-> scaling   = LINEAR;
   axis->annotate   = 1;
   axis->nticks     = 5;
   axis->ntspacing  = 1;
   axis->minorticks = 10;
   axis->scaleset   = 0;

   axis->format     = NULL;
   axis->title      = NULL;
}

/*----------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------*/

static void vopldev_init(vopldev *dev)
{
   dev->s1 = 0.0;
   dev->sn = 0.0;
   
   dev->markerscale = 0.75;

   dev->fit           = STRAIGHT_LINE;
   dev->degree        = 2;
   dev->splinetype    = 0;
   dev->gridX         = 0;
   dev->gridY         = 0;
   dev->box           = 0;
   dev->startind      = 0;
   dev->arrayind      = 1;
   dev->precision     = 256;
   dev->markerspacing = 1;
   dev->forceticks    = 0;

   dev->marker        = NULL;
   dev->graphtitle    = NULL;

   vopldev_axis_init(&dev->axes[XIND]);
   vopldev_axis_init(&dev->axes[YIND]);
   vopldev_axis_init(&dev->axes[ZIND]);
}

/*----------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------*/

static vopldev* vopldev_get()
{
   vopldev *dev = (vopldev*)malloc( sizeof(vopldev) );
   vopldev_init(dev);

   return dev;
}

/*----------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------*/

static void _voplsetcurrentplotdev(vopldev *plotdev_)
{
   /* "plotdev" is the static gloabal plotdev */
   plotdev = plotdev_;
}

/*----------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------*/

/*
 * Finds a free plotdev slot (or allocates a new one).
 */
static int _voplgetfreeplotdevslot(void)
{
   int	i, ind = -1;
   /*
    * Find a free plotdev entry.
    */
   if (vopl_device.window == NULL)
   {
      /*
       * allocate here first time for those implementations
       * that don't do the right thing with realloc(0, ...)
       * Why 4?, I dunno, seemed like a good idea at the time...
       * and it is *one more* than 3.
       */
      vopl_device.window = (voplWindow *)vallocate(4 * sizeof(voplWindow));
      memset(vopl_device.window, 0, 4 * sizeof(voplWindow));
      vopl_device.nwins = 4;

      for (i = 0; i < vopl_device.nwins; i++)
      {
         vopl_device.window[i].id = -1;
      }
      ind = 0;
   }
   else
   {
      for (i = 0; i < vopl_device.nwins; i++)
      {
         if (vopl_device.window[i].id == -1)
         {
            ind = i;
            break;
         }
      }
   }

   if (ind < 0)
   {
      /*
       * Get 4 more... it's *one more* than 3!
       */
      vopl_device.window = (voplWindow *)realloc( vopl_device.window, (vopl_device.nwins +4)*sizeof(voplWindow) );

      if (!vopl_device.window)
      {
         verror(VERR_MALLOC, "_voplgetfreeplotdevslot");
      }
      
      for (i = vopl_device.nwins; i < vopl_device.nwins + 4; i++)
      {
         memset(&vopl_device.window[i], 0, sizeof(voplWindow));
         vopl_device.window[i].id = -1;
      }

      ind = vopl_device.nwins;
      vopl_device.nwins += 4;
   }

   return(ind);
}

/*----------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------*/

