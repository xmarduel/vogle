/*
ppm driver for vogle; Version 1.0, John S. Urban, Feb 1997

This driver makes  pbmplus-readable color (P3 and P6  format)  pixmap   files.

I usually use the popular netplus/pbmplus(1) packages to    convert  the    P3
and   P6    files    to    other formats to use in HTML documents or to import
into many other products.

The xv(1) package can display, rescale and convert P3 and P6 files too.

Unfortunately,  ppm  only  supports  single-frame  files.  For  P3  files  you
need   to   use  voutput(3[cf]}   to   keep   changing the output file, or use
csplit(1) on the P3 files to split them apart.

A very different but much more productive tack is taken  with  the  binary  P6
format.   Writing  to device p6 writes to the  standard  input  of  a  command
called   'p6to'   that  MUST BE IN YOUR SEARCH PATH. So something like:

#!/bin/sh
# this is the p6to script used by the VOGLE p6 driver
exec cat >$$_p6.ppm

would do, but this is intended primarily for you to use with scripts that call
pbmplus commands; giving VOGLE the appearance of being able to write in dozens
of popular bitmap formats. This way, you can write  GIF  or  PCX  or  JPEG  or
whatever kind of bitmap you can generate with P6 input files.

In fact, I only have the pbmplus package handy on a  SunOS machine, but I  use
vogle  the  most on a Cray. No problem. I   use   a remote   shell   to get to
the command from another machine and I  generate   GIF   images    this    way
without    even noticing   all   the   forks   and   network connections being
made -- really!

One of the easiest ways is to make a script with a case statement in  it  that
triggers  from an environmental variable.  The details vary with the scripting
language (Bourne, ksh, csh, perl, tcl/tk) but the  basic  idea  is  the  same.
Here's a basic Bourne shell script:

#!/bin/sh
# this is the p6to script used by the VOGLE p6 driver
case "$P6TO" in
GIF)remsh sirius ppmtogif >$$.gif # gif file
# remsh starts a remote shell. Use rsh on some machines
# sirius seems like a good fake name for a remote Sun. But siriusly folks ...
;;
TIFF)remsh sirius pnmtotiff >$$.tiff # tiff file
;;
XBM)remsh sirius 'ppmtopgm|pgmtopbm|pbmtoxbm' > $$.xbm # greyscale X11 bitmap
;;
*)
cat > $$_p6.ppm
# if you need more control of the filename, consider doing a putenv in your
# program of a variable name that is then used to build the file name.
;;
esac
exit

The popen(); remote shell and IO redirect all work so nicely together you just
figure that anyone not using Unix just never heard of it.

Please pass any upgrades or comments back to me ( urban@cray.com) if you get a
chance.

*-----------------------------------------------------------------*
| Author: John S. Urban                                           |
*-----------------------------------*-----------------------------*
| Cray Research                     | urban@cray.com              | CURRENTLY PREFERRED
| Silicon Graphics, Incorporated    | urban@sgi.com               |
*-----------------------------------*-----------------------------*
================================================================================
   USAGE NOTES ON THE PPM DRIVER:

See the PBM driver, from which I derived this.

I have used this driver on UNICOS(Cray) and NeXT but it should  be  reasonably
portable.

In this version of the driver, only the color number (from 0 to 255) is stored
in  the pixmap. This color number is then used to generate RGB values when the
page is WRITTEN. This reduces the amount of storage needed, but means that  if
you  draw with pen N and then change the color of pen N and draw with it again
that everything in the printed image that used pen N  will  all  be  the  same
color  (the last one defined for pen N before printing). To get a "true color"
behavior would require saving RGB values for each point,  which  would  triple
the storage requirements but would otherwise be easy to do.

Line thickness is supported with filled rectangular  polygons  when  the  line
thickness  is  greater  than  1.  Square  ends  are  used  that go only to the
endpoints unless line thickness is greater than  5,  in  which  case  complete
circles  are  added  to the endpoints.  If very short polylines are drawn with
the circles on the ends slight errors can occur.

If you have a pre-ANSI C compiler  you  will  have  to  remove  the  PROTOTYPE
statements and change a few procedure headers back to the old K&R style.
================================================================================

References: 1) Fundamentals of Interactive Computer Graphics, Foley & Van Dam, Addison Wesley Publishing Company, 1982
            2) ppm - portable bitmap file format, 27 September 1991, Copyright (C) 1989, 1991 by Jef Poskanzer.

 Copyright (C) 1996, 1997 John S. Urban

 This  software  is  public  domain  and  may be  used  for  any  purpose
 commercial or otherwise.  It is offered  without any guarantee as to its
 suitability  for any purpose or as to the sanity of its  writers.  We do
 ask that the  source is passed on to anyone  that  requests  a copy, and
 that people who get copies don't go round claiming they wrote it.

 PS:
 If you feel guilty about using this for commercial purposes and want to
 send me money go ahead. If you can't find me give something to charity
 and forget about it.

================================================================================

The  macro  SET_PIXEL will set a given pixel in the graphics arrays :

#define SET_BYTE_R(x,y,intensity) ( *(graphics_r + (x) * ppm->Y_SIZE + (y) ) = (intensity) )
#define SET_BYTE_G(x,y,intensity) ( *(graphics_g + (x) * ppm->Y_SIZE + (y) ) = (intensity) )
#define SET_BYTE_B(x,y,intensity) ( *(graphics_b + (x) * ppm->Y_SIZE + (y) ) = (intensity) )

#define SET_PIXEL(x,y) (SET_BYTE_R((x),(y),(cur_r)),SET_BYTE_G((x),(y),(cur_g)),SET_BYTE_B((x),(y),(cur_b)))

*/

