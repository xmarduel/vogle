#include <math.h>
#include "vogle.h"


#define MAX(x, y)	((x) > (y) ? (x) : (y))
#define MIN(x, y)	((x) < (y) ? (x) : (y))
#define ABS(x)		((x) < 0 ? -(x) : (x))

static float	F[6][4], S[6][4], I[4], p[MAXVERTS][4], newp[MAXVERTS][3];
static int	nout, first[6], np;
static int	ip1[MAXVERTS], ip2[MAXVERTS];

static int	yintersect(float y, float x1, float y1, float x2, float y2, float *xint);
static int	intersect(int side, Vector I, Vector p);
static int	checkbacki(void);
static int	visible(int side);
static void	polyclip(register int n);
static void	shclip(float p[4], int side);
static void	shclose(int side);
static void	hatch(int n, float p[][3]);

/*
 *  Orientation of backfacing polygons(in screen coords)
 */
static	int	clockwise = 1;

static	void	polyoutline(int n, int ipx[], int ipy[]);

/*
 * polyfill
 *
 *	set the polygon fill flag. This will always turn off hatching.
 */
void
polyfill(int onoff)
{
   Token	*tok;

   if (vdevice.inobject)
   {
      tok = newtokens(2);
      tok[0].i = POLYFILL;
      tok[1].i = onoff;

      return;
   }

   vdevice.attr->a.fill = onoff;
   vdevice.attr->a.hatch = 0;
}

/*
 * polyhatch
 *
 *	set the polygon hatch flag. This will always turn off fill.
 */
void
polyhatch(int onoff)
{
   Token	*tok;

   if (vdevice.inobject)
   {
      tok = newtokens(2);
      tok[0].i = POLYHATCH;
      tok[1].i = onoff;

      return;
   }

   vdevice.attr->a.hatch = onoff;
   vdevice.attr->a.fill = 0;
}

/*
 * hatchang
 *
 *	set the hatch angle
 */
void
hatchang(float a)
{
   Token	*tok;

   if (!vdevice.initialised)
      verror(VERR_UNINIT, "hatchang");

   if (vdevice.inobject)
   {
      tok = newtokens(3);

      tok[0].i = HATCHANG;
      tok[1].f = cos((double)(a*D2R));
      tok[2].f = sin((double)(a*D2R));
      return;
   }

   vdevice.attr->a.hatchcos = cos((double)(a*D2R));
   vdevice.attr->a.hatchsin = sin((double)(a*D2R));
}

/*
 * hatchpitch
 *
 *	set the hatch pitch
 */
void
hatchpitch(float a)
{
   Token	*tok;

   if (!vdevice.initialised)
      verror(VERR_UNINIT, "hatchpitch");

   if (vdevice.inobject)
   {
      tok = newtokens(2);

      tok[0].i = HATCHPITCH;
      tok[1].f = a;
      return;
   }

   vdevice.attr->a.hatchpitch = a;
}

/*
 * hatch
 *
 *	hatch the polygon defined by the n points in the array p.
 */
