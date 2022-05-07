#include "vogle.h"

/*
 * rect
 *
 * draw a rectangle given two opposite corners
 *
 */
void
rect(float x1, float y1, float x2, float y2)
{
   int	sync, flag = 0;

   if (!vdevice.initialised)
      verror(VERR_UNINIT, "rect");

   if ((vdevice.attr->a.fill || vdevice.attr->a.hatch) && !vdevice.inpolygon)
   {
      flag = 1;
      makepoly();		/* want it filled */
   }

   if ((sync = vdevice.sync))
      vdevice.sync = 0;

   move2(x1, y1);
   draw2(x2, y1);
   draw2(x2, y2);
   draw2(x1, y2);

   if (flag)
   {
      closepoly();
   }
   else
   {
      draw2(x1, y1);
   }
   
   if (sync)
   {
      vdevice.sync = 1;
      (*vdevice.dev.Vsync)();
   }
}

/*
 * srect
 *
 * draw a rectangle given two opposite corners in screen coords.
 *
 */
void
srect(float x1, float y1, float x2, float y2)
{
   int	sync;

   if (!vdevice.initialised)
      verror(VERR_UNINIT, "rect");

   if ((sync = vdevice.sync))
      vdevice.sync = 0;

   smove2(x1, y1);
   sdraw2(x2, y1);
   sdraw2(x2, y2);
   sdraw2(x1, y2);

   sdraw2(x1, y1);

   if (sync)
   {
      vdevice.sync = 1;
      (*vdevice.dev.Vsync)();
   }
}