#define SET_PIXEL(x,y) \
	if (x >= 0 && x <ppm->X_SIZE && y >= 0 && y < ppm->Y_SIZE)\
		 ( *(ppm->graphics_rgb + (x) * ppm->Y_SIZE + (y) ) = (char)(ppm->color) )

/******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "vogle.h"

#define P3  2
#define P6  3

#define UNDRAWN 0
#define DRAWN   1

#define byte unsigned char

extern FILE     *_voutfile();

typedef struct
{
        int     r, g, b;
} ColorTable;


typedef struct
{
	unsigned long	localid;	/* Must be first in structure */
	int	id;
	char *fp_name; /* filename to be produced */
	FILE     *fp;
	FILE     *fpP6;
	int drawn; /* flag whether page is blank or not */
	int rasters; /* line thickness */
	int lastx, lasty;     /* position of last draw */
	ColorTable *coltab;
	int driver;			/* P3 or P6 or PPM */
	int color;
	int X_SIZE, Y_SIZE; /* size of graphics array */
	byte *graphics_rgb; /* the graphics data */
} Ppmdev;

Ppmdev	*ppm;



#define MAX(x,y)  ((x) > (y) ? (x) : (y))
#define MIN(x,y)  ((x) < (y) ? (x) : (y))
#define ABS(x)    ((x) < 0 ? -(x) : (x))

#define PROTOTYPE




/* Functions */
static void PPM_MEMSET(void); /* set graphics array to all zero */
static int PPM_color(int col);
static int PPM_mapcolor(int indx, int r, int g, int b);
static void PPM_SOLID_FILL(int n, int x[], int y[]);
static void PPM_DRAW_LINE(int x,int y);
static int PPM_fill(int n, int x[], int y[]);
static void P3_print_graphics(Ppmdev *ppm);
static void P6_print_graphics(Ppmdev *ppm);
static void PPM_PRINT(Ppmdev *ppm);
static void *P3_winopen(const char *dev, const char *title, int id);
static void *P6_winopen(const char *dev, const char *title, int id);
static int PPM_setlw(int w);
static int PPM_clear(void);
static int PPM_font(const char *font);
static int PPM_string(const char *s);
static int PPM_char(char c);
static int PPM_backb(void *p, int old, int bw, int bh);
void _PPM_devcpy(char *);
static int PPM_YINTERCEPT(int yscan, int x1, int y1, int x2, int y2, int *xintercept, int *yprev);

static unsigned long    localppmid = 0L;

