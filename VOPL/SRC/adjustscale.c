
/** \file adjustscale.c
 *
 */

#include "vopl.h"

#include <float.h>


/*
 *	adjustscale
 *
 *	Adjust the scaling for the specified axis according to the
 *	values in the array x.
 */
void adjustscale(double *x, int n, char axis)
{
   int		i, nticks;
   double       min, max, a, b, div;
   axisdata	*ax;

   /* get ax from the static data "plotdev" */
   switch (axis)
   {
      case 'x':
      case 'X':
         ax = &plotdev->axes[XIND];
         break;
      case 'y':
      case 'Y':
         ax = &plotdev->axes[YIND];
         break;
      case 'z':
      case 'Z':
         ax = &plotdev->axes[ZIND];
         break;
      default:
         vopl_error(VOPL_ERR_AXIS_NB, "adjustscale");
   }

   nticks = ax->nticks;
   min    = ax->min;
   max    = ax->max;
   ax->scaleset = 2;

   /*
    * Find min and max of array x.
    */

   a = b = x[0];
   for (i = plotdev->arrayind; i < n; i += plotdev->arrayind)
   {
      a = MIN(a, x[i]);
      b = MAX(b, x[i]);
   }

   if (a < min || b > max)
   {
      /* We only do this if we have to */

      if (a < min) 
         min = a;

      if (b > max)
         max = b;

      if (!nticks)
         nticks = 5;

      if (ax->scaling == LOGARITHMIC)
         logscale(min, max, nticks, &a, &b, &div);
      else if (plotdev->forceticks) 
         linscale2(min, max, nticks, &a, &b, &div);
      else
         linscale1(min, max, 5, &a, &b, &div) ;

      /* the min & max are set by the calculus */
      ax->min = a;
      ax->max = b;

      ax->div = div;
   }
}

void setwindow(double min, double max, char axis)
{
   int		nticks;
   double   a, b, div;
   axisdata	*ax;

   switch (axis)
   {
      case 'x':
      case 'X':
         ax = &plotdev->axes[XIND];
         break;
      case 'y':
      case 'Y':
         ax = &plotdev->axes[YIND];
         break;
      case 'z':
      case 'Z':
         ax = &plotdev->axes[ZIND];
         break;
      default:
         vopl_error(VOPL_ERR_AXIS_NB, "setwindow");
   }

   ax->scaleset = 1;

   nticks = ax->nticks;
   
   if (ax->scaling == LOGARITHMIC)
      logscale(min, max, nticks, &a, &b, &div);
   else if (plotdev->forceticks)
      linscale2(min, max, nticks, &a, &b, &div);
   else
      linscale1(min, max, 5, &a, &b, &div) ;

   /* the min & max are set by us */
   ax->min = min;
   ax->max = max;
   
   ax->div = div;
}


void unsetwindow(char axis)
{
   axisdata	*ax;

   switch (axis)
   {
      case 'x':
      case 'X':
         ax = &plotdev->axes[XIND];
         break;
      case 'y':
      case 'Y':
         ax = &plotdev->axes[YIND];
         break;
      case 'z':
      case 'Z':
         ax = &plotdev->axes[ZIND];
         break;
      default:
         vopl_error(VOPL_ERR_AXIS_NB, "unsetwindow");
   }

   ax->scaleset = 0;

   ax->min = -FLT_MAX;
   ax->max =  FLT_MAX;
}
