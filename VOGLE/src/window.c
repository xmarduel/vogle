#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "vogle.h"

#define MIN(x,y)        ((x) < (y) ? (x) : (y))

/*
 * This is so we can access the one and only thing about a driver
 * that isn't private to it.
 */
typedef struct
{
   unsigned long localid;
   /* Other stuff */
} Generic;

static DevEntry *nowindow = NULL;

/*
 * This just returns an error.
 */
static int
errorentry(void)
{
   return(verror(VERR_INVALIDW, "(deleted window)"));
}

/*
 * Just makes a structure that points to the errorentry function.
 */
void
makenowindowstruct(void)
{
   /*
    * Isn't ANSI C just great! Just look at this beautiful code!
    */
   nowindow = (DevEntry *)vallocate(sizeof(DevEntry));
   memset(nowindow, 0, sizeof(DevEntry));

   nowindow->Vbackb	= (int (*)(void *, int, int, int))errorentry;
   nowindow->Vchar	= (int (*)(char))errorentry;
   nowindow->Vcheckkey	= (int (*)(void))errorentry;
   nowindow->Vclear	= (int (*)(void))errorentry;
   nowindow->Vcolor	= (int (*)(int))errorentry;
   nowindow->Vdraw	= (int (*)(int, int))errorentry;
   nowindow->Vfill	= (int (*)(int, int[], int[]))errorentry;
   nowindow->Vdraw 	= (int (*)(int, int))errorentry;
   nowindow->Vfrontb	= (int (*)(void))errorentry;
   nowindow->Vgetkey	= (int (*)(void))errorentry;
   nowindow->Vgetevent	= (unsigned long (*)(Vevent *, int))errorentry;
   nowindow->Vwinopen	= (void * (*)(const char *, const char *, int))errorentry;
   nowindow->Vwinset	= (int (*)(void *))errorentry;
   nowindow->Vwinclose	= (int (*)(void *))errorentry;
   nowindow->Vwinraise	= (int (*)(void *))errorentry;
   nowindow->Vwindel	= (int (*)(void *))errorentry;
   nowindow->Vlocator	= (int (*)(int *, int *))errorentry;
   nowindow->Vmapcolor	= (int (*)(int, int, int, int))errorentry;
   nowindow->Vpnt	= (int (*)(int, int))errorentry;
   nowindow->Vsetlw	= (int (*)(int))errorentry;
   nowindow->Vstring	= (int (*)(const char *))errorentry;
   nowindow->Vswapb	= (int (*)(void))errorentry;
   nowindow->Vsync	= (void (*)(void))errorentry;
}


/*
 * Finds a free window slot (or allocates a new one).
 */
