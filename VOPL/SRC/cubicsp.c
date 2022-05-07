
/** \file cubicsp.c
 *
 */

#include "vopl.h"


/*
 * 	cubicsp
 *
 *	Fit cubic splines using Cardinal splines and vogle "curve" routine
 */
void cubicsp(double *x, double *y, int n)
{
   double  geom[4][3], x0, y0, xnp1, ynp1;
   int	   i, j;

   /*
      *  Make fake end points
    */
   x0 =   2.0 * WhatX(x[0])   - WhatX(x[1]);
   y0   = 2.0 * WhatY(y[0])   - WhatY(y[1]);
   xnp1 = 2.0 * WhatX(x[n-1]) - WhatX(x[n-2]);
   ynp1 = 2.0 * WhatY(y[n-1]) - WhatY(y[n-2]);

   /*
      *  Do first segment
    */
   for (i = 0; i < 3; i++)
   for (j = 0; j < 4; j++)
   {
      geom[j][i] = 0.0;
   }

   geom[0][0] = x0;
   geom[0][1] = y0;
   for (i = 1; i < 4; i++)
   {
      geom[i][0] = WhatX(x[i-1]);
      geom[i][1] = WhatY(y[i-1]);
   }

   curve(geom);

   /*
      * In between curve segments
    */
   for (j = 0; j < n-3; j++)
   {
      for (i = 0; i < 4; i++)
      {
         geom[i][0] = WhatX(x[i+j]);
         geom[i][1] = WhatY(y[i+j]);
      }

      curve(geom);
   }

   /*
      *  Last segment
    */
   for (i = 0; i < 3; i++)
   {
      geom[i][0] = WhatX(x[n+i-3]);
      geom[i][1] = WhatY(y[n+i-3]);
   }

   geom[3][0] = xnp1;
   geom[3][1] = ynp1;
   curve(geom);
}
