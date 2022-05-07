
/** \file spline.c
 *
 */

#include "vopl.h"

/* 
 *  spline
 *
 *     calculates a cubic spline through the data points
 *     
 */
void spline(double *x, double *y, int np)
{
   double 	*l, *u, *z, *a, *b, *c, *d, *h, *xx;
   float	*alpha;
   int 	delta;
   double 	*newm1();
   double 	**newm2();

   double 	t, ax, ay, xdiv;
   int   	n, i, j;

   /* get memory for variables */

   h = newm1(np);
   l = newm1(np);
   u = newm1(np);
   z = newm1(np);
   a = newm1(np);
   b = newm1(np);
   c = newm1(np);
   d = newm1(np);
   xx = newm1(np);
   alpha = newm1(np);

   n = np - 2;
   for (i = 0; i <= n; i++)
   {
      xx[i] = WhatX(x[i]);
      h[i] = WhatX(x[i + 1]) - xx[i];
      a[i] = WhatY(y[i]);
   }

   n = np - 1;
   xx[n] = WhatX(x[n]);
   a[n] = WhatY(y[n]);

   if (plotdev->splinetype == CLAMPED)
   {
      alpha[0] = 3 * (a[1] - a[0]) / h[0] - 3 * plotdev->s1;
      alpha[n] = 3 * plotdev->sn - 3 * (a[n] - a[n - 1]) / h[n - 1];
   }

   for (i = 1; i < n; i++)
   {
      alpha[i] = 3 * (a[i + 1] * h[i - 1] - a[i] * (xx[i + 1] - xx[i - 1])  
                      + a[i - 1] * h[i]) / (h[i - 1] * h[i]);
   }
   
   if (plotdev->splinetype == CLAMPED)
   {
      l[0] = 2 * h[0];
      u[0] = 0.5;
      z[0] = alpha[0] / l[0];
   }
   else
   {
      l[0] = 1.0;
      u[0] = 0.0;
      z[0] = 0.0;
   }

   for (i = 1; i <= n; i++)
   {
      l[i] = 2 * (xx[i + 1] - xx[i - 1]) - h[i - 1] * u[i - 1];
      u[i] = h[i] / l[i]; 
      z[i] = (alpha[i] - (h[i - 1] * z[i - 1])) / l[i];
   }

   if (plotdev->splinetype == CLAMPED)
   {
      l[n] = h[n- 1] * (2.0 - u[n - 1]);
      z[n] = (alpha[n] - h[n - 1] * z[n - 1]) / l[n];
      c[n] = z[n];
   }
   else
   {
      l[n] = 1.0;
      z[n] = 0.0;
      c[n] = 0.0;
   }

   for (j = n - 1; j >= 0; j--)
   {
      c[j] = z[j] - (u[j] * c[j + 1]);
      b[j] = (a[j + 1] - a[j]) / h[j] - (h[j] * (c[j + 1] + 2 * c[j]) / 3.0);
      d[j] = (c[j + 1] - c[j]) / (3.0 * h[j]);
   }


   /*
    * Draw the trace
    */

   delta = (int)((double)plotdev->precision / (np - 1));
   ay = a[0]; /*  + b[0] * t + c[0] * t * t + d[0] * t * t * t;*/
   move2(xx[0], ay);

   for (j = 0; j <= n - 1; j++ )
   {
      xdiv = h[j] / (double)delta;
      for (t = 0.0; t <= h[j]; t += xdiv)
      {
         ax = xx[j] + t;
         ay = a[j]  + b[j] * t + c[j] * t * t + d[j] * t * t * t;
         draw2(ax, ay);
      }
   }

   free(h);
   free(l);
   free(u);
   free(z);
   free(a);
   free(b);
   free(c);
   free(d);
   free(xx);
   free(alpha);
}
