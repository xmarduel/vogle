#include "vogle.h"

static	float	Vcx, Vcy, Vsx, Vsy;

/*
 * VtoWxy
 *
 * Return the world x and y values corresponding to the input
 * screen x and y values.
 * THIS ROUTINE IS BOLLOCKS and should be rethought.
 */
void
VtoWxy(float xs, float ys, float *xw, float *yw)
{
   float	d, a1, a2, b1, b2, c1, c2, A, B;

   A = (xs - Vcx) / Vsx;
   B = (ys - Vcy) / Vsy;

   a1 = vdevice.transmat->m[0][0] - vdevice.transmat->m[0][3] * A;
   a2 = vdevice.transmat->m[0][1] - vdevice.transmat->m[0][3] * B;

   b1 = vdevice.transmat->m[1][0] - vdevice.transmat->m[1][3] * A;
   b2 = vdevice.transmat->m[1][1] - vdevice.transmat->m[1][3] * B;

   c1 = vdevice.transmat->m[3][3] * A - vdevice.transmat->m[3][0];
   c2 = vdevice.transmat->m[3][3] * B - vdevice.transmat->m[3][1];

   d = (a2 * b1 - b2 * a1);

   if (d != 0.0)
   {
      *xw = (b1 * c2 - c1 * b2) / d;
      *yw = (c1 * a2 - a1 * c2) / d;
   }
   else
   {
      *xw = A;
      *yw = B;
   }

   if (*xw == 0.0)
      *xw = A;

   if (*yw == 0.0)
      *yw = B;
}

/*
 * calcW2Vcoeffs
 *
 *	Calculate the linear coeffs defining the mapping of world
 *	space to actual device space
 */
void
CalcW2Vcoeffs(void)
{
   Vcx = (float)(vdevice.maxVx + vdevice.minVx) * 0.5;
   Vcy = (float)(vdevice.maxVy + vdevice.minVy) * 0.5;

   Vsx = (float)(vdevice.maxVx - vdevice.minVx) * 0.5;
   Vsy = (float)(vdevice.maxVy - vdevice.minVy) * 0.5;
}

/*
 * WtoVx
 *
 * return the Screen X coordinate corresponding to world point 'p'
 * (does the perspective division as well)
 */
int
WtoVx(float p[])
{
   return((int)(p[0] * Vsx / p[3] + Vcx));
}

/*
 * WtoVy
 *
 * return the Screen Y coordinate corresponding to world point 'p'
 * (does the perspective division as well)
 */
int
WtoVy(float p[])
{
   return((int)(p[1] * Vsy / p[3] + Vcy));
}


