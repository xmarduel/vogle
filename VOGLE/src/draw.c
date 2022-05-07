#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>


#include "vogle.h"

static	int	do_dash = 1;

/* a == b or b > a */
#define NEAR(a, b)	(((a) - (b)) / (a) < 0.0001)
#define checkdash(l)	((*(++l->dashp) == '\0' ? *(l->dashp = l->style) : *l->dashp) != '0')

/*
 * Set the current dash length
 */
void
setdash(float d)
{
   Token	*tok;

   if (vdevice.inobject)
   {
      tok = newtokens(2);

      tok[0].i = SETDASH;
      tok[1].f = d;

      return;
   }

   if (d < 0.0)
      d = -d;

   if (d == 0.0)
      d = 0.1;


   vdevice.attr->a.dash = d;
}

/*
 * Set the current linestyle as a character string of 1's and 0's
 */
void
linestyle(const char *l)
{
   Attribute *line;
   char	*c;
   Token	*tok;

   line = &vdevice.attr->a;

   if (vdevice.inobject)
   {
      if (l && *l)
      {
         tok = newtokens(2 + strlen(l) / sizeof(Token));

         tok[0].i = LINESTYLE;
         strcpy((char *)&tok[1], l);
      }
      else
      {
         tok = newtokens(2);
         tok[0].i = LINESTYLE;
         tok[1].i = 0;
      }

      return;
   }


   if (!l || !*l)
   {
      line->style = NULL;
   }
   else
   {
      /*
       * If it's all non-0 (or ' ') then it's a solid line
       */
      for (c = (char *)l; *c != '\0'; c++)
      {
         if (*c == '0' || *c == ' ')
         {
            if (line->style)
            {
               free(line->style);	/* probably could realloc */
            }
            
            line->style = (char *)vallocate(strlen(l) + 1);
            strcpy(line->style, l);
            line->dashp = line->style;
            line->adist = 0.0;
            do_dash = *line->dashp != '0';

            return;
         }
      }
      line->style = NULL;
   }

   return;
}

/*
 * Draw dashed lines (Duh)
 * Assumes p0 - p1 are valid (possibly clipped endpoints)
 */
void
dashline(register Vector p0, register Vector p1)
{
   int	vx, vy, sync;
   float	dx, dy, dz, dw, dist, ldist, tdist;
   Vector	pd;
   Attribute *line;


   line = &vdevice.attr->a;
   if (line->dash == 0.0)
   {
      vx = WtoVx(p1);
      vy = WtoVy(p1);

      (*vdevice.dev.Vdraw)(vx, vy);
      return;
   }

   /*
    * The distance for this line segment
    */
   dx = p1[V_X] - p0[V_X];
   dy = p1[V_Y] - p0[V_Y];
   dz = p1[V_Z] - p0[V_Z];
   dw = p1[V_W] - p0[V_W];
   ldist = sqrt(dx*dx + dy*dy + dz*dz + dw*dw);

   /*
      * If this distance is less than it takes to
    * complete the current dash then just do it.
    */
   if (ldist <= (line->dash - line->adist))
   {

      if (NEAR(line->dash, line->adist))
      {
         line->adist = 0.0;
         do_dash = checkdash(line);
      }

      line->adist += ldist;

      vx = WtoVx(p1);
      vy = WtoVy(p1);

      if (do_dash)
         (*vdevice.dev.Vdraw)(vx, vy);

      vdevice.cpVx = vx;
      vdevice.cpVy = vy;

      return;

   }
   else
   {
      if ((sync = vdevice.sync))                /* We'll sync at the end */
         vdevice.sync = 0;

      /*
      * If this distance will take us over the end of a
       * dash then break it up.
       */

      /*
       * Handle the initial case where we start in the middle
       * of a dash.
       */

      tdist = 0.0;
      copyvector(pd, p0);

      if (line->adist > 0.0)
      {

         tdist = (line->dash - line->adist);

         if (NEAR(line->dash, line->adist))
         {
            line->adist = 0.0;
            do_dash = checkdash(line);
         }

         line->adist += tdist;

         dist = tdist / ldist;
         pd[V_X] += dx * dist;
         pd[V_Y] += dy * dist;
         pd[V_Z] += dz * dist;
         pd[V_W] += dw * dist;
         vx = WtoVx(pd);
         vy = WtoVy(pd);


         if (do_dash)
            (*vdevice.dev.Vdraw)(vx, vy);

         vdevice.cpVx = vx;
         vdevice.cpVy = vy;
      }

      dx *= line->dash / ldist;
      dy *= line->dash / ldist;
      dz *= line->dash / ldist;
      dw *= line->dash / ldist;
      dist = line->dash;

      while (tdist <= ldist - dist)
      {

         if (NEAR(line->dash, line->adist))
         {
            line->adist = 0.0;
            do_dash = checkdash(line);
         }

         pd[V_X] += dx;
         pd[V_Y] += dy;
         pd[V_Z] += dz;
         pd[V_W] += dw;

         vx = WtoVx(pd);
         vy = WtoVy(pd);

         line->adist += dist;
         tdist += dist;

         if (do_dash)
            (*vdevice.dev.Vdraw)(vx, vy);

         vdevice.cpVx = vx;
         vdevice.cpVy = vy;
      }


      /*
       * Check the last little bit....
       */
      if (NEAR(line->dash, line->adist))
      {
         line->adist = 0.0;
         do_dash = checkdash(line);
      }

      dx = p1[V_X] - pd[V_X];
      dy = p1[V_Y] - pd[V_Y];
      dz = p1[V_Z] - pd[V_Z];
      dw = p1[V_W] - pd[V_W];
      dist = sqrt(dx*dx + dy*dy + dz*dz + dw*dw);

      line->adist += dist;

      vx = WtoVx(p1);
      vy = WtoVy(p1);

      if (do_dash)
         (*vdevice.dev.Vdraw)(vx, vy);

      vdevice.cpVx = vx;
      vdevice.cpVy = vy;
   }

   if (sync)
   {
      vdevice.sync = 1;
      (*vdevice.dev.Vsync)();
   }

}