static void
hatch(int n, float p[][3])
{
   float yl, xint, y1, y2, x1, x2, tmp, ymax = -1.0e30, ymin = 1.0e30;
   float	xi[MAXVERTS], xtmp[MAXVERTS], ytmp[MAXVERTS];
   float	z, s, c, pitch;
   int	sync, i, j, sorted;

   c = vdevice.attr->a.hatchcos;
   s = vdevice.attr->a.hatchsin;
   pitch = vdevice.attr->a.hatchpitch;

   if ((sync = vdevice.sync))
      vdevice.sync = 0;

   /*
    * Rotate by the x-hatch angle...
    */

   for (i = 0; i < n; i++)
   {
      xtmp[i] = p[i][0] * c - p[i][1] * s;
      ytmp[i] = p[i][0] * s + p[i][1] * c;
      ymax = MAX(ymax, ytmp[i]);
      ymin = MIN(ymin, ytmp[i]);
   }

   /*
    * For each y value, get a list of X intersections...
    */
   yl = ymax - pitch;
   while (yl > ymin)
   {
      j = 0;
      for (i = 0; i < n-1; i++)
      {
         if (yintersect(yl, xtmp[i], ytmp[i], xtmp[i+1], ytmp[i+1], &xint))
            xi[j++] = xint;
      }
      
      /*
       * Last one.
       */
      if (yintersect(yl, xtmp[n-1], ytmp[n-1], xtmp[0], ytmp[0], &xint))
         xi[j++] = xint;
      /*
       * Sort the X intersections...
       */
      sorted = 0;
      while (!sorted)
      {
         sorted = 1;
         for (i = 0; i < j-1; i++)
         {
            if (xi[i] > xi[i+1])
            {
               tmp = xi[i];
               xi[i] = xi[i+1];
               xi[i+1] = tmp;
               sorted = 0;
            }
         }
      }

      /*
       *  Draw the lines (Rotated back)...
       */
      z = p[0][2];
      for (i = 0; i < j-1; i += 2)
      {
         y1 = yl * c - xi[i] * s;
         y2 = yl * c - xi[i+1] * s;
         x1 = xi[i] * c + yl * s;
         x2 = xi[i+1] * c + yl * s;
         move(x1, y1, z);
         draw(x2, y2, z);
      }
      yl -= pitch;
   }

#if 0
   /* Draw the outline */

   move(p[0][0], p[0][1], p[0][2]);

   for (i = 1; i < n; i++)
   {
      draw(p[i][0], p[i][1], p[i][2]);
   }

   draw(p[0][0], p[0][1], p[0][2]);
#endif

   if (sync)
   {
      vdevice.sync = 1;
      (*vdevice.dev.Vsync)();
   }
}

static int
yintersect(float y, float x1, float y1, float x2, float y2, float *xint)
{
   float	t, a;

   a = y2 - y1;
   if (ABS(a) >= 0.00001)
   {
      t = (y - y1) / a;
      if (t >= 0.0 && t <= 1.0)
      {
         *xint = x1 + t*(x2 - x1);
         return (1);
      }
   }
   return (0);
}

/*
 * backfacedir
 *
 *	Set which direction backfacing polygons are defined to be.
 *	1 = clockwise (in screen coords) 0 = anticlockwise.
 */
void
backfacedir(int cdir)
{
   clockwise = cdir;
}

/*
 * backface
 *
 *	Turns on culling of backfacing polygons. A polygon is
 * backfacing if it's orientation in *screen* coords is clockwise.
 */
void
backface(int onoff)
{
   vdevice.attr->a.backface = onoff;
}

/*
 * dopoly
 *
 *	do a transformed polygon with n edges using fill or hatch
 */
static int
dopoly(int n)
{
   int	i;
   char	buf[100];

   if (n > MAXVERTS)
   {
      sprintf(buf, "dopoly: can't hatch or fill polygon");
      verror(VERR_MAXPNTS, buf);
   }

   if (!vdevice.clipoff)
   {
      polyclip(n);
      if (vdevice.pickwin.pickm)
      {
         if (nout > 0)
            vdevice.pickwin.pick = 1;
         return(0);
      }

   }
   else
   {
      nout = n;
      for (i = 0; i < n; i++)
      {
         ip1[i] = WtoVx(p[i]);
         ip2[i] = WtoVy(p[i]);
      }
   }


   if (vdevice.attr->a.backface && checkbacki())
   {
      /*return(0);*/
      
      if (vdevice.attr->a.fill)
      {
         return(0);
      }
      else
      {
         return(0);
         
         /* XAM */
         
         int oldcolor = vdevice.attr->a.color;
         
         color(GREEN);
         
         vdevice.cpVx = ip1[0];
         vdevice.cpVy = ip2[0];
         vdevice.cpVvalid = 0;
         polyoutline(nout, ip1, ip2);
         
         color(oldcolor);
         
         return 1;
      }
   }

   if (vdevice.attr->a.fill)
   {
      if (nout > 2)
      {
         (*vdevice.dev.Vfill)(nout, ip1, ip2);
      }
   }
   else
   {
      vdevice.cpVx = ip1[0];
      vdevice.cpVy = ip2[0];
      vdevice.cpVvalid = 0;
      polyoutline(nout, ip1, ip2);
   }

   return(1);
}

