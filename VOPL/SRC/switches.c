
/** \file switches.c
 *
 */

#include "vopl.h"


/*
 * fit
 *
 *	Specify the type of fit for a graph
 */
void fit(int type)
{

   if (type < 0 || type > MAX_FIT)
   {
      vopl_error(VOPL_ERR_UNKNOWN_FIT_TYPE,"fit");
   }

   plotdev->fit = type;
}

/*
 * skip
 *
 *	causes scaling and plotting to skip every n data points
 */
void skip(int n)
{
   plotdev->arrayind = n;
}

/*
 * endslopes
 *
 *	Specify the end slopes for a cubic spline fit.
 *	If No slopes have been specified, then we'll use
 *	the VOGLE curve routine.
 */
void endslopes(double a, double b)
{
   plotdev->s1 = a;
   plotdev->sn = b;
   plotdev->splinetype = CLAMPED;
}

/*
 * gridspacing
 *
 *	Turns grids on or off
 */
void gridspacing(int spacingX, int spacingY)
{
   plotdev->gridX = spacingX;
   plotdev->gridY = spacingY;
}

void  withbox(int yes)
{
   plotdev->box = yes;
}

/*
 * scaling
 *
 *	Specify the type of scaling to be used for axis drawing
 * and graph plotting.
 */
void scaling(int type, char axis)
{
   switch (axis)
   {
      case 'x':
      case 'X':
         plotdev->axes[XIND].scaling = type;
         break;
      case 'y':
      case 'Y':
         plotdev->axes[YIND].scaling = type;
         break;
      case 'z':
      case 'Z':
         plotdev->axes[ZIND].scaling = type;
         break;
      default:
         vopl_error(VOPL_ERR_AXIS_NB,"scaling");
   }
}

/*
 * tickmarks
 *
 *	Specify the number of tickmarks on an axis. For autoscaled
 *	axes (ie those using "adjustscale"), the number of tick marks
 *	may not come out exactly as specified.
 */
void tickmarks(int num, char axis)
{

   if (num < 0)
   {
      vopl_error(VOPL_ERR_TICKMARKS,"tickmarks");
   }
   else
   {
      switch (axis)
      {
         case 'x':
         case 'X':
            plotdev->axes[XIND].nticks = num;
            break;
         case 'y':
         case 'Y':
            plotdev->axes[YIND].nticks = num;
            break;
         case 'z':
         case 'Z':
            plotdev->axes[ZIND].nticks = num;
            break;
         default:
            vopl_error(VOPL_ERR_AXIS_NB,"tickmarks");
      }

      plotdev->forceticks = 1;
   }
}

/*
 * tickspacing
 *
 *	Specify the spacing of tickmarks on an axis. For autoscaled
 *	axes (ie those using "adjustscale"), the number of tick marks
 *	may not come out exactly as specified.
 */
void tickspacing(int num, char axis)
{
   if (num < 0)
   {
      vopl_error(VOPL_ERR_TICKMARKS,"tickspacing");
   }
   else
   {
      switch (axis)
      {
         case 'x':
         case 'X':
            plotdev->axes[XIND].ntspacing = num;
            break;
         case 'y':
         case 'Y':
            plotdev->axes[YIND].ntspacing = num;
            break;
         case 'z':
         case 'Z':
            plotdev->axes[ZIND].ntspacing = num;
            break;
         default:
            vopl_error(VOPL_ERR_AXIS_NB,"tickspacing");
      }
   }
}


/*
 * minorticks
 *
 *	Specify the number of minortickmarks bewteen each tick on an axis.
 * 	On a log scaled axis, this simply says to draw/not draw the minor
 *	minor tick marks ... their number being always set to 10.
 */
void minorticks(int num, char axis)
{
   if (num < 0)
   {
      vopl_error(VOPL_ERR_TICKMARKS,"minorticks");
   }
   else
   {
      switch (axis)
      {
         case 'x':
         case 'X':
            plotdev->axes[XIND].minorticks = num;
            break;
         case 'y':
         case 'Y':
            plotdev->axes[YIND].minorticks = num;
            break;
         case 'z':
         case 'Z':
            plotdev->axes[ZIND].minorticks = num;
            break;
         default:
            vopl_error(VOPL_ERR_AXIS_NB,"minorticks");
      }
   }
}

/*
 * annotate
 *
 *	Turn on or off labeling of the drawn axes
 */
void annotate(char *format, char axis)
{
   axisdata	*ax;


   /*
    * wooh Jimmy!
    */
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
         vopl_error(VOPL_ERR_AXIS_NB,"annotate");
   }

   ax->annotate = (format ? 1 : 0);
   ax->format = savestr(ax->format, format);
}

/*
 * arrayindex
 *
 *	How to index into the arrays passed to adjustscale and plot
 *	eg, increment the index by one or by more.
 *
 */
void arrayindex(int i)
{
   plotdev->arrayind = i;
}

/*
 * marker
 *
 *	Defines the current marker string to be used on subsequent
 *	plot calls.
 */
void marker(char *string)
{
   plotdev->marker = savestr(plotdev->marker, string);
}

/*
 * markerspacing
 *
 *	Sets the spacing for the markers.
 */
void markerspacing(int spacing)
{
   plotdev->markerspacing = spacing;
}

/*
 * markerscale
 *
 *	Defines a multiplicative scale factor (from the default) for
 *	the size of markers drawn.
 */
void markerscale(double s)
{
   plotdev->markerscale = s;
}


/*
 * graphtitle
 *
 *	sets the title for the graph
 */
void graphtitle(char *s)
{
   plotdev->graphtitle = savestr(plotdev->graphtitle, s);
}

/*
 * range
 *
 *	explicitly set the range of an axis
 */
void range(double min, double max, char axis)
{
   axisdata	*ax;


   switch (axis)
   {
      case 'x':
      case 'X':
         ax = &plotdev->axes[XIND];
         ax->div = (WhatX(max) - WhatX(min)) / ax->nticks;
         break;
      case 'y':
      case 'Y':
         ax = &plotdev->axes[YIND];
         ax->div = (WhatY(max) - WhatY(min)) / ax->nticks;
         break;
      case 'z':
      case 'Z':
         ax = &plotdev->axes[ZIND];
         ax->div = (WhatZ(max) - WhatZ(min)) / ax->nticks;
         break;
      default:
         vopl_error(VOPL_ERR_AXIS_NB,"range");
   }

   ax->min = min;
   ax->max = max;

   if (ax->scaling == LINEAR)
      ax->div = (max - min) / ax->nticks;
   else
      ax->div = (log10((double)max) - log10((double)min)) / ax->nticks;

   ax->scaleset = 1;
}
