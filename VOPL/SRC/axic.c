
/** \file axis.c
 *
 */

#include "vogle.h"
#include "vopl.h"

#include <string.h>


#define  SIDE_S1  0 /* tag the box (4 sides) */
#define  SIDE_N1  1
#define  SIDE_W1  2
#define  SIDE_E1  3


/* -------------------------------------------------- */
/* -------------------------------------------------- */

static void draw_lin_x_tics(int side);
static void draw_lin_y_tics(int side);

static void draw_log_x_tics(int side);
static void draw_log_y_tics(int side);

static void log_axis (int);
static void lin_axis (int);

static void drawaxis(char axis);

static void formatlabel(double alabel, char *form);


static axisdata *get_axisdata(char ax)
{
   axisdata	*axe = NULL;

   switch (ax)
   {
      case 'x':
      case 'X':
         axe = &plotdev->axes[XIND];
         break;
      case 'y':
      case 'Y':
         axe = &plotdev->axes[YIND];
         break;
      case 'z':
      case 'Z':
         vopl_error(VOPL_ERR_Z_AXIS,"get_axisdata");
         break;
      default:
         vopl_error(VOPL_ERR_AXIS_NB,"get_axisdata");
   }

   return axe;
}   

/*
 * drawtitle
 *
 * 	Draws a title for the graph.
 */
void drawtitle(void)
{
   if (plotdev->graphtitle == (char *)NULL)
      return;

   pushattributes();
   pushmatrix();
   pushviewport();

   ortho2(-1.0, 1.0, -1.0, 1.0);

   textsize(TEXTWIDTH * 1.5, TEXTHEIGHT * 1.5);

   move2(0.5 * (XMAX - XMIN) + XMIN, YMAX + 4 * TEXTHEIGHT);
   centertext(1);

   drawstr(plotdev->graphtitle);

   centertext(0);
   popviewport();
   popmatrix();
   popattributes();
}

/*
 * axistitle
 *
 *	Draws an axistitle
 */
void axistitle(char *title, char ax)
{
   char	**s;
   axisdata	*axe = get_axisdata(ax);
   double x,y;

   /*
      * save the goddam title somewhere....
    */
   s = &(axe->title);
   *s = savestr(*s, title);

   /* plot title */
   if (axe->title != (char *)NULL)
   {
      textsize(TEXTWIDTH, TEXTHEIGHT);
      textang(0.0);

      switch(ax)
      {
         case 'x':
         case 'X':

            if ( axe->scaling == LINEAR )
            {
               xcentertext();
               topjustify();
            }
           
            centertext(1);

            x = 0.5 * (XMAX - XMIN) + XMIN;
            y = (YMIN - 4.0 * TEXTWIDTH);
            textsize(1.3 * TEXTWIDTH, 1.3 * TEXTHEIGHT);
            move2(x, y);
            drawstr(axe->title);

            break;

         case 'y':
         case 'Y':

            if ( axe->scaling == LINEAR )
            {
               textjustify(0);
               ycentertext();
               rightjustify();
            }   

            centertext(0); 

            x = XMIN - 4.0 * TEXTWIDTH;
            y = 0.5 * (YMAX - YMIN) + YMIN;
            textsize(1.3 * TEXTWIDTH, 1.3 * TEXTHEIGHT);
            move2(x, y);
            drawstr(axe->title);

            break;
      }  
      textang(0.0); 
   }		
}


/*
* drawaxis
 *
 *	Draws an x, y or z axis, with or without annotation, tickmarks etc
 */
static void drawaxis(char axis)
{
   int	ind;

   switch (axis)
   {
      case 'x':
      case 'X':
         ind = XIND;
         break;
      case 'y':
      case 'Y':
         ind = YIND;
         break;
      case 'z':
      case 'Z':
         vopl_error(VOPL_ERR_Z_AXIS,"drawaxis");
         break;
      default:
         vopl_error(VOPL_ERR_AXIS_NB,"axisdata");
   }

   /* XAM has commented this !*/
   /*
   if (!plotdev->axes[ind].scaleset)
   { 
      plotdev->axes[ind].min = -1.0;
      plotdev->axes[ind].max =  1.0;
   }
   */
   
   if (plotdev->axes[ind].scaling == LOGARITHMIC)
   {
      log_axis(ind);
   }
   else
   {
      lin_axis(ind);
   }
}