/******************************************************************************/
static void PPM_MEMSET(void) /* set graphics array to all zero */
{
        /*--- IF YOU HAVE IT, MEMSET IS PROBABLY FASTER*/
        memset(ppm->graphics_rgb, (char)ppm->color, sizeof(byte) * ppm->Y_SIZE * ppm->X_SIZE);
        /*---*/
        /*
        for ( i=0; i< (ppm->X_SIZE * ppm->Y_SIZE); i++)
        {
           *(ppm->graphics_rgb + i) = (char)ppm->color;
        }
        */
}
/******************************************************************************/
static int PPM_color(int col) /* change the current color */
{
        ppm->color = ABS(col % 256) ;
        return(1);
}
/******************************************************************************/
static int PPM_mapcolor(int indx, int r, int g, int b) /* set values in pseudo color map.  */
{
        if (indx < 256 && indx >= 0)
        {
                ppm->coltab[indx].r = ABS(r % 256) ;
                ppm->coltab[indx].g = ABS(g % 256) ;
                ppm->coltab[indx].b = ABS(b % 256) ;
        }
        return(1);
}
/******************************************************************************/
static void *PPM_winopen(const char *dev, const char *title, int id)
{
	int prefx, prefy, prefxs, prefys;
	int i;
	Ppmdev *p;
                
	ppm = (Ppmdev *)vallocate(sizeof(Ppmdev));
	p = ppm;
	memset(ppm, 0, sizeof(Ppmdev));
	ppm->id = id;

	ppm->localid = localppmid++;

	ppm->fp = _voutfile();

	if ( title != NULL && strcmp(title , "") != 0 ) 
		ppm->fp_name = strdup(title);
	else
	   ppm->fp_name = strdup("unknown.ppm");

	/* ---DETERMINE SIZE OF GRAPHICS PIXMAP */
        /* see if a size was user-specified using the prefsize procedure */
        getprefposandsize(&prefx, &prefy, &prefxs, &prefys);
        if (prefxs != -1 )
        {
                if (prefys <= 0 )
                {
                        fprintf(stderr,"*PPM_init* y size of %d set to 400\n",prefys);
                        prefys = 400;
                }
                else
                {
                        vdevice.dev.sizeSy = prefys;
                }
                if (prefxs <= 0 )
                {
                        fprintf(stderr,"*PPM_init* y size of %d set to 600\n",prefys);
                        prefxs = 600;
                }
                else
                {
                        vdevice.dev.sizeSx = prefxs;
                }
        }
        else
        {
                /* nice default value */
                prefx = 0;
                prefy = 0;
                vdevice.dev.sizeSy = 400;
                vdevice.dev.sizeSx = 600;
        }
        vdevice.dev.sizeX = vdevice.dev.sizeY = MIN(vdevice.dev.sizeSy,vdevice.dev.sizeSx);
        ppm->X_SIZE=vdevice.dev.sizeSx;
        ppm->Y_SIZE=vdevice.dev.sizeSy;
        ppm->graphics_rgb = (byte *) malloc( ppm->X_SIZE * ppm->Y_SIZE * sizeof(byte) ); /* the graphics array */
        PPM_MEMSET(); /* set the graphics array to 0 */

        vdevice.dev.depth = 1;

        /* Cause scaling to be 0 to maxX maxY: prefx, vdevice.sizeSx+prefx, prefy, vdevice.sizeSy+prefy */

        ppm->lastx = -1111111;
        ppm->lasty = -1111111;

        ppm->drawn = UNDRAWN;

        ppm->coltab = (ColorTable *)vallocate(256 * sizeof(ColorTable));

        PPM_mapcolor(0, 0, 0, 0);
        PPM_mapcolor(1, 255, 0, 0);
        PPM_mapcolor(2, 0, 255, 0);
        PPM_mapcolor(3, 255, 255, 0);
        PPM_mapcolor(4, 0, 0, 255);
        PPM_mapcolor(5, 255, 0, 255);
        PPM_mapcolor(6, 0, 255, 255);
        PPM_mapcolor(7, 255, 255, 255);

        for(i=8; i<256; i++)
        {
           PPM_mapcolor(i, 255, 255, 255);
        }

	ppm->driver = P6;
        
   return((void *)ppm);
}

