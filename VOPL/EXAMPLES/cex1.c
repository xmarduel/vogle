#include <stdio.h>

#include "vogle.h"
#include "vopl.h"

#define	N	100

float	x[N], y[N];

/*
 *	A very simple test program for vopl.
 *
 * 	This one draws a graph of y = sin(x) 0 <= x <= 2 * PI
 */
int main()
{
   double t, dt;
   int	  i,j,k;
   int nx=2,ny=2;
   int idx[10];
   
   /*
    *	Generate the points
    */
   t = 0.0;
   dt = 2 * PI / N;

   for (i = 0; i != N; i++)
   {
      x[i] = 1+t;
      y[i] = 2+sin(t);
      t = t + dt;
   }

   /*
    *	As we are now about to do some graphics we initialise VOGLE
    *	and clear to BLACK
    */

   prefsize(800, 500);
  
   
   idx[0] = vopl_winopen("X11", "w1"); expandviewport();
   idx[1] = vopl_winopen("X11", "w2"); expandviewport();
   idx[2] = vopl_winopen("X11", "w3"); expandviewport();
   idx[3] = vopl_winopen("X11", "w4"); expandviewport();
   idx[4] = vopl_winopen("X11", "w5"); expandviewport();
   idx[5] = vopl_winopen("X11", "w6"); expandviewport();
   
   color(BLACK);
   clear();


   k = 0;

   subp(nx,ny);

   for (i=1; i<=nx; i++)
   for (j=1; j<=ny; j++)
   {
      panel(i,j);

      /*annotate("%d", 'x');*/
      k = (i-1)*ny + (j-1);

      if ( k%2 != 0 )
      {
         scaling(LOGARITHMIC,'x');
         scaling(LOGARITHMIC,'y');
      }
      else
      {
         scaling(LINEAR,'x');
         scaling(LINEAR,'y');
      }
      
      withbox(1);
      /*
       *	Adjust the scaling according to x and y arrays
       */
      adjustscale(x, N, 'x');
      adjustscale(y, N, 'y');

      /*
       *	Now set the color to GREEN
       */
      color(GREEN);

      /*
       *	Draw the default set of axes (in GREEN)
       */
      axistitle("(X)", 'x');
      axistitle("(Y)", 'y');

      drawaxes();
      /*
       *	Set color to RED
       */
      color(RED);

      /*
       *	Draw the Graph
       */
      plot2(x, y, N);

      /*
       * title
       */
      graphtitle("THE TITLE");
      drawtitle();

      plotmarker( 2.0, 2.0, VOPL_CIRCLE);

      /*
       *	Wait around a bit
       */
      getkey();

      vopl_windel(idx[k++]);
   }

   /*
    *	Hang around again
    */
   getkey();

   vopl_winclose(idx[5]);

   getkey();

   vopl_winraise(idx[5]);

   getkey();

   /*
    * bugger off
    */
   vexit();

   return EXIT_SUCCESS;
}