/*
* drawaxes
 *
 *	Draws the x and y axes, with or without annotation, tickmarks etc
 */
void drawaxes(void)
{
   drawaxis('x');
   drawaxis('y');
}

/*
* lin_axis
 *
 *	draw a linear axis with its ticks
 */
static void lin_axis(int axis)
{
   pushattributes();
   pushmatrix();
   pushviewport();

   ortho2(-1.0, 1.0, -1.0, 1.0);

   clipping(0);

   switch (axis)
   {
      case XIND:

         /* "bottom" axis */
         move2(XMIN, YMIN);
         draw2(XMAX, YMIN);

         draw_lin_x_tics(SIDE_S1);

         if ( plotdev->box ) /* "top" axis */
         {
            move2(XMIN, YMAX);
            draw2(XMAX, YMAX);

            draw_lin_x_tics(SIDE_N1);
         }   

            break;

      case YIND:

         /* "left" axis */
         move2(XMIN, YMIN);
         draw2(XMIN, YMAX);

         /* draw tics */
         draw_lin_y_tics(SIDE_W1);


         if ( plotdev->box ) /* "right" axis */
         {
            move2(XMAX, YMIN);
            draw2(XMAX, YMAX);

            /* draw tics */
            draw_lin_y_tics(SIDE_E1);	   
         }   
            break;

      case ZIND:
         vopl_error(VOPL_ERR_Z_AXIS,"lin_axis");
         break;

      default:
         vopl_error(VOPL_ERR_AXIS_NB,"lin_axis");
   }

   popviewport();
   popmatrix();
   popattributes();
   clipping(1);
}

/*
* log_axis
 *
 *	Does a logarithmic axis with exponential style annotation
 */
static void log_axis(int axis)
{
   pushattributes();
   pushmatrix();
   pushviewport();

   ortho2(-1.0, 1.0, -1.0, 1.0);

   clipping(0);

   switch (axis)
   {
      case XIND:

         /* "bottom" axis */
         move2(XMIN, YMIN);
         draw2(XMAX, YMIN);

         draw_log_x_tics(SIDE_S1);

         if ( plotdev->box )/* "top" axis */
         {
            move2(XMIN, YMAX);
            draw2(XMAX, YMAX);

            draw_log_x_tics(SIDE_N1);
         }   
            break;

      case YIND:

         /* "left" axis */
         move2(XMIN, YMIN);
         draw2(XMIN, YMAX);

         draw_log_y_tics(SIDE_W1);

         if ( plotdev->box ) /* "right" axis */
         {
            move2(XMAX, YMIN);
            draw2(XMAX, YMAX);

            /* draw tics */
            draw_log_y_tics(SIDE_E1);
         }

            break;
		      
      case ZIND:
         vopl_error(VOPL_ERR_Z_AXIS,"log_axis");
         break;
		      
      default:
         vopl_error(VOPL_ERR_AXIS_NB,"log_axis");
   }

   popviewport();
   popmatrix();
   popattributes();
   clipping(1);
}