static void *
P3_winopen(const char *dev, const char *title, int id)
{
   Ppmdev *p = (Ppmdev *)PPM_winopen(dev, title, id);
   p->driver = P3;
   return((void *)p);
}

static void *
P6_winopen(const char *dev, const char *title, int id)
{
   Ppmdev *p = (Ppmdev *)PPM_winopen(dev, title, id);
   p->driver = P6;
   return((void *)p);
}

static int
PPM_winset(void *p)
{
	ppm = (Ppmdev *)p;

	return(ppm->id);
}

/* 
 * PPM_winraise
 * 	Raise a window
 */
static int
PPM_winraise(void *p)
{
	/* Nothing */
	return(1);
}

/*
 * PPM_windel
 *
 */
static int
PPM_windel(void *p)
{
	Ppmdev	*pp = (Ppmdev *)p;

	if ( pp == NULL )
		return 1;

	PPM_PRINT(pp);
        
	if (pp->fp != NULL)
	{
		fclose(pp->fp);
	}

	free(pp->graphics_rgb);
	free(pp->coltab);
	free(pp->fp_name);
	
	free(pp);
	pp = NULL;

	return(1);
}

/* 
 * PPM_winclose
 * 	Close a window
 */
static int
PPM_winclose(void *p)
{
	PPM_windel(p);
	return(1);
}

/******************************************************************************/
static void PPM_DRAW_LINE(int x,int y) /* draws a line across a graphics array */
{
        int runcount;
        int dx,dy;
        int xinc,yinc;
        int xplot,yplot;

        SET_PIXEL(ppm->lastx,ppm->lasty); /* move to initial spot */

        runcount=0;

        dx = abs(ppm->lastx-x);

        if (x > ppm->lastx)  
           xinc=  1;
        else
        if (x == ppm->lastx) 
           xinc=  0;
        else
           xinc= -1;

        dy = abs(ppm->lasty-y);

        if (y > ppm->lasty)  
           yinc=  1;
        else
        if (y == ppm->lasty) 
           yinc=  0;
        else
           yinc= -1;

        xplot = ppm->lastx;
        yplot = ppm->lasty;

        if (dx>dy)
        {
                /* iterate x */
                while (xplot != x)
                {
                        xplot += xinc;
                        runcount += dy;
                        if (runcount >= (dx-runcount))
                        {
                                yplot += yinc;
                                runcount -= dx;
                        }
                        SET_PIXEL(xplot,yplot);
                }
        }
        else
        {
                /* iterate y */
                while (yplot != y)
                {
                        yplot += yinc;
                        runcount += dx;
                        if (runcount >= (dy-runcount))
                        {
                                xplot += xinc;
                                runcount -= dy;
                        }
                        SET_PIXEL(xplot,yplot);
                }
        }
       
        ppm->lastx = xplot;
        ppm->lasty = yplot;

	return;
}

