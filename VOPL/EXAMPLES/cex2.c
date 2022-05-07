#include <stdio.h>
#include <math.h>
#include "vopl.h"

#define	PI	3.14159265358979
#define	N	300

float	x[N], y[N];

/*
 *	Another very simple test program for vopl.
 *
 * 	This one also draws a graph of y = sin(x) 0 <= x <= 2 * PI
 *	but shows some of the axis and range setting stuff
 */
main()
{
	int	i;
	char	device[30] = "X11";
	float	t, dt;

/*
 *	Get VOGLE device
 */
	/*
         printf("Enter VOGLE device: ");
	gets(device);
         */
        vopl_winopen("X11", "Xavier1");
        

/*
 *	Generate the points
 */
	t = 0.0;
	dt = 2 * PI / N;

	for (i = 0; i != N; i++) {
		x[i] = t;
		y[i] = sin(t);
		t = t + dt;
	}

/*
 *	Set the X-scaling to be absolute 0 - 10 (ie no auto-scaling)
 */
	range(0.0, 10.0, 'x');
/*
 *	Autoscale the Y-axis
 */
	adjustscale(y, N, 'y');
/*
 *	Anyone for some axis titles?
 */
	axistitle("This one's for you", 'x');
	axistitle("This one's for me", 'y');
/*
 *	As we are now about to do some graphics we initialise VOGLE
 *	and clear to BLACK
 */
	vopl_winopen("X11", "Xavier2");
	color(0);
	clear();
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
 *	Draw the Graph
 */
	plot2(x, y, N);
/*
 *	Wait around a bit
 */
	getkey();
/*
 *	Now draw a little one in the top right hand corner
 *	by reseting the VOGLE viewport.
 */
	viewport(0.0, 1.0, 0.0, 1.0);
/*
 *	Draw it again, but do the plot first (in BLUE) then the axes
 *	(in YELLOW)
 */
	color(4);
	plot2(x, y, N);
	color(3);
	drawaxes();
/*
 *	Hang around again
 */
	getkey();
/*
 *	Clear it all away
 */
	color(0);
	clear();
/*
 *	Reset the viewport to be a "long skinny one"
 */
	viewport(-1.0, 1.0, -0.5, 0.5);
/*
 *	Autoscale the X-axis again by first setting a ridicuous scale with
 *	range that adjustscale will change.
 */
	range(1000.0, -1000.0, 'x');
	adjustscale(x, N, 'x');
/*
 *	Change the X-axis title
 */
	axistitle("Blark Bonk Bloot", 'x');
/*
 *	And draw it all again...
 */
	color(5);
	drawaxes();
	color(6);
	plot2(x, y, N);
/*
 *	Hang around again
 */
	getkey();
/*
 *	Bugger off...
 */

	vexit();
}