static void draw_lin_x_tics(int side)
{
   double    tinc,alabel;
   int       i,j,count;
   double    x,sx,ssx;
   double    Y_CURR  = ( side == SIDE_S1 ? YMIN : YMAX );
   axisdata  *ax;
   char	     form[50];

   ax = &plotdev->axes[XIND];

   x = ax->max - ax->min;
   count = x / ax->div + 0.5;

   tinc = (XMAX - XMIN) / x * ax->div;

   for (i = 0; i <= count; i++)
   {
      sx = i * tinc + XMIN;

      if (i < count && ax->minorticks)
      {
         for (j = 1; j <= ax->minorticks; j++)
         {
            ssx = sx + tinc * j / ax->minorticks;

            move2(ssx, Y_CURR);
            if (!(ax->minorticks & 1) && j == ax->minorticks/2)
            {
               switch( side )
               {
                  case SIDE_S1: draw2(ssx, Y_CURR + 0.75 * LINELEN); break;
                  case SIDE_N1: draw2(ssx, Y_CURR - 0.75 * LINELEN); break;
               }
            }
            else
            {
               switch( side )
               {
                  case SIDE_S1: draw2(ssx, Y_CURR + 0.5 * LINELEN); break;
                  case SIDE_N1: draw2(ssx, Y_CURR - 0.5 * LINELEN); break;
               }
            }

            if (ax->minorticks && plotdev->gridX && !(j % plotdev->gridX))
            {
               /*
                draw2(ssx, Y_CURRT);
                */
            }
         }

         if (plotdev->gridX)
         {
            if (!(i % plotdev->gridX))
            {
               /*
                setdash(0.3);
                move2(sx, YMAX);
                draw2(sx, YMIN);
                setdash(0);
                */
               double y_curr = YMIN;
               double dy     = ( YMAX-YMIN ) /  200.0 ;
               move2(sx, YMIN);

               while( y_curr < YMAX - 3*dy )
               {
                  rdraw2(0, 1*dy);
                  y_curr += 1*dy;
                  rmove2(0, 2*dy);
                  y_curr += 2*dy;
               }
            }
         }

         if (ax->ntspacing && !(i % ax->ntspacing))
         {
            move2(sx, Y_CURR);
            /*draw2(sx, Y_CURR - LINELEN);*/
            switch( side )
            {
               case SIDE_S1: draw2(sx, Y_CURR + 1.0 * LINELEN); break; /* major ticks*/
               case SIDE_N1: draw2(sx, Y_CURR - 1.0 * LINELEN); break; /* major ticks*/
            }
         }

         if (ax->annotate)
         {
            alabel = ax->min + ax->div * i;
            if (ax->format)
               sprintf(form, ax->format, alabel);
            else
               formatlabel(alabel, form);

            /*
             move2(sx, YMIN - LINELEN - 1.1 * TEXTHEIGHT / 2);
             */

            switch( side )
            {
               case SIDE_S1: move2(sx, Y_CURR -  LINELEN - 2.5 * TEXTHEIGHT / 2); break;
               case SIDE_N1: move2(sx, Y_CURR +  LINELEN - 0.4 * TEXTHEIGHT / 2); break;
            }
            drawstr(form);
         }
      }
   }
}		

static void draw_log_x_tics(int side)
{
   double 	sx, ssx, tinc, alabel;
   int 		i, j;
   char		form[21];
   axisdata	*ax;
   double    Y_CURR  = ( side == SIDE_N1 ? YMAX : YMIN );

   ax = &plotdev->axes[XIND];

   /*
      * Get the minimum exponent value.
    */
   alabel = (double)log10((double)ax->min);

   tinc = (XMAX - XMIN) / (ax->nticks);
   for (i = 0; i <= ax->nticks; i++)
   {
      sx = i * tinc + XMIN;
      /* Minor ticks */
      if (i < ax->nticks && ax->minorticks) 
      {
         for (j = 1; j < 10; j++)
         {
            ssx = sx + tinc * log10((double)j);

            move2(ssx, Y_CURR);
            /*
               if (j == 5)
             draw2(ssx, Y_CURR - 0.75 * LINELEN);
             else
             draw2(ssx, Y_CURR - 0.5 * LINELEN);
             */
            switch( side )
            {
               case SIDE_S1: draw2(ssx, Y_CURR + 0.6 * LINELEN); break;
               case SIDE_N1: draw2(ssx, Y_CURR - 0.6 * LINELEN); break;
            }

            if (ax->minorticks && plotdev->gridX && !(j % plotdev->gridX))
            {
               /*draw2(ssx, Y_CURRT);*/
            }   
         }

         if (plotdev->gridX)
         {
            if (!(i % plotdev->gridX))
            {
               /*
               move2(sx, YMIN);
                draw2(sx, YMAX);
                */
               double y_curr = YMIN;
               double dy     = ( YMAX-YMIN ) /  200.0 ;
               move2(sx, YMIN);

               while( y_curr < YMAX - 3*dy )
               {
                  rdraw2(0, 1*dy);
                  y_curr += 1*dy;
                  rmove2(0, 2*dy);
                  y_curr += 2*dy;  
               } 
            }
         }   

         if (ax->ntspacing && !(i % ax->ntspacing))
         {
            move2(sx, Y_CURR);
            /*
            draw2(sx, Y_CURR - LINELEN);
             */
            switch( side )
            {
               case SIDE_S1: draw2(sx, Y_CURR + 1.2 * LINELEN); break;
               case SIDE_N1: draw2(sx, Y_CURR - 1.2 * LINELEN); break;
            }
         }   

         if (ax->annotate)
         {
            formatlabel(alabel, form);

            textsize(0.7 * TEXTWIDTH, 0.7 * TEXTHEIGHT);
            /*move2(sx + 0.5 * TEXTWIDTH, Y_CURR - LINELEN - 0.8 * TEXTHEIGHT);*/
            switch( side )
            {
               case SIDE_S1: move2(sx + 0.5 * TEXTWIDTH, Y_CURR - LINELEN - 0.8 * TEXTHEIGHT); break;
               case SIDE_N1: move2(sx + 0.5 * TEXTWIDTH, Y_CURR - LINELEN + 1.6 * TEXTHEIGHT); break;
            }
            drawstr(form);
            rmove2(-strlength(form), 0.0);
            textsize(TEXTWIDTH, TEXTHEIGHT);
            /*rmove2(-1.3 * TEXTWIDTH, -0.9 * TEXTHEIGHT);*/
            switch( side )
            {
               case SIDE_S1: rmove2(-1.3 * TEXTWIDTH, -0.5 * TEXTHEIGHT); break;
               case SIDE_N1: rmove2(-1.3 * TEXTWIDTH, -0.6 * TEXTHEIGHT); break;
            }
            drawstr("10");
         }
         alabel += ax->div;  
      }
   }   
}