/******************************************************************************/
static void PPM_ENDCAP_CIRCLE(int x, int y) /* Draw a circle on thick line segment end point */
{
        /* there are more efficient ways to do this */
        /* circle precision */
#define NSEGS	15
        static int nsegs= NSEGS;
        float cx, cy, dx, dy, angle, cosine, sine ;
        /* array to place circle points on */
        int cxras[NSEGS], cyras[NSEGS];
        int i;

        angle = 2.0 * PI / nsegs;
        cosine = cos((double)angle);
        sine = sin((double)angle);

        /* first point on circle */
        cxras[0] = cx =  x + ppm->rasters/2.0;
        cyras[0] = cy = y;
        for (i = 1; i < nsegs; i++)
        {
                dx = cx - x;
                dy = cy - y;
                cxras[i] = ( cx = x + dx * cosine - dy * sine) ;
                cyras[i] = ( cy = y + dx * sine   + dy * cosine) ;
        }
        PPM_SOLID_FILL(nsegs,cxras,cyras);
}
/******************************************************************************/
static int PPM_fill(int n, int x[], int y[]) /* "fill" a polygon */
{
        int     i;

        /* update current position if needed */
        ppm->lastx=x[0];
        ppm->lasty=y[0];

        for (i = 1; i < n; i++)
        {
                PPM_DRAW_LINE(x[i],y[i]); /* draw outline across graphics array */
        }
        if ( x[n-1] != x[0] || y[n-1] != y[0] ) /* close the polygon if it is not closed */
                PPM_DRAW_LINE(x[0],y[0]);

        PPM_SOLID_FILL(n, x, y);

        /* update current position */
        ppm->lastx = vdevice.cpVx = x[n - 1];
        ppm->lasty = vdevice.cpVy = y[n - 1];

        ppm->drawn = DRAWN;

	return(1);
}
/******************************************************************************/
static int PPM_draw(int x, int y) /* print the commands to draw a line from the current graphics position to (x, y).  */
{
        int     holdx, holdy;
        int xwide[4], ywide[4];
        float cosa, sina;
        double angle;

        if (ppm->lastx != vdevice.cpVx || ppm->lasty != vdevice.cpVy)
        {
             ppm->lastx=vdevice.cpVx;
             ppm->lasty=vdevice.cpVy;
        }

        if ( ppm->rasters <= 1)
        {
           PPM_DRAW_LINE(x,y);
        }
        else
        {
           /* thick lines are made from filled polygon(s) */
           /* add a circle to ends of really thick lines */
           if( ppm->rasters >= 6)
           {
              holdx=ppm->lastx;
              holdy=ppm->lasty;
              PPM_ENDCAP_CIRCLE(ppm->lastx,ppm->lasty);
              PPM_ENDCAP_CIRCLE(x,y);
              ppm->lastx=holdx;
              ppm->lasty=holdy;
           }
           
           angle=atan2((double)(y-ppm->lasty),(double)(x-ppm->lastx)) + PI/2.0;
           cosa=(ppm->rasters/2.0)*cos(angle);
           sina=(ppm->rasters/2.0)*sin(angle);
           xwide[0]=x+cosa;
           xwide[1]=ppm->lastx+cosa;
           xwide[2]=ppm->lastx-cosa;
           xwide[3]=x-cosa;

           ywide[0]=y+sina;
           ywide[1]=ppm->lasty+sina;
           ywide[2]=ppm->lasty-sina;
           ywide[3]=y-sina;

           PPM_SOLID_FILL(4,xwide,ywide);
        }
        ppm->drawn = DRAWN;

	return(1);
}

static int
PPM_pnt(int x, int y)
{
	SET_PIXEL(x, y);
	ppm->lastx = x;
	ppm->lasty = y;
	ppm->drawn = DRAWN;

	return(1);
}