/*
 * polyoutline
 *
 *	draws a polygon outline from already transformed points.
 */
static void
polyoutline(int n, int ipx[], int ipy[])
{
   int	i;

   if (n > 2)
   {
      for (i = 1; i < n; i++)
      {
         (*vdevice.dev.Vdraw)(ipx[i], ipy[i]);

         vdevice.cpVx = ipx[i];
         vdevice.cpVy = ipy[i];
      }
      
      (*vdevice.dev.Vdraw)(ipx[0], ipy[0]);

      vdevice.cpVx = ipx[0];
      vdevice.cpVy = ipy[0];
   }
}

/*
 * polyobj
 *
 *	construct a polygon from a object token list.
 */
void
polyobj(int n, Token dp[])
{
   int	b, i, j;
   float	vect[4], result[4];

   for (i = 0, j = 0; i < n; i++, j += 3)
   {
      vect[V_X] = dp[j + V_X].f;
      vect[V_Y] = dp[j + V_Y].f;
      vect[V_Z] = dp[j + V_Z].f;
      vect[V_W] = 1;
      multvector(result, vect, vdevice.transmat->m);
      p[i][V_X] = result[V_X];
      p[i][V_Y] = result[V_Y];
      p[i][V_Z] = result[V_Z];
      p[i][V_W] = result[V_W];
   }

   /* Already un-synced in the callobj routine... */

   b = dopoly(n);
   if (b && vdevice.attr->a.hatch)
   {
      hatch(n, (float (*)[3])dp);
   }
   
   vdevice.pos->cpW[V_X] = dp[V_X].f;
   vdevice.pos->cpW[V_Y] = dp[V_Y].f;
   vdevice.pos->cpW[V_Z] = dp[V_Z].f;
}

/*
 * poly2
 *
 *	construct a polygon from a (x, y) array of points provided by the user.
 */
void
poly2(int n, float dp[][2])
{
   int	i;
   float	np[MAXVERTS][3];

   for (i = 0; i < n; i++)
   {
      np[i][V_X] = dp[i][V_X];
      np[i][V_Y] = dp[i][V_Y];
      np[i][V_Z] = 0.0;
   }

   poly(n, np);
}

/*
 * poly
 *
 *	construct a polygon from an array of points provided by the user.
 */
void
poly(int n, float dp[][3])
{
   int	sync, b, i, j;
   Vector	vect, result;
   Token	*tok;

   if (!vdevice.initialised)
      verror(VERR_UNINIT, "poly");

   if (vdevice.inobject)
   {
      tok = newtokens(2 + 3 * n);
      tok[0].i = POLY;
      tok[1].i = n;
      for (i = 0, j = 2; i < n; i++, j += 3)
      {
         tok[j + V_X].f = dp[i][V_X];
         tok[j + V_Y].f = dp[i][V_Y];
         tok[j + V_Z].f = dp[i][V_Z];
      }
      return;
   }


   for (i = 0; i < n; i++)
   {
      vect[V_X] = dp[i][V_X];
      vect[V_Y] = dp[i][V_Y];
      vect[V_Z] = dp[i][V_Z];
      vect[V_W] = 1;
      multvector(result, vect, vdevice.transmat->m);
      p[i][V_X] = result[V_X];
      p[i][V_Y] = result[V_Y];
      p[i][V_Z] = result[V_Z];
      p[i][V_W] = result[V_W];
   }

   if ((sync = vdevice.sync))
      vdevice.sync = 0;

   b = dopoly(n);
   if (b && vdevice.attr->a.hatch)
   {
      hatch(n, dp);
   }
   
   if (sync)
   {
      vdevice.sync = 1;
      (*vdevice.dev.Vsync)();
   }

   vdevice.pos->cpW[V_X] = dp[0][V_X];
   vdevice.pos->cpW[V_Y] = dp[0][V_Y];
   vdevice.pos->cpW[V_Z] = dp[0][V_Z];
}

