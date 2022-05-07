#include <stdio.h>
#include <math.h>
#include "vopl.h"

#define	N	10

/*
 *	Another simple test program for vopl.
 *
 * 	This one tries to show the various "fit" options
 */
main()
{
	char		device[30];
	static float	x[N] = {
			1.0, 2.0, 3.0, 6.0,
			17.0, 19.0, 23.0, 45.0,
			50.0, 56.0
	};
	static float	y[N] = {
			1.0, 3.0, 5.0, 9.0,
			17.0, 45.0, 23.0, 99.0,
			50.0, 20.0
	};

/*
 *	Get VOGLE device
 */
	/*
         printf("Enter VOGLE device: ");
	gets(device);
         */
        vopl_winopen("X11", NULL);

/*
 *	First we'll do a linear least square fit.
 */
	fit(2);
	degree(1);
/*
 *	Adjust the scaling according to x and y arrays
 */
	adjustscale(x, N, 'x');
	adjustscale(y, N, 'y');
/*
 *	Give it a title
 */
	graphtitle("Linear Least square fit");
/*
 *	As we are now about to do some graphics we initialise VOGLE
 *	and clear to BLACK
 */
	
	color(0);
	clear();
/*
 *	Draw the title in CYAN
 */
	color(6);
	drawtitle();
/*
 *	Now set the color to GREEN
 */
	color(2);

/*
 *	Draw the default set of axes (in GREEN)
 */
	drawaxes();
/*
 *	Set color to RED
 */
	color(1);
/*
 *	Change to the "markers" font and set the current marker string
 */
	font("markers");
	marker("i");
/*
 *	Draw the Graph
 */
	plot2(x, y, N);
/*
 *	Wait around a bit
 */
	getkey();
/*
 *	Now we'll do a second order fit.
 */
	degree(2);
	graphtitle("Second order least square fit");

	color(0);
	clear();

	color(7);
	plot2(x, y, N);
/*
 *	Change back to the "text" type font to draw the title and axes
 */
	font("futura.m");

	color(3);
	drawaxes();

	color(6);
	drawtitle();
/*
 * 	Wait a bit
 */
	getkey();
/*
 *	Now do a Cubic spline fit (cardinal spline for this one)
 */
	fit(3);
	
	color(0);
	clear();

	color(5);
	drawaxes();

	graphtitle("Cardinal Cubic Spline Fit");
	color(6);
	drawtitle();

/*
 *	Note, we haven't changed to the Marker font here
 */
	plot2(x, y, N);

	getkey();

	vexit();
}