static void draw_lin_y_tics(int side)
{
   double    y,sy,ssy, tinc,alabel;
   int      i,j,count;
   double    X_CURR  = ( side == SIDE_W1 ? XMIN : XMAX );
   axisdata	*ax;
   char		form[50];

   ax = &plotdev->axes[YIND];

   y = ax->max - ax->min;
   tinc = (YMAX - YMIN) / y * ax->div;
   count = y / ax->div + 0.5;

   for (i = 0; i <= count; i++)
   {
      sy = i * tinc + YMIN;

      if (i < count && ax->minorticks)
      {
         for (j = 1; j <= ax->minorticks; j++)
         {
            ssy = sy + tinc * j / ax->minorticks;

            move2(X_CURR, ssy);

            if (!(ax->minorticks & 1) && j == ax->minorticks/2)
            {
               switch(side)
               {
                  case SIDE_W1: draw2(X_CURR + 0.70 * LINELEN, ssy); break;
                  case SIDE_E1: draw2(X_CURR - 0.75 * LINELEN, ssy); break;
               }
               /* draw2(X_CURR - 0.75 * LINELEN, ssy); */
            }			
            else
            {
               switch(side)
               {
                  case SIDE_W1: draw2(X_CURR + 0.4 * LINELEN, ssy); break;
                  case SIDE_E1: draw2(X_CURR - 0.5 * LINELEN, ssy); break;
               }
               /*draw2(X_CURR - 0.5 * LINELEN, ssy);*/
            }			

            if (ax->minorticks && plotdev->gridY && !(j % plotdev->gridY))
            {
               /*draw2(X_CURRT, ssy);*/
            }  	
         }

         if (plotdev->gridY)
         { 
            if (!(i % plotdev->gridY))
            {
               /*
               setdash(0.3);
                move2(XMIN, sy);
                draw2(XMAX, sy);
                setdash(0);
                */
               double x_curr = XMIN;
               double dx     = ( XMAX-XMIN ) /  200.0 ;
               move2(XMIN,sy);

               while( x_curr < XMAX - 3*dx )
               {
                  rdraw2(1*dx, 0);
                  x_curr += 1*dx;
                  rmove2(2*dx, 0);
                  x_curr += 2*dx;  
               } 
            }
         }			

         if (ax->ntspacing && !(i % ax->ntspacing))
         {
            move2(X_CURR, sy);
            switch(side)
            {
               case SIDE_W1: draw2(X_CURR + 1.0 * LINELEN, sy); break;
               case SIDE_E1: draw2(X_CURR - 1.0 * LINELEN, sy); break;
            }
            /*draw2(X_CURR - LINELEN, sy);*/
         }

         if (ax->annotate)
         {
            alabel = ax->min + ax->div * i;
            if (ax->format)
               sprintf(form, ax->format, alabel);
            else
               formatlabel(alabel, form);

            /*
               move2(XMIN - strlength(form) - 2 * LINELEN, sy - TEXTHEIGHT / 2);
             */
            switch(side)
            {
               case SIDE_W1: move2(X_CURR - 5.8 * LINELEN, sy - TEXTHEIGHT / 2); break;
               case SIDE_E1: move2(X_CURR + 1.4 * LINELEN, sy - TEXTHEIGHT / 2); break;
            }

            drawstr(form);
         }
      }
   }		
}