/*
 * pmove
 *
 *	set the start position of a polygon
 */
void
pmove(float x, float y, float z)
{
   np = 0;
   p[np][V_X] = x;
   p[np][V_Y] = y;
   p[np][V_Z] = z;
   p[np][V_W] = 1.0;
}

/*
 * pdraw
 *
 *	add another vertex to the polygon array
 */
void
pdraw(float x, float y, float z)
{
   char	buf[100];

   np++;

   if (np >= MAXVERTS)
   {
      sprintf(buf, "pdraw: can't draw polygon");
      verror(VERR_MAXPNTS, buf);
   }

   p[np][V_X] = x;
   p[np][V_Y] = y;
   p[np][V_Z] = z;
   p[np][V_W] = 1.0;
}

/*
 * makepoly
 *
 *	set up a polygon which will be constructed by a series of
 * move draws.
 */
void
makepoly(void)
{
   vdevice.inpolygon = 1;
   vdevice.pmove = pmove;
   vdevice.pdraw = pdraw;
   np = 0;
   p[np][V_X] = vdevice.pos->cpW[V_X];
   p[np][V_Y] = vdevice.pos->cpW[V_Y];
   p[np][V_Z] = vdevice.pos->cpW[V_Z];
   p[np][V_W] = 1.0;

}

/*
 * closepoly
 *
 *	draw the polygon started by the above.
 */
void
closepoly(void)
{
   float	lstx, lsty, lstz;
   Vector	result;
   int	sync, b, i, j;
   Token	*tok;

   if (!vdevice.initialised)
      verror(VERR_UNINIT, "closepoly");

   vdevice.inpolygon = 0;

   if (vdevice.inobject)
   {
      tok = newtokens(2 + 3 * (np + 1));
      tok[0].i = POLY;
      tok[1].i = np + 1;
      for (i = 0, j = 2; i <= np; i++, j += 3)
      {
         tok[j + V_X].f = p[i][V_X];
         tok[j + V_Y].f = p[i][V_Y];
         tok[j + V_Z].f = p[i][V_Z];
      }

      return;
   }

   lstx = p[np][V_X];
   lsty = p[np][V_Y];
   lstz = p[np][V_Z];

   np++;

   if (vdevice.attr->a.hatch)
   {
      for (i = 0; i < np; i++)
      {
         newp[i][0] = p[i][0];
         newp[i][1] = p[i][1];
         newp[i][2] = p[i][2];
      }
   }

   for (i = 0; i < np; i++)
   {
      multvector(result, p[i], vdevice.transmat->m);
      p[i][V_X] = result[V_X];
      p[i][V_Y] = result[V_Y];
      p[i][V_Z] = result[V_Z];
      p[i][V_W] = result[V_W];
   }

   if ((sync = vdevice.sync))
   {
      vdevice.sync = 0;
   }
   
   b = dopoly(np);
   if (b && vdevice.attr->a.hatch)
   {
      hatch(np, newp);
   }
   
   if (sync)
   {
      vdevice.sync = 1;
      (*vdevice.dev.Vsync)();
   }

   vdevice.pos->cpW[V_X] = lstx;
   vdevice.pos->cpW[V_Y] = lsty;
   vdevice.pos->cpW[V_Z] = lstz;
}

/*
 * checkbacki
 *
 *	Checks if a transformed polygon is backfacing or not.
 */
