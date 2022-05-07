#include "vogle.h"

/*
 * point
 *
 * plot a point in x, y, z.
 *
 */
void
point(float x, float y, float z)
{
   move(x, y, z);
   draw(x, y, z);
}

/*
 * point
 *
 * plot a point in x, y.
 *
 */
void
point2(float x, float y)
{
   move(x, y, 0.0);
   draw(x, y, 0.0);
}

/*
 * spoint
 *
 * plot a point in screen coords.
 *
 */
void
spoint2(float xs, float ys)
{
   if (!vdevice.initialised)
   {
      verror(VERR_UNINIT, "spoint2");
   }
   
   (*vdevice.dev.Vdraw)(vdevice.cpVx = (xs + 0.5) * (vdevice.maxVx - vdevice.minVx),
                        vdevice.cpVy = (ys + 0.5) * (vdevice.maxVy - vdevice.minVy));
}