/*
 * draw
 *
 * draw a line form the logical graphics position to the
 * the world coordinates x, y, z.
 *
 */
void
draw(float x, float y, float z)
{
   Token	*tok;
   int	vx, vy;
   Vector	res;

   /*
    if (!vdevice.initialised)
    verror(VERR_UNINIT, "draw");
    */

   if (vdevice.inpolygon)
   {
      (*vdevice.pdraw)(x, y, z);

      vdevice.pos->cpW[V_X] = x;
      vdevice.pos->cpW[V_Y] = y;
      vdevice.pos->cpW[V_Z] = z;

      vdevice.cpVvalid = 0;

      return;
   }

   if (vdevice.inobject)
   {
      tok = newtokens(4);

      tok[0].i = DRAW;
      tok[1].f = x;
      tok[2].f = y;
      tok[3].f = z;

      vdevice.pos->cpW[V_X] = x;
      vdevice.pos->cpW[V_Y] = y;
      vdevice.pos->cpW[V_Z] = z;

      vdevice.cpVvalid = 0;

      return;
   }

   if (!vdevice.cpVvalid)
      multvector(vdevice.pos->cpWtrans, vdevice.pos->cpW, vdevice.transmat->m);

   vdevice.pos->cpW[V_X] = x;
   vdevice.pos->cpW[V_Y] = y;
   vdevice.pos->cpW[V_Z] = z;
   multvector(res, vdevice.pos->cpW, vdevice.transmat->m);

   if (vdevice.clipoff)
   {
      vx = WtoVx(res);		/* just draw it */
      vy = WtoVy(res);

      if (vdevice.attr->a.style)
      {
         dashline(vdevice.pos->cpWtrans, res);
         vdevice.cpVvalid = 0;
         return;
      }

      (*vdevice.dev.Vdraw)(vx, vy);

      vdevice.cpVx = vx;
      vdevice.cpVy = vy;

      vdevice.cpVvalid = 0;
   }
   else
   {
      if (vdevice.cpVvalid)
         quickclip(vdevice.pos->cpWtrans, res);
      else
         clip(vdevice.pos->cpWtrans, res);
   }

   vdevice.pos->cpWtrans[V_X] = res[V_X];
   vdevice.pos->cpWtrans[V_Y] = res[V_Y];
   vdevice.pos->cpWtrans[V_Z] = res[V_Z];
   vdevice.pos->cpWtrans[V_W] = res[V_W];
}


/*
 * draw2
 *
 * draw a line from the logical graphics position  to the
 * the world coordinates x, y.
 *
 */
void
draw2(float x, float y)
{
   draw(x, y, 0.0);
}

/*
 * rdraw
 *
 * 3D relative draw from the logical graphics position by dx, dy, dz.
 *
 */
void
rdraw(float dx, float dy, float dz)
{
   draw((vdevice.pos->cpW[V_X] + dx), (vdevice.pos->cpW[V_Y] + dy), (vdevice.pos->cpW[V_Z] + dz));
}

/*
 * rdraw2
 *
 * 2D relative draw from the logical graphics position by dx, dy.
 *
 */
void
rdraw2(float dx, float dy)
{
   draw((vdevice.pos->cpW[V_X] + dx), (vdevice.pos->cpW[V_Y] + dy), 0.0);
}

/*
 * sdraw2
 *
 * Draw directly in proportion to screen coordinates.
 */
void
sdraw2(float xs, float ys)
{
   int	nx, ny;

   if (!vdevice.initialised)
      verror(VERR_UNINIT, "sdraw2");

   nx = (xs / 2 + 0.5) * vdevice.dev.sizeX;
   ny = (0.5 + ys / 2) * vdevice.dev.sizeY;

   (*vdevice.dev.Vdraw)(nx, ny);

   vdevice.cpVx = nx;
   vdevice.cpVy = ny;
   vdevice.cpVvalid = 0;
}

/*
 * rsdraw2
 *
 * Relative draw as a fraction of screen size.
 */
void
rsdraw2(float dxs, float dys)
{
   int	ndx, ndy;

   if (!vdevice.initialised)
      verror(VERR_UNINIT, "rsdraw2");

   ndx = dxs * vdevice.dev.sizeSx / 2;
   ndy = dys * vdevice.dev.sizeSy / 2;

   (*vdevice.dev.Vdraw)(vdevice.cpVx + ndx, vdevice.cpVy + ndy);

   vdevice.cpVx += ndx;
   vdevice.cpVy += ndy;
}