static	int
checkbacki(void)
{

#ifdef	PC	/*	Only has 16 bit ints */
#define	BACKFACE(z)	(clockwise ? ((z) <= 0L) : ((z) > 0L))
   long	z;
#else
#define	BACKFACE(z)	(clockwise ? ((z) <= 0) : ((z) > 0))
   int	z;
#endif

   int	x1, x2, y1, y2;

   x1 = ip1[1] - ip1[0];
   x2 = ip1[2] - ip1[1];
   y1 = ip2[1] - ip2[0];
   y2 = ip2[2] - ip2[1];

#ifdef	PC
   z = (long)x1 * (long)y2 - (long)y1 * (long)x2;
#else
   z = x1 * y2 - y1 * x2;
#endif

   return(BACKFACE(z));
}

/*
 * The following routines are an implementation of the Sutherland - Hodgman
 * polygon clipper, as described in "Reentrant Polygon Clipping"
 * Communications of the ACM Jan 1974, Vol 17 No. 1.
 */
static void
polyclip(register int n)
{
   int	i;

   nout = 0;
   for (i = 0; i < 6; i++)
   {
      first[i] = 1;
   }
   
   for (i = 0; i < n; i++)
   {
      shclip(p[i], 0);
   }
   
   shclose(0);
}

static void
shclip(float p[4], int side)
{
   float	P[4];

   if (side == 6)
   {
      ip1[nout] = WtoVx(p);
      ip2[nout++] = WtoVy(p);
   }
   else
   {
      copyvector(P, p);
      if (first[side])
      {
         first[side] = 0;
         copyvector(F[side], P);
      }
      else if (intersect(side, I, P))
      {
         shclip(I, side + 1);
      }
      copyvector(S[side], P);
      if (visible(side))
      {
         shclip(S[side], side + 1);
      }
   }
}

static void
shclose(int side)
{
   if (side < 6)
   {
      if (intersect(side, I, F[side]))
         shclip(I, side + 1);

      shclose(side + 1);

      first[side] = 1;
   }
}

static int
intersect(int side, register Vector I, register Vector p)
{
   register	float	wc1 = 0.0, wc2 = 0.0, a;

   switch (side) {
      case 0:		/* x - left */
         wc1 = p[3] + p[0];
         wc2 = S[side][3] + S[side][0];
         break;
      case 1:		/* x - right */
         wc1 = p[3] - p[0];
         wc2 = S[side][3] - S[side][0];
         break;
      case 2:		/* y - bottom */
         wc1 = p[3] + p[1];
         wc2 = S[side][3] + S[side][1];
         break;
      case 3:		/* y - top */
         wc1 = p[3] - p[1];
         wc2 = S[side][3] - S[side][1];
         break;
      case 4:		/* z - near */
         wc1 = p[3] + p[2];
         wc2 = S[side][3] + S[side][2];
         break;
      case 5:		/* z - far */
         wc1 = p[3] - p[2];
         wc2 = S[side][3] - S[side][2];
         break;
      default:
         verror(VERR_CLIPERR, "intersect: ridiculous side value");
   }

   if (wc1 * wc2 < 0.0) {	/* Both are opposite in sign - crosses */
      a = wc1 / (wc1 - wc2);
      if (a < 0.0 || a > 1.0) {
         return(0);
      } else {
         I[0] = p[0] + a * (S[side][0] - p[0]);
         I[1] = p[1] + a * (S[side][1] - p[1]);
         I[2] = p[2] + a * (S[side][2] - p[2]);
         I[3] = p[3] + a * (S[side][3] - p[3]);
         return(1);
      }
   }

return(0);
}

static int
visible(int side)
{
   float	wc = 0.0;

   switch (side) {
      case 0:		/* x - left */
         wc = S[side][3] + S[side][0];
         break;
      case 1:		/* x - right */
         wc = S[side][3] - S[side][0];
         break;
      case 2:		/* y - bottom */
         wc = S[side][3] + S[side][1];
         break;
      case 3:		/* y - top */
         wc = S[side][3] - S[side][1];
         break;
      case 4:		/* z - near */
         wc = S[side][3] + S[side][2];
         break;
      case 5:		/* z - far */
         wc = S[side][3] - S[side][2];
         break;
      default:
         verror(VERR_CLIPERR, "visible: ridiculous side value");
   }

   return(wc >= 0.0);
}