/*******************************************************************************/
static void P3_print_graphics(Ppmdev *ppm) /* print_graphics -- print the graphics bit array as a ppm P3 file*/
{
   int x; /* current x BYTE */
   int y; /* current y location */
   int index, pix;
   int X_SIZE = ppm->X_SIZE;
   int Y_SIZE = ppm->Y_SIZE;
        
   (void) fprintf(ppm->fp,"P3\n"); /* magic number of a clear text PPM file */
   (void) fprintf(ppm->fp,"# CREATOR: VOGLE ppm driver; version 1.0 1997/02/02\n"); /* ppm P3 file can contain comment lines*/
   (void) fprintf(ppm->fp,"# Uncopyright (C) 19970202, John S. Urban\n");
   (void) fprintf(ppm->fp,"# csplit multiframe files: csplit -f P3 -k $.p3 '%%^P3%%' '/^P3/' '{999}'\n");
   (void) fprintf(ppm->fp,"%d %d\n",ppm->X_SIZE,ppm->Y_SIZE); /* size of bitmap */
   (void) fprintf(ppm->fp,"255\n"); /* maximum value of a color intensity*/

   /* notice going from bottom to top because putting out in a right handed coordinate system, was assuming left-handed */
   for (y = (Y_SIZE-1); y >= 0; y--)
   {
      /* Loop for each byte in the array */
      for ( x = 0; x < X_SIZE ; x++)
      {
         index = ppm->Y_SIZE * x + y;
         pix   = (int)*(ppm->graphics_rgb + index);
         /* The manual says a P3 ppm file should not be wider than 70 characters */
         (void) fprintf(ppm->fp,"%d %d %d\n",
                        ppm->coltab[pix].r,
                        ppm->coltab[pix].g,
                        ppm->coltab[pix].b);
      }
   } /* end of writing a column */
   (void) fprintf(ppm->fp,"\n");
   ppm->drawn = UNDRAWN;

   fflush(ppm->fp);
}
/*******************************************************************************/
static void P6_print_graphics(Ppmdev *ppm) /* print_graphics -- print the graphics bit array as a ppm P6 file*/
{
   int x; /* current x BYTE */
   int y; /* current y location */
   int index,pix;
   char* xstring;
   int i;
   int X_SIZE = ppm->X_SIZE;
   int Y_SIZE = ppm->Y_SIZE;

   if ( strstr(ppm->fp_name, ".ppm") == NULL )
       ppm->fpP6 = (FILE *)popen("p6to", "w");
   else
       ppm->fpP6 = (FILE *)fopen(ppm->fp_name, "w");
   
   if (!ppm->fpP6)
   {
      fprintf(stderr, "Couldn't open pipe to lpr command.\n");
      exit(1);
   }


   (void) fprintf(ppm->fpP6,"P6\n"); /* magic number of a ppm file */
   (void) fprintf(ppm->fpP6,"# CREATOR: VOGLE ppm driver; version 2.0 1997/06/06\n"); /*ppm P6 file can contain comment lines*/
   (void) fprintf(ppm->fpP6,"# Uncopyright (C) 19970202, John S. Urban\n");
   (void) fprintf(ppm->fpP6,"%d %d\n",ppm->X_SIZE,ppm->Y_SIZE); /* size of bitmap */
   (void) fprintf(ppm->fpP6,"255\n"); /* maximum value of a color intensity*/

   xstring = (void*)malloc( 3*X_SIZE );
   
	/* notice going from bottom to top because putting out in a right handed coordinate system, was assuming left-handed */
   for (y = Y_SIZE-1; y >= 0; y--)
   {
      /* Loop for each byte in the array */
      i=0;

      for ( x = 0; x < X_SIZE ; x++)
      {
         index = ppm->Y_SIZE * x + y;
         pix   = *(ppm->graphics_rgb + index);
         /*
         putc((char)ppm->coltab[pix].r, ppm->fpP6);
         putc((char)ppm->coltab[pix].g, ppm->fpP6);
         putc((char)ppm->coltab[pix].b, ppm->fpP6);
         */
         xstring[i++] = (char)ppm->coltab[pix].r;
         xstring[i++] = (char)ppm->coltab[pix].g;
         xstring[i++] = (char)ppm->coltab[pix].b;
      }
      fwrite(xstring, 1, 3*X_SIZE, ppm->fpP6);
   }

   fflush(ppm->fpP6);
                  
   if (ppm->fpP6 != NULL)
   {
      if ( strstr(ppm->fp_name, ".ppm") == NULL )
      {
         pclose(ppm->fpP6); /* Was fclose - bernie -process doesn't wait(2) otehrwise */
		   ppm->fpP6 = NULL;
      }
      else
      {
         fclose(ppm->fpP6);
         ppm->fpP6 = NULL;
      }
   }

   ppm->drawn = UNDRAWN;

   free(xstring);
}
/******************************************************************************/
static void PPM_PRINT(Ppmdev *ppm) /* exit from vogle printing the command to flush the buffer.  */
{
        if ( ppm->drawn )
        {
                switch(ppm->driver)
                {
                case P3:
                        P3_print_graphics(ppm);
                        break;
                case P6:
                        P6_print_graphics(ppm);
                        break;
                default:
                        fprintf(stderr, "ppm driver: UNKNOWN DRIVER NAME\n");
                        P3_print_graphics(ppm);
                }
        }
}
/******************************************************************************/
static int PPM_setlw(int w) /* Set the line width */
{

        if (w == 0)
                w = 1;
        else if (w == 1)
                w = 2;

        ppm->rasters = MAX(1,w);

	return(1);
}
/******************************************************************************/
static int PPM_clear(void) /* flush current page and clear graphics array */
{
        PPM_PRINT(ppm);
        PPM_MEMSET();

	return(1);
}
/******************************************************************************/
static int PPM_font(const char *font) /* load in large or small */
{
        fprintf(stderr, "E-R-R-O-R: NO HARDWARE FONT\n");
        if (strcmp(font, "small") == 0) {
                vdevice.hwidth = 97.01; /* Size in plotter resolution units */
                vdevice.hheight = vdevice.hwidth * 2.0;
        } else if (strcmp(font, "large") == 0) {
                vdevice.hwidth = 145.5;
                vdevice.hheight = vdevice.hwidth * 2.0;
        } else
                return(-1);

        return(1);
}
/******************************************************************************/
static int PPM_string(const char *s) /* output a string.  */
{
        if (ppm->lastx != vdevice.cpVx || ppm->lasty != vdevice.cpVy){
                ppm->lastx=vdevice.cpVx;
                ppm->lasty=vdevice.cpVy;
        }

        fputs(s, ppm->fp);

        ppm->lastx = ppm->lasty = -1111111; /* undefine current position because used hardware text ?*/
        ppm->drawn = DRAWN;

	return(-1);
}
/******************************************************************************/
static int PPM_char(char c) /* output a character */
{
  char  s[2];
  s[0] = c; s[1]='\0';
  return(PPM_string(s));
}