int
_vgetfreewindowslot(void)
{
   int	i, ind = -1;
   /*
    * Find a free window entry.
    */
   if (vdevice.wins == NULL)
   {
      /*
       * allocate here first time for those implementations
       * that don't do the right thing with realloc(0, ...)
       * Why 4?, I dunno, seemed like a good idea at the time...
       * and it is *one more* than 3.
       */
      vdevice.wins = (Vwindow *)vallocate(4 * sizeof(Vwindow));
      memset(vdevice.wins, 0, 4 * sizeof(Vwindow));
      vdevice.nwin = 4;
      for (i = 0; i < vdevice.nwin; i++)
         vdevice.wins[i].id = -1;

      ind = 0;
   }
   else
   {
      for (i = 0; i < vdevice.nwin; i++)
      {
         if (vdevice.wins[i].id == -1)
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
      vdevice.wins = (Vwindow *)realloc(vdevice.wins, (vdevice.nwin + 4)*sizeof(Vwindow));
      if (!vdevice.wins)
         verror(VERR_MALLOC, "winopen");

      for (i = vdevice.nwin; i < vdevice.nwin + 4; i++)
      {
         memset(&vdevice.wins[i], 0, sizeof(Vwindow));
         vdevice.wins[i].id = -1;
      }

      ind = vdevice.nwin;
      vdevice.nwin += 4;
   }

   return(ind);
}

/*
 * Open a window on the specified device. If it can make use of
 * a title, let it do so.
 */
int
winopen(const char *dev, const char *title)
{
   void	*devwin;
   int	ind;

   ind = _vgetfreewindowslot();

   if (_vgetvogledev(dev) > 0)
   {
      Generic	*generic;
      if ((devwin = (*vdevice.dev.Vwinopen)(dev, title, ind)) != NULL)
      {
         generic = (Generic *)devwin;
         vdevice.wins[ind].id = ind;
         vdevice.wins[ind].localid = generic->localid;
         vdevice.wins[ind].devwin = devwin;
         vdevice.wins[ind].dev = vdevice.dev;
         if (!vdevice.initialised)
            _vdovogleinit();

         vdevice.curwin = ind;

         calcviewport();

         return(ind);
      }
   }

   return(verror(VERR_BADDEV, "winopen"));
}

/*
 * Deprecated function vinit.
 */
int
vinit(const char *dev)
{
   return(winopen(dev, ""));
}

/*
 * Sets drawing into a particular window.
 * Returns the window in use before this call (or an error).
 */
int
winset(int win)
{
   int	cwin = vdevice.curwin;

   if (win < 0 || win >= vdevice.nwin)
      return(verror(VERR_BADWIN, "winset"));

   if (vdevice.wins[win].id != -1)
   {
      vdevice.dev = vdevice.wins[win].dev;
      (*vdevice.dev.Vwinset)(vdevice.wins[win].devwin);
      vdevice.curwin = win;
      calcviewport();
      return(cwin);
   }
   else
   {
      return(verror(VERR_BADWIN, "winset"));
   }
}

/*
 * Close a window (ie make it an icon).
 */
int
winclose(int win)
{
   DevEntry	*ent = &vdevice.wins[win].dev;

   if (win < 0 || win >= vdevice.nwin)
      return(verror(VERR_BADWIN, "winclose"));

   if (vdevice.wins[win].id != -1)
   {
      (*ent->Vwinclose)(vdevice.wins[win].devwin);
      return(1);
   }

   return(verror(VERR_BADWIN, "winclose"));
}

/*
 * Raise a window from it's "closed" state.
 */
int
winraise(int win)
{
   DevEntry	*ent = &vdevice.wins[win].dev;

   if (win < 0 || win >= vdevice.nwin)
      return(verror(VERR_BADWIN, "winraise"));

   if (vdevice.wins[win].id != -1)
   {
      (*ent->Vwinraise)(vdevice.wins[win].devwin);
      return(1);
   }

   return(verror(VERR_BADWIN, "winraise"));
}

/*
 * Completely delete all references to a window.
 */
int
windel(int win)
{
   DevEntry	*ent = &vdevice.wins[win].dev;

   if (win < 0 || win >= vdevice.nwin)
      return(verror(VERR_BADWIN, "windel"));

   if (vdevice.wins[win].id != -1)
   {
      (*ent->Vwindel)(vdevice.wins[win].devwin);
      if (!nowindow)
         makenowindowstruct();

      /*
       * Clear it, in case it get's reused later.
       */
      memset(&vdevice.wins[win], 0, sizeof(Vwindow));
      vdevice.wins[win].id = -1;

      vdevice.wins[win].dev = *nowindow;
      if (vdevice.curwin == win)
         vdevice.dev = *nowindow;

      return(1);
   }

   return(verror(VERR_BADWIN, "windel"));
}

#if 0

/* WILL WE KEEP THIS? */
/*
 * vnewdev
 *
 * reinitialize vogle to use a new device but don't change any
 * global attributes like the window and viewport settings.
 */
void
vnewdev(const char *device)
{
   if (!vdevice.initialised)
      verror(VERR_UNINIT, "vnewdev");

   pushviewport();

   winopen(device, "");

   /*
    * Need to update font for this device if hardware font is what was
    * being used previously.
    */

   if (!strcmp(vdevice.attr->a.font, "small"))
   {
      if (!(*vdevice.dev.Vfont)(vdevice.dev.small))
         verror(VERR_FILEIO, "font(small)");
   }
   else if (!strcmp(vdevice.attr->a.font, "large"))
   {
      if (!(*vdevice.dev.Vfont)(vdevice.dev.large))
         verror(VERR_FILEIO, "font(large)");
   }

   popviewport();
}
#endif

/*
 * vgetdev
 *
 *	Returns the name of the current vogle device
 *	in the buffer buf. Also returns a pointer to
 *	the start of buf.
 */
char	*
vgetdev(char *buf)
{
   /*
    * Note no exit if not initialized here - so that vexit
    * can be called before printing the name.
    */
   if (vdevice.dev.devname)
      strcpy(buf, vdevice.dev.devname);
   else
      strcpy(buf, "(no device)");

   return(&buf[0]);
}

/*
 * vgetevent
 *
 *	This is a crummy first hack at putting some kind of extended
 * event handling into vogle. This is mainly for X11, so we can handle
 * things like window size changes etc etc.
 */
int
vgetevent(Vevent *vev, int block)
{
   int	i;
   unsigned long	devwinid = (*vdevice.dev.Vgetevent)(vev, block);

   if (devwinid == 0L)
      return(-1);	/* No events */

   for (i = 0; i < vdevice.nwin; i++)
   {
      if (vdevice.wins[i].id != -1 && vdevice.wins[i].localid == devwinid)
      {
         vev->w = i;
         if (vev->type == VRESIZE)
         {
            /*
             * Ignore all this guf if we are already this
             * size!
             */
            if (vdevice.wins[i].dev.sizeSx != vev->x || vdevice.wins[i].dev.sizeSy != vev->y)
            {
               vdevice.wins[i].dev.sizeSx = vev->x;
               vdevice.wins[i].dev.sizeSy = vev->y;
               vdevice.wins[i].dev.sizeX = vdevice.wins[i].dev.sizeY = MIN(vev->x, vev->y);
               if (vdevice.curwin == i)
               {
                  vdevice.dev.sizeSx = vev->x;
                  vdevice.dev.sizeSy = vev->y;
                  vdevice.dev.sizeX = vdevice.dev.sizeY = MIN(vev->x, vev->y);
               }

               /*
                * Need to change size of backbuffer as well
                */
               if (vdevice.wins[i].havebackb)
               {
                  /*printf("Resizing: %d 0x%x\n", i, vdevice.wins[i].devwin);*/
                  (*vdevice.wins[i].dev.Vbackb)(vdevice.wins[i].devwin, 1, vev->x, vev->y);
               }
            }
         }
         return(vev->w);
      }
   }
   return(-1);
}

/*
 * Returns the vogle window id of the current window.
 */
int
vgetwin(void)
{
   return(vdevice.curwin);
}

/*
 * Checks if the window id number is valid.
 */
int
vwinidvalid(int id)
{
   if (id >= 0 && id < vdevice.nwin)
   {
      if (vdevice.wins[id].id != -1)
         return(1);
   }

   return(0);
}
