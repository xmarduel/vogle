

#include "vogle.h"


static int	nsegs = 32;

/*
 * arcprecision
 *
 *	sets the number of segments in an arc or circle.
 *	- obsolete function.
 */
void
arcprecision(int noseg)
{
   nsegs = noseg;
}

/*
 * circleprecision
 *
 *	sets the number of segments in an arc or circle.
 */
void
circleprecision(int noseg)
{
   nsegs = noseg;
}

/*
 * arc
 *
 * draw an arc at a given location.  Precision of arc (# line segments)
 * is calculated from the value given to circleprecision.
 *
 */
void
arc(float x, float y, float radius, float startang, float endang)
{
   Token	*t;
   float	cx, cy, dx, dy;
   float	deltang, cosine, sine, angle;
   int	sync, i, numsegs;

   if (!vdevice.initialised)
      verror(VERR_UNINIT, "arc");

   if ((sync = vdevice.sync))
      vdevice.sync = 0;

   angle = startang * D2R;
   numsegs = fabs(endang - startang) / 360.0 * nsegs + 0.5;
   deltang = (endang - startang) * D2R / numsegs;
   cosine = cos((double)deltang);
   sine = sin((double)deltang);

   if (vdevice.inobject)
   {
      t = newtokens(8);
      t[0].i = ARC;
      t[1].f = x;
      t[2].f = y;
      t[3].f = radius * cos((double)angle);
      t[4].f = radius * sin((double)angle);
      t[5].f = cosine;
      t[6].f = sine;
      t[7].i = numsegs;
      return;
   }

   /* calculates initial point on arc */

   cx = x + radius * cos((double)angle);
   cy = y + radius * sin((double)angle);
   move2(cx, cy);

   for (i = 0; i < numsegs; i++)
   {
      dx = cx - x;
      dy = cy - y;
      cx = x + dx * cosine - dy * sine;
      cy = y + dx * sine + dy * cosine;
      draw2(cx, cy);
   }

   if (sync)
   {
      vdevice.sync = 1;
      (*vdevice.dev.Vsync)();
   }
}

/*
 * sector
 *
 *	draw a sector in a given location. The number of line segments in the
 * arc of the segment is the same as in arc.
 */
void
sector(float x, float y, float radius, float startang, float endang)
{
   Token	*t;
   float	cx, cy, dx, dy;
   float	deltang, cosine, sine, angle;
   int	sync, i, numsegs, inpoly;

   if (!vdevice.initialised)
      verror(VERR_UNINIT, "segment");

   angle = startang * D2R;
   numsegs = fabs(endang - startang) / 360.0 * nsegs + 0.5;
   deltang = (endang - startang) * D2R / numsegs;
   cosine = cos((double)deltang);
   sine = sin((double)deltang);

   if (vdevice.inobject)
   {
      t = newtokens(8);
      t[0].i = SECTOR;
      t[1].f = x;
      t[2].f = y;
      t[3].f = radius * cos((double)angle);
      t[4].f = radius * sin((double)angle);
      t[5].f = cosine;
      t[6].f = sine;
      t[7].i = numsegs;
      return;
   }

   if ((sync = vdevice.sync))
      vdevice.sync = 0;

   inpoly = vdevice.inpolygon;

   if ((vdevice.attr->a.fill || vdevice.attr->a.hatch) && !inpoly)
      makepoly();		/* want it filled */

   move2(x, y);
			/* calculates initial point on arc */

   cx = x + radius * cos((double)angle);
   cy = y + radius * sin((double)angle);

   draw2(cx, cy);

   for (i = 0; i < numsegs; i++)
   {
      dx = cx - x;
      dy = cy - y;
      cx = x + dx * cosine - dy * sine;
      cy = y + dx * sine + dy * cosine;
      draw2(cx, cy);
   }

   if ((vdevice.attr->a.fill || vdevice.attr->a.hatch) && !inpoly)
   {
      closepoly();
   }
   else
   {
      if (sync)
         vdevice.sync = 1;

      draw2(x, y);
   }
}

/*
 * circle
 *
 * Draw a circle of given radius at given world coordinates. The number of
 * segments in the circle is the same as that of an arc.
 *
 */
void
circle(float x, float y, float radius)
{
   Token	*t;
   float	cx, cy, dx, dy;
   float	angle, cosine, sine;
   int	sync, i, inpoly;

   if (!vdevice.initialised)
      verror(VERR_UNINIT, "circle");

   angle = 2.0 * PI / nsegs;
   cosine = cos((double)angle);
   sine = sin((double)angle);

   if (vdevice.inobject)
   {
      t = newtokens(7);
      t[0].i = CIRCLE;
      t[1].f = x;
      t[2].f = y;
      t[3].f = radius;
      t[4].f = cosine;
      t[5].f = sine;
      t[6].i = nsegs;
      return;
   }

   if ((sync = vdevice.sync))
      vdevice.sync = 0;

   cx = x + radius;
   cy = y;

   inpoly = vdevice.inpolygon;

   if ((vdevice.attr->a.fill || vdevice.attr->a.hatch) && !inpoly)
      makepoly();		/* want it filled */

   move2(cx, cy);
   for (i = 0; i < nsegs - 1; i++)
   {
      dx = cx - x;
      dy = cy - y;
      cx = x + dx * cosine - dy * sine;
      cy = y + dx * sine + dy * cosine;
      draw2(cx, cy);
   }

   if ((vdevice.attr->a.fill || vdevice.attr->a.hatch) && !inpoly)
   {
      closepoly();
   }
   else
   {
      if (sync)
         vdevice.sync = 1;

      draw2(x + radius, y);
   }
}