static void
PPM_sync(void)
{
	fflush(ppm->fp);
}

/*
 * Do nothing functions.
 */
static int
PPM_backb(void *p, int old, int bw, int bh)
{
	return(-1);
}

static int
PPM_frontb(void)
{
	return(-1);
}

static int
PPM_swapb(void)
{
	return(-1);
}

static int
PPM_checkkey(void)
{
	return(-1);
}

static int
PPM_getkey(void)
{
	return(-1);

}

static int
PPM_locator(int *x, int *y)
{
	return(-1);
}

static unsigned long
PPM_getevent(Vevent *vev, int block)
{
	vev->type = -1;
	vev->data = 0;
	vev->w = 0;
	return(ppm->localid);
}

/******************************************************************************/
static DevEntry PPMdev = {
                "p6",		/* name of device */
                "large",	/* name of large font */
                "small",	/* name of small font */
		8, 0, 0, 0, 0,
                PPM_backb,	/* Set drawing in back buffer */
                PPM_char,	/* Draw a hardware character */
                PPM_checkkey,	/* Check if a key was hit */
                PPM_clear,	/* Clear the screen to current color */
                PPM_color,	/* Set current color */
                PPM_draw,	/* Draw a line */
                PPM_fill,	/* Fill a polygon */
                PPM_font,	/* Set hardware font */
                PPM_frontb,	/* Set drawing in front buffer */
                PPM_getkey,	/* Wait for and get the next key hit */
                PPM_getevent,	/* Get the next event */
                PPM_winopen,	/* Initialize the device */
                PPM_winset,	/* set device */
                PPM_winclose,	/* Close window */
                PPM_winraise,	/* Raise window */
                PPM_windel,	/* Delete window */
                PPM_locator,	/* Get mouse x and y */
                PPM_mapcolor,	/* Set color indices */
                PPM_pnt,	/* A dot */
                PPM_setlw,	/* Set line width */
                PPM_string,	/* Draw a hardware string */
                PPM_swapb,	/* Swap front and back buffers */
                PPM_sync	/* Syncronize the display */
};
/******************************************************************************/
void
_PPM_devcpy(char *name)
{
	vdevice.dev = PPMdev;
	vdevice.dev.devname = name;
	if (strcmp(name, "p3") == 0)
		vdevice.dev.Vwinopen = P3_winopen;
	if (strcmp(name, "p6") == 0)
		vdevice.dev.Vwinopen = P6_winopen;
	if (strcmp(name, "ppm") == 0)
		vdevice.dev.Vwinopen = P6_winopen;
}
/*******************************************************************************/
static int PPM_YINTERCEPT(int yscan, int x1, int y1, int x2, int y2, int *xintercept,int *yprev)
/*
Determine if scan line intercepts the line segment. If it does, return the x intercept.
*/
{
        int deltay, yprevious;
        float   t;
        yprevious = *yprev; /* the value we need to use in this pass */
        *yprev = y1;        /* store the value for the next call to (probably) use */
        deltay = y2 - y1;
        if ( deltay == 0 ){
                /* horizontal lines do not contribute to scan line intercepts */
                *yprev=yprevious;
                return(0);
        }
        t = (float)(yscan - y1) / deltay;
        if (t > 0.0 && t <= 1.0) {
                /* scan line and line segment intersect but not at leading vertex */
                *xintercept = x1 + t*(x2 - x1) + 0.5;
                return (1);
        } else if ( t == 0.0 ){
                /* scan line and line segment intersect at leading vertex */
                *xintercept = x1 + t*(x2 - x1) + 0.5;
                if(yprevious <= y1 && y2 <= y1 ){
                   /* local maximum */
                   return (1);
                } else if(yprevious >= y1 && y2 >= y1 ){
                   /* local minimum */
                   return (1);
                } else{
                   /* ignore duplicate at vertex that is not a local maximum or minimum */
                   return (0);
                }
        }
        /* scan line and line segment did not intersect */
        return (0);
}
/*******************************************************************************/
static void PPM_SOLID_FILL(int n, int x[], int y[]) /* fill polygon of n points drawn by polyline <x,y>.  */
{
        int i, j, sorted, yhorizontal, xint, tmp, xmin, xmax, ymax, ymin, xi[MAXVERTS], yprev;

        if ( n > MAXVERTS) {
           fprintf(stderr,"*PPM_SOLID_FILL* more than %d vertices in a polygon\n",MAXVERTS);
           return;
        }

        /* find clip range */
        ymin = ymax = y[0];
        xmin = xmax = y[0];
        for (i = 0; i < n; i++) {
                ymax = MAX(ymax, y[i]);
                ymin = MIN(ymin, y[i]);
                xmax = MAX(xmax, x[i]);
                xmin = MIN(xmin, x[i]);
        }
        /* ensure scan lines are generated that do not cause out-of-bound problems in the y direction */
        ymin=MAX(ymin,0);
        ymax=MIN(ymax,ppm->Y_SIZE);

        /* For each y value, get a list of X intersections... */
        yhorizontal = ymax ;
        while (yhorizontal >= ymin) {
                j = 0;
                yprev = y[n-1];
                for (i = 0; i < n-1; i++)
                        if (PPM_YINTERCEPT(yhorizontal, x[i], y[i], x[i+1], y[i+1], &xint, &yprev))
                                        xi[j++] = xint;
                /* Last one. */
                if (PPM_YINTERCEPT(yhorizontal, x[n-1], y[n-1], x[0], y[0], &xint, &yprev))
                                xi[j++] = xint;

                /* odd pairs means something went wrong in figuring out whether to count vertices or not */
                if( 2 * (j/2) != j){
                   fprintf(stderr,"*PPM_SOLID_FILL* Internal error: odd number of intersection points (%d) \n",j);
                }

                /* Sort the X intersections... */
                sorted = 0;
                while (!sorted) {
                        sorted = 1;
                        for (i = 0; i < j-1; i++)
                                if (xi[i] > xi[i+1]) {
                                        tmp = xi[i];
                                        xi[i] = xi[i+1];
                                        xi[i+1] = tmp;
                                        sorted = 0;
                                }
                }

                /* Draw the horizontal lines */
                /* should make sure within X clipping range */
                for (i = 0; i < j-1; i += 2) {
                        ppm->lastx=MAX(0,MIN(xi[i],ppm->X_SIZE));
                        ppm->lasty=yhorizontal;
                        PPM_DRAW_LINE(MAX(0,MIN(xi[i+1],ppm->X_SIZE)), yhorizontal);
                }
                yhorizontal -= 1;
        }
}
/*******************************************************************************/