static void draw_log_y_tics(int side)
{
   double 	sy, ssy, tinc, alabel;
   int 		i, j;
   char		form[21];
   axisdata	*ax;
   double       X_CURR  = ( side == SIDE_W1 ? XMIN : XMAX );

   ax = &plotdev->axes[YIND];

   /*
      * Get the minimum exponent value.
    */
   alabel = (double)log10((double)ax->min);

   centertext(0);

   tinc = (YMAX - YMIN) / (ax->nticks);

   for (i = 0; i <= ax->nticks; i++)
   {
      sy = i * tinc + YMIN;

      if (i < ax->nticks && ax->minorticks)
      {
         for (j = 1; j < 10; j++)
         {
            ssy = sy + tinc * log10((double)j);

            move2(X_CURR, ssy);
            /*
               if (j == 5)
             draw2(X_CURR - 0.75 * LINELEN, ssy);
             else
             draw2(X_CURR - 0.5  * LINELEN, ssy);
             */
            switch(side)
            {
               case SIDE_W1: draw2(X_CURR + 0.6 * LINELEN, ssy); break;
               case SIDE_E1: draw2(X_CURR - 0.6 * LINELEN, ssy); break;
            }

            if (ax->minorticks && plotdev->gridY && !(j % plotdev->gridY))
            {
               /*draw2(X_CURRT, ssy);*/
            }   
         }

         if (plotdev->gridY) 
         {
            if (!(i % plotdev->gridY))
            {
               setdash(0.3);
               move2(XMIN, sy);
               draw2(XMAX, sy);
               setdash(0);
            }
         }   

         if (ax->ntspacing && !(i % ax->ntspacing))
         {
            move2(X_CURR, sy);
            /*
            draw2(X_CURR - LINELEN, sy);
             */
            switch(side)
            {
               case SIDE_W1: draw2(X_CURR + LINELEN, sy); break;
               case SIDE_E1: draw2(X_CURR - LINELEN, sy); break;
            }
         }

         if (ax->annotate)
         {
            formatlabel(alabel, form);
            textsize(0.7 * TEXTWIDTH, 0.7 * TEXTHEIGHT);
            /*
               move2(X_CURR - strlength(form) -1.5 * LINELEN, sy + 0.5 * TEXTHEIGHT);
             */
            switch(side)
            {
               case SIDE_W1: move2(X_CURR   - strlength(form)   - 1.5 * LINELEN, sy + 0.5 * TEXTHEIGHT); break;
               case SIDE_E1: move2(X_CURR /*- strlength(form)*/ + 2.5 * LINELEN, sy + 0.5 * TEXTHEIGHT); break;
            }
            drawstr(form);
            rmove2(-strlength(form), -0.75 * TEXTHEIGHT);
            textsize(TEXTWIDTH, TEXTHEIGHT);
            rmove2(-1.1 * TEXTWIDTH, 0.0);
            drawstr("10");
         }

         alabel += ax->div;
      } 
   }    
} 


/*
* formatlabel
 *
 *	chose a "nice" format for an axis label, awful at the moment.
 */
static void formatlabel(double alabel, char *form)
{
   int	i;

   if (ABS(alabel) < 1.0e-9)
      alabel = 0.0;

   if ((ABS(alabel) > 999999.0) || (ABS(alabel) < 0.000001))
      sprintf(form, "%.4g", alabel);
   else 
      sprintf(form, "%f", alabel);

   /*
      * if there is no 'e' in there or there is a dot then
    * eat trailing zeros.....
    */
   if ( strchr(form, 'e') == (char *)NULL &&
        strchr(form, 'E') == (char *)NULL &&
        strchr(form, '.') != (char *)NULL  )
   {
      i = strlen(form) - 1;

      while (i >= 0 && form[i] == '0') 
         i--;

      if ( form[i] == '.' )
         i--;

      form[++i] = '\0';
      if ( form[0] == '\0' )
      {
         form[0] = '0';
         form[1] = '\0';
      }
   }
}

