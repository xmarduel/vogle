
/** \file plo.c
 *
 */

#include "vopl.h"


/*
 * for splines calculation
 */

static double	cardinal[4][4] = {
   {-0.5,   1.5,  -1.5,  0.5},
   { 1.0,  -2.5,   2.0, -0.5},
   {-0.5,   0.0,   0.5,	 0.0},
   { 0.0,   1.0,   0.0,  0.0}
};

/*
 * plot2
 *
 *	display the 2-d graph described by x and y;
 */
void plot2(double *x, double *y, int n)
{
   int	i;
   double	tx, ty, left, right, bottom, top;
   double	dashlen, a, b;

   /*
    * Set the view up so that clipping happens...
    */

   pushmatrix();
   pushviewport();
   pushattributes();

   getviewport(&left, &right, &bottom, &top);

   viewport(left   + (1.0 + XMIN) / 2.0 * (right - left),
            right  - (1.0 - XMAX) / 2.0 * (right - left),
            bottom + (1.0 + YMIN) / 2.0 * (top - bottom),
            top    - (1.0 - YMAX) / 2.0 * (top - bottom));

   if (!plotdev->axes[XIND].scaleset)
      adjustscale(x, n, 'x');

   if (!plotdev->axes[YIND].scaleset)
      adjustscale(y, n, 'y');

   ortho2(WhatX(plotdev->axes[XIND].min),
          WhatX(plotdev->axes[XIND].max),
          WhatY(plotdev->axes[YIND].min),
          WhatY(plotdev->axes[YIND].max));

   /*
    * Set dash len so that we get about 200 dashes in one plot
    */
   a = WhatX(plotdev->axes[XIND].max) - WhatX(plotdev->axes[XIND].min);
   b = WhatY(plotdev->axes[YIND].max) - WhatY(plotdev->axes[YIND].min);

   dashlen = sqrt(a*a + b*b) / 100.0;
   dashlen = 0.015;
   setdash(dashlen);

   /*
    * Do whatever type of fit is required....
    */
   switch (plotdev->fit)
   {
      case NO_LINES:
         /*
          * Don't do a sniveling thing......
          */
         break;

      case STRAIGHT_LINE:
         /*
          * Just draw it.....
          */

         move2(WhatX(x[0]), WhatY(y[0]));
         for (i = 1; i < n; i++)
         {
            draw2(WhatX(x[i]), WhatY(y[i]));
         }

            break;

      case LEAST_SQUARE:
         /*
          * Do an orthogonal polynomial fit.....
          */

         /*
          * If degree is zero then simply plot a line y = average y
          */
         if (plotdev->degree == 0)
            avefit(x, y, n);
         /*
          * If degree is 1 then it's a simple linear least square fit...
          */
         else if (plotdev->degree == 1)
            llsfit(x, y, n);
         else
            orthofit(x, y, plotdev->degree, n);
         break;

      case CUBIC_SPLINE:
         /*
          * Do cubic spline fits......
          */
         /*
         * If no endslopes have been used then use VOGLE curve
          * routine with cardinal splines.
          */
         if (plotdev->splinetype == FREE)
         {
            curvebasis(cardinal);
            curveprecision(20);
            cubicsp(x, y, n);
         }
         else
         {
            spline(x, y, n);
         }
         break;

      case POWER_EQN:
         /*
          * do power equation fit
          */
         pefit(x, y, n);
         break;

      case SGR_FIT:
         /*
          * do a saturated growth rate fit
          */
         sgrfit(x, y, n);
         break;

      default:
         vopl_error(VOPL_ERR_UNKNOWN_FIT_TYPE,"plot2");
   }

   /*
    * Set up the marker size
    */

   if (plotdev->markerspacing && plotdev->marker != (char *)NULL)
   {
      if (plotdev->markerscale != 0.0)
      {
         pushattributes();
         centertext(1);
         /*clipping(0);*/
         tx = plotdev->markerscale * TEXTWIDTH  * WhatX((plotdev->axes[XIND].max - plotdev->axes[XIND].min));
         ty = plotdev->markerscale * TEXTHEIGHT * WhatY((plotdev->axes[YIND].max - plotdev->axes[YIND].min));

         textsize(tx, ty);

         linestyle("");
         for (i = 0; i < n; i += plotdev->markerspacing)
         {
            move2(WhatX(x[i]), WhatY(y[i]) + 0.15 * ty);
            drawstr(plotdev->marker);
         }
         popattributes();
      }
      else
      {
         for (i = 0; i < n; i += plotdev->markerspacing)
         {
            point2(WhatX(x[i]), WhatY(y[i]));
         }
      }
   }

   popattributes();
   popviewport();
   popmatrix();
}
