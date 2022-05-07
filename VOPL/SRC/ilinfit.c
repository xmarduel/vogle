
/** \file ilinfit.c
 *
 */

#include "vopl.h"


/*
 * pefit
 *
 *	power equation fit
 *
 *	ln y = ln a + b.ln x
 *
 */
void pefit(double *x, double *y, int n)
{
   double  xval, xstep, yval, sumx, sumy, sumxy, sumxx, a, b, yave, xave;
   int	   i;

   sumx = sumy = sumxy = sumxx = 0.0;

   for (i = plotdev->startind; i < n; i += plotdev->arrayind)
   {
      sumx += WhatX(log(x[i]));
      sumy += WhatY(log(y[i]));
      sumxy += WhatX(log(x[i])) * WhatY(log(y[i]));
      sumxx += WhatX(log(x[i])) * WhatY(log(x[i]));
   }

   yave = sumy / n;
   xave = sumx / n;

   b = (n * sumxy - sumx * sumy) / (n * sumxx - sumx * sumx);
   a = exp(yave - b * xave);

   xval = WhatX(plotdev->axes[XIND].min);
   yave = a * pow(xval, b);

   move2(xval, yave);

   xstep = (WhatX(plotdev->axes[XIND].max) - WhatX(plotdev->axes[XIND].min)) / (2 * n);
   for (i = plotdev->startind; i < 2 * n; i += plotdev->arrayind)
   {
      xval += xstep;
      yval = a * pow(xval, b);
      draw2(xval, yval);
   }
}

/*
 * sgrfit
 *
 *	saturated growth rate fit
 *
 *	y = c * x / (d + x)
 *
 *      invert
 *
 *      1/y = d/c . 1/x + 1/c
 *
 *
 */
void sgrfit(double *x, double *y, int n)
{
   double	xval, xstep, yval, sumx, sumy, sumxy, sumxx, a, b, yave, xave;
   double	a3, b3;
   int	i;

   sumx = sumy = sumxy = sumxx = 0.0;

   for (i = plotdev->startind; i < n; i += plotdev->arrayind)
   {
      sumx += 1.0 / x[i];
      sumy += 1.0 / y[i];
      sumxy += (1.0 / x[i]) * (1.0 / y[i]);
      sumxx += (1.0 / x[i]) * (1.0 / x[i]);
   }

   yave = sumy / n;
   xave = sumx / n;

   b = (n * sumxy - sumx * sumy) / (n * sumxx - sumx * sumx);
   a = yave - b * xave;

   a3 = 1.0 / a;
   b3 = b * a3;

   xval = WhatX(plotdev->axes[XIND].min);
   yval = a3 * xval / (b3 + xval);
   move2(xval, yave);

   xstep = (WhatX(plotdev->axes[XIND].max) - WhatX(plotdev->axes[XIND].min)) / (2 * n);

   for (i = plotdev->startind; i < 2 * n; i += plotdev->arrayind)
   {
      xval += xstep;
      yval = a3 * xval / (b3 + xval);
      draw2(xval, yval);
   }
}
