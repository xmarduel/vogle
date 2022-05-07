
#include <stdio.h>
#include <math.h>
#include "vogle.h"
#include "vopl.h"

#define NN 200

static void generate_points(double *x, double *y, int N, double t)
{
   int i;
   double k = 0.0;
   double dk = 2 * PI / N;

   for (i = 0; i != N; i++)
   {
      x[i] = 1+k;
      y[i] = 2+sin(k+t/50.0);
      k = k + dk;
   }
}

/*
 *	A very simple test program for vopl.
 *
 * 	This one draws a graph of y = sin(x) 0 <= x <= 2 * PI
 */
int main()
{
   
   int   t;
   char  thetitle[50];

   /*const int N	=  200;*/
   const int NB_ITER = 1000;
   
   double	x[NN], y[NN];

   /*
    *	As we are now about to do some graphics we initialise VOGLE
    *	and clear to BLACK
    */


   prefsize(800, 400);

   vopl_winopen("X11",NULL);
   expandviewport();

   backbuffer();
   
   for (t=0; t<NB_ITER; t++)
   {
      color(BLACK);
      clear();

      /*
       *	Generate the points
       */
      generate_points(x, y, NN, t);
      

      /*annotate("%d", 'x');*/
      scaling(LINEAR,'x');
      scaling(LINEAR,'y');
      /*gridspacing(2,1);*/
      withbox(1);


      /*
       * Now set the color to GREEN
       */
      color(GREEN);

      /*
       * Draw the default set of axes (in GREEN)
       */
      axistitle("(X)", 'x');
      axistitle("(Y)", 'y');

      drawaxes();
      /*
       * Set color to RED
       */
      color(RED);

     /*
      *	Draw the Graph
      */
      plot2(x, y, NN);
      
      /*
       * title
       */
      sprintf(thetitle,"SIN(%5d t)",t);
      graphtitle(thetitle);
      drawtitle();

      /*
       * flush !
       */
      swapbuffers();

      /*
       * the first time ...
       */
      if (t==0)
      {
         /*vgetevent(&vev,0);*/
         getkey();
      }

   }
   
   /*
    *	Hang around again
    */
   getkey();

   /*
    * bugger off
    */
   vexit();

   return EXIT_SUCCESS;
}
