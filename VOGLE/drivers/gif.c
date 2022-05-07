
/*
 
 gif driver for vogle; Version 1.0, Xavier Marduel, Feb 2014

 */

#define SET_PIXEL(x,y) \
if (x >= 0 && x <gif->X_SIZE && y >= 0 && y < gif->Y_SIZE)\
( *(gif->graphics_rgb + (x) * gif->Y_SIZE + (y) ) = (char)(gif->color) )

/******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "vogle.h"

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
	int drawn; /* flag whether page is blank or not */
	int rasters; /* line thickness */
	int lastx, lasty;     /* position of last draw */
	ColorTable *coltab;
	int color;
	int X_SIZE, Y_SIZE; /* size of graphics array */
	byte *graphics_rgb; /* the graphics data */
} Gifdev;

Gifdev	*gif;



#define MAX(x,y)  ((x) > (y) ? (x) : (y))
#define MIN(x,y)  ((x) < (y) ? (x) : (y))
#define ABS(x)    ((x) < 0 ? -(x) : (x))

#define PROTOTYPE




/* Functions */
static void GIF_MEMSET(void); /* set graphics array to all zero */
static int GIF_color(int col);
static int GIF_mapcolor(int indx, int r, int g, int b);
static void GIF_SOLID_FILL(int n, int x[], int y[]);
static void GIF_DRAW_LINE(int x,int y);
static int GIF_fill(int n, int x[], int y[]);
static void GIF_print_graphics(Gifdev *gif);
static void GIF_PRINT(Gifdev *gif);
static void *GIF_winopen(const char *dev, const char *title, int id);
static int GIF_setlw(int w);
static int GIF_clear(void);
static int GIF_font(const char *font);
static int GIF_string(const char *s);
static int GIF_char(char c);
static int GIF_backb(void *p, int old, int bw, int bh);
void _GIF_devcpy(char *);
static int GIF_YINTERCEPT(int yscan, int x1, int y1, int x2, int y2, int *xintercept, int *yprev);

static unsigned long    localgifid = 0L;

/******************************************************************************/
static void GIF_MEMSET(void) /* set graphics array to all zero */
{
	memset(gif->graphics_rgb, (char)gif->color, sizeof(byte) * gif->Y_SIZE * gif->X_SIZE);
}
/******************************************************************************/
static int GIF_color(int col) /* change the current color */
{
	gif->color = ABS(col % 256) ;
	return(1);
}
/******************************************************************************/
static int GIF_mapcolor(int indx, int r, int g, int b) /* set values in pseudo color map.  */
{
	if (indx < 256 && indx >= 0)
	{
		gif->coltab[indx].r = ABS(r % 256) ;
		gif->coltab[indx].g = ABS(g % 256) ;
		gif->coltab[indx].b = ABS(b % 256) ;
	}
	return(1);
}
/******************************************************************************/
static void *GIF_winopen(const char *dev, const char *title, int id)
{
	int prefx, prefy, prefxs, prefys;
	int i;
	Gifdev *p;
	
	gif = (Gifdev *)vallocate(sizeof(Gifdev));
	p = gif;
	memset(gif, 0, sizeof(Gifdev));
	gif->id = id;
	
	gif->localid = localgifid++;
	
	gif->fp = _voutfile();
	
	if ( title != NULL && strcmp(title , "") != 0 ) 
		gif->fp_name = strdup(title);
	else
	   gif->fp_name = strdup("unknown.gif");
	
	/* ---DETERMINE SIZE OF GRAPHICS PIXMAP */
	/* see if a size was user-specified using the prefsize procedure */
	getprefposandsize(&prefx, &prefy, &prefxs, &prefys);
	if (prefxs != -1 )
	{
		if (prefys <= 0 )
		{
			fprintf(stderr,"*GIF_init* y size of %d set to 400\n",prefys);
			prefys = 400;
		}
		else
		{
			vdevice.dev.sizeSy = prefys;
		}
		if (prefxs <= 0 )
		{
			fprintf(stderr,"*GIF_init* y size of %d set to 600\n",prefys);
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
	gif->X_SIZE=vdevice.dev.sizeSx;
	gif->Y_SIZE=vdevice.dev.sizeSy;
	gif->graphics_rgb = (byte *) malloc( gif->X_SIZE * gif->Y_SIZE * sizeof(byte) ); /* the graphics array */
	GIF_MEMSET(); /* set the graphics array to 0 */
	
	vdevice.dev.depth = 1;
	
	/* Cause scaling to be 0 to maxX maxY: prefx, vdevice.sizeSx+prefx, prefy, vdevice.sizeSy+prefy */
	
	gif->lastx = -1111111;
	gif->lasty = -1111111;
	
	gif->drawn = UNDRAWN;
	
	gif->coltab = (ColorTable *)vallocate(256 * sizeof(ColorTable));
	
	GIF_mapcolor(0, 0, 0, 0);
	GIF_mapcolor(1, 255, 0, 0);
	GIF_mapcolor(2, 0, 255, 0);
	GIF_mapcolor(3, 255, 255, 0);
	GIF_mapcolor(4, 0, 0, 255);
	GIF_mapcolor(5, 255, 0, 255);
	GIF_mapcolor(6, 0, 255, 255);
	GIF_mapcolor(7, 255, 255, 255);
	
	for(i=8; i<256; i++)
	{
		GIF_mapcolor(i, 255, 255, 255);
	}
	
   return((void *)gif);
}


static int
GIF_winset(void *p)
{
	gif = (Gifdev *)p;
	
	return(gif->id);
}

/* 
 * GIF_winraise
 * 	Raise a window
 */
static int
GIF_winraise(void *p)
{
	/* Nothing */
	return(1);
}

/*
 * GIF_windel
 *
 */
static int
GIF_windel(void *p)
{
	Gifdev	*pp = (Gifdev *)p;
	
	if ( pp == NULL )
		return 1;
	
	GIF_PRINT(pp);
	
	if (pp->fp != NULL)
	{
		fclose(pp->fp);
	}
	
	free(pp->graphics_rgb);
	free(pp->coltab);
	free(pp->fp_name);
	
	free(pp);
	pp = NULL;
	
	return 1;
}

/* 
 * GIF_winclose
 * 	Close a window
 */
static int
GIF_winclose(void *p)
{
	GIF_windel(p);
	
	return(1);
}

/******************************************************************************/
static void GIF_DRAW_LINE(int x,int y) /* draws a line across a graphics array */
{
	int runcount;
	int dx,dy;
	int xinc,yinc;
	int xplot,yplot;
	
	SET_PIXEL(gif->lastx,gif->lasty); /* move to initial spot */
	
	runcount=0;
	
	dx = abs(gif->lastx-x);
	
	if (x > gif->lastx)  
      xinc=  1;
	else
   if (x == gif->lastx) 
      xinc=  0;
	else
      xinc = -1;
   
	dy = abs(gif->lasty-y);
	
	if (y > gif->lasty)  
      yinc=  1;
	else
   if (y == gif->lasty) 
      yinc=  0;
	else
      yinc= -1;
	
	xplot = gif->lastx;
	yplot = gif->lasty;
	
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
	
	gif->lastx = xplot;
	gif->lasty = yplot;
	
	return;
}

/******************************************************************************/
static void GIF_ENDCAP_CIRCLE(int x, int y) /* Draw a circle on thick line segment end point */
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
	cxras[0] = cx =  x + gif->rasters/2.0;
	cyras[0] = cy = y;
	for (i = 1; i < nsegs; i++)
	{
		dx = cx - x;
		dy = cy - y;
		cxras[i] = ( cx = x + dx * cosine - dy * sine) ;
		cyras[i] = ( cy = y + dx * sine   + dy * cosine) ;
	}
	GIF_SOLID_FILL(nsegs,cxras,cyras);
}
/******************************************************************************/
static int GIF_fill(int n, int x[], int y[]) /* "fill" a polygon */
{
	int     i;
	
	/* update current position if needed */
	gif->lastx=x[0];
	gif->lasty=y[0];
	
	for (i = 1; i < n; i++)
	{
		GIF_DRAW_LINE(x[i],y[i]); /* draw outline across graphics array */
	}
	if ( x[n-1] != x[0] || y[n-1] != y[0] ) /* close the polygon if it is not closed */
		GIF_DRAW_LINE(x[0],y[0]);
	
	GIF_SOLID_FILL(n, x, y);
	
	/* update current position */
	gif->lastx = vdevice.cpVx = x[n - 1];
	gif->lasty = vdevice.cpVy = y[n - 1];
	
	gif->drawn = DRAWN;
	
	return(1);
}
/******************************************************************************/
static int GIF_draw(int x, int y) /* print the commands to draw a line from the current graphics position to (x, y).  */
{
	int     holdx, holdy;
	int xwide[4], ywide[4];
	float cosa, sina;
	double angle;
	
	if (gif->lastx != vdevice.cpVx || gif->lasty != vdevice.cpVy)
	{
		gif->lastx=vdevice.cpVx;
		gif->lasty=vdevice.cpVy;
	}
	
	if ( gif->rasters <= 1)
	{
		GIF_DRAW_LINE(x,y);
	}
	else
	{
		/* thick lines are made from filled polygon(s) */
		/* add a circle to ends of really thick lines */
		if( gif->rasters >= 6)
		{
			holdx=gif->lastx;
			holdy=gif->lasty;
			GIF_ENDCAP_CIRCLE(gif->lastx,gif->lasty);
			GIF_ENDCAP_CIRCLE(x,y);
			gif->lastx=holdx;
			gif->lasty=holdy;
		}
		
		angle=atan2((double)(y-gif->lasty),(double)(x-gif->lastx)) + PI/2.0;
		cosa=(gif->rasters/2.0)*cos(angle);
		sina=(gif->rasters/2.0)*sin(angle);
		xwide[0]=x+cosa;
		xwide[1]=gif->lastx+cosa;
		xwide[2]=gif->lastx-cosa;
		xwide[3]=x-cosa;
		
		ywide[0]=y+sina;
		ywide[1]=gif->lasty+sina;
		ywide[2]=gif->lasty-sina;
		ywide[3]=y-sina;
		
		GIF_SOLID_FILL(4,xwide,ywide);
	}
	gif->drawn = DRAWN;
	
	return(1);
}

static int
GIF_pnt(int x, int y)
{
	SET_PIXEL(x, y);
	gif->lastx = x;
	gif->lasty = y;
	gif->drawn = DRAWN;
	
	return(1);
}

static void write_gif(Gifdev *gif);

/*******************************************************************************/
static void GIF_print_graphics(Gifdev *gif) /* print_graphics -- print the graphics bit array as a gif file*/
{
	gif->fp = (FILE *)fopen(gif->fp_name, "wb");
	
   if (!gif->fp)
   {
      fprintf(stderr, "Couldn't open file.\n");
      exit(1);
   }
	
	write_gif(gif);
	
	fclose(gif->fp);
	gif->fp = NULL;
	
   gif->drawn = UNDRAWN;
}

/******************************************************************************/
static void GIF_PRINT(Gifdev *gif) /* exit from vogle printing the command to flush the buffer.  */
{
	if ( gif->drawn )
	{
		GIF_print_graphics(gif);
	}
}
/******************************************************************************/
static int GIF_setlw(int w) /* Set the line width */
{
	
	if (w == 0)
		w = 1;
	else if (w == 1)
		w = 2;
	
	gif->rasters = MAX(1,w);
	
	return(1);
}
/******************************************************************************/
static int GIF_clear(void) /* flush current page and clear graphics array */
{
	GIF_PRINT(gif);
	GIF_MEMSET();
	
	return(1);
}
/******************************************************************************/
static int GIF_font(const char *font) /* load in large or small */
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
static int GIF_string(const char *s) /* output a string.  */
{
	if (gif->lastx != vdevice.cpVx || gif->lasty != vdevice.cpVy){
		gif->lastx=vdevice.cpVx;
		gif->lasty=vdevice.cpVy;
	}
	
	fputs(s, gif->fp);
	
	gif->lastx = gif->lasty = -1111111; /* undefine current position because used hardware text ?*/
	gif->drawn = DRAWN;
	
	return(-1);
}
/******************************************************************************/
static int GIF_char(char c) /* output a character */
{
	char  s[2];
	s[0] = c; s[1]='\0';
	return(GIF_string(s));
}

static void
GIF_sync(void)
{
	fflush(gif->fp);
}

/*
 * Do nothing functions.
 */
static int
GIF_backb(void *p, int old, int bw, int bh)
{
	return(-1);
}

static int
GIF_frontb(void)
{
	return(-1);
}

static int
GIF_swapb(void)
{
	return(-1);
}

static int
GIF_checkkey(void)
{
	return(-1);
}

static int
GIF_getkey(void)
{
	return(-1);
	
}

static int
GIF_locator(int *x, int *y)
{
	return(-1);
}

static unsigned long
GIF_getevent(Vevent *vev, int block)
{
	vev->type = -1;
	vev->data = 0;
	vev->w = 0;
	return(gif->localid);
}

/******************************************************************************/
static DevEntry GIFdev = {
	"gif",		/* name of device */
	"large",	/* name of large font */
	"small",	/* name of small font */
	8, 0, 0, 0, 0,
	GIF_backb,	/* Set drawing in back buffer */
	GIF_char,	/* Draw a hardware character */
	GIF_checkkey,	/* Check if a key was hit */
	GIF_clear,	/* Clear the screen to current color */
	GIF_color,	/* Set current color */
	GIF_draw,	/* Draw a line */
	GIF_fill,	/* Fill a polygon */
	GIF_font,	/* Set hardware font */
	GIF_frontb,	/* Set drawing in front buffer */
	GIF_getkey,	/* Wait for and get the next key hit */
	GIF_getevent,	/* Get the next event */
	GIF_winopen,	/* Initialize the device */
	GIF_winset,	/* set device */
	GIF_winclose,	/* Close window */
	GIF_winraise,	/* Raise window */
	GIF_windel,	/* Delete window */
	GIF_locator,	/* Get mouse x and y */
	GIF_mapcolor,	/* Set color indices */
	GIF_pnt,	/* A dot */
	GIF_setlw,	/* Set line width */
	GIF_string,	/* Draw a hardware string */
	GIF_swapb,	/* Swap front and back buffers */
	GIF_sync	/* Syncronize the display */
};
/******************************************************************************/
void
_GIF_devcpy(char *name)
{
	vdevice.dev = GIFdev;
	vdevice.dev.devname = name;
	
	vdevice.dev.Vwinopen = GIF_winopen;
}
/*******************************************************************************/
static int GIF_YINTERCEPT(int yscan, int x1, int y1, int x2, int y2, int *xintercept,int *yprev)
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
static void GIF_SOLID_FILL(int n, int x[], int y[]) /* fill polygon of n points drawn by polyline <x,y>.  */
{
	int i, j, sorted, yhorizontal, xint, tmp, xmin, xmax, ymax, ymin, xi[MAXVERTS], yprev;
	
	if ( n > MAXVERTS) {
		fprintf(stderr,"*GIF_SOLID_FILL* more than %d vertices in a polygon\n",MAXVERTS);
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
	ymax=MIN(ymax,gif->Y_SIZE);
	
	/* For each y value, get a list of X intersections... */
	yhorizontal = ymax ;
	while (yhorizontal >= ymin) {
		j = 0;
		yprev = y[n-1];
		for (i = 0; i < n-1; i++)
			if (GIF_YINTERCEPT(yhorizontal, x[i], y[i], x[i+1], y[i+1], &xint, &yprev))
				xi[j++] = xint;
		/* Last one. */
		if (GIF_YINTERCEPT(yhorizontal, x[n-1], y[n-1], x[0], y[0], &xint, &yprev))
			xi[j++] = xint;
		
		/* odd pairs means something went wrong in figuring out whether to count vertices or not */
		if( 2 * (j/2) != j){
			fprintf(stderr,"*GIF_SOLID_FILL* Internal error: odd number of intersection points (%d) \n",j);
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
                        gif->lastx=MAX(0,MIN(xi[i],gif->X_SIZE));
                        gif->lasty=yhorizontal;
                        GIF_DRAW_LINE(MAX(0,MIN(xi[i+1],gif->X_SIZE)), yhorizontal);
                }
                yhorizontal -= 1;
        }
}
/*******************************************************************************/



#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef unsigned char uchar;
typedef signed char schar;
typedef unsigned int uint;
typedef unsigned short int ushort;
typedef unsigned long int ulong;

typedef signed char int8;
typedef unsigned char uint8;

typedef signed short int int16;
typedef unsigned short int uint16;

typedef signed long int int32;
typedef unsigned long int uint32;


#pragma pack (push, 1)

typedef struct
{
  char          Id[6];
  uint16        Width;
  uint16        Height;
  uint8         Info;
  uint8         BackGroundColor;
  uint8         Res;
} tGifScreenDescriptor;

typedef struct
{
  uint8         Id;
  uint16        LeftOffset;
  uint16        TopOffset;
  uint16        Width;
  uint16        Height;
  uint8         Info;
} tGifImageDescriptor;

#pragma pack (pop)

typedef enum
{
  GIF_ERROR_OK,
  GIF_ERROR_BAD_ARGUMENT,
  GIF_ERROR_MALFORMED_DATA,
  GIF_ERROR_NO_MEMORY,
  GIF_ERROR_UNSUPPORTED_FORMAT
} tGifError;

extern tGifError Gif_Encode (const uint8* pImage,
                             int TightPixelPacking,
                             uint16 Width,
                             uint16 Height,
                             uint16 ColorCount,
                             uint8 BackGroundColor,
                             tGifScreenDescriptor* pScreenDescriptor,
                             tGifImageDescriptor* pImageDescriptor,
                             uint8** ppCompressedImage,
                             size_t* pCompressedSize);

/*
  GIF structure:

  (GIF signature + Screen Descriptor) (Global Color Map)?
  (
    ((Image Descriptor) (Local Color Map)? (Raster Data))
   |
    (GIF Extension Block)
  )+
  (GIF Terminator)?

  ()  - occurs once
  ()? - optional: may not occur or may occur just once
  ()+ - occurs at least once
    | - logical or
*/

#define GIF_HASH_TABLE_SHIFT            12
#define GIF_HASH_TABLE_SIZE             (1 << GIF_HASH_TABLE_SHIFT)

#define GIF_EMPTY_HASH_LIST             0xFFFFu
#define GIF_NO_HASH_TABLE_ITEM          0xFFFFu

#define GIF_MK_HASH_ITEM(_PREV_INDEX_,_CH_,_NEXT_INDEX_) \
  (((uint32)(_PREV_INDEX_) & 0xFFF) | \
   ((uint32)((_CH_) & 0xFF) << 12) | \
   ((uint32)((_NEXT_INDEX_) & 0xFFF) << 20))

#define GIF_GET_HASH_NEXT_INDEX(_ITEM_) \
  (((_ITEM_) >> 20) & 0xFFF)

#define GIF_SET_HASH_NEXT_INDEX(_ITEM_,_NEXT_INDEX_) \
  _ITEM_ = (_ITEM_ & 0x000FFFFFUL) | \
            ((uint32)((_NEXT_INDEX_) & 0xFFF) << 20);

#define GIF_GET_HASH_PREV_INDEX(_ITEM_) \
  ((_ITEM_) & 0xFFF)

#define GIF_GET_HASH_CH(_ITEM_) \
  (((_ITEM_) >> 12) & 0xFF)

#define GIF_MK_HASH(_PREV_INDEX_,_CH_) \
  (((GIF_MK_HASH_ITEM(_PREV_INDEX_,_CH_,0) * 648055UL) >> 8) & 0xFFF)

#define GIF_CMP_HASH_ITEMS(_ITEM1_,_ITEM2_) \
  (((_ITEM1_) ^ (_ITEM2_)) & 0xFFFFFUL)

/*
  Encoding hash table organization:

  ...
  aIndices[100] = GIF_EMPTY_HASH_LIST - there're no items with hash value of 100

                          +--+
                          v  |
  aIndices[101] -> aItems[0]-+ - there's only one item with hash value of 101,
                                 this item is at index 0. Because it's the last
                                 (and the only) item on the list of items with
                                 hash value of 101, it points to itself.

                                         +--+
                                         v  |
  aIndices[102] -> aItems[87] -> aItems[13]-+ - there're two items with hash
                                                value of 102. The last one
                                                points to itself.
  ...

  aItems[x] layout:
  Bits: 31  ...  20 19  ...  12 11  ...  0
        \_________/ \_________/ \________/
             |           |           |
             |           |           +- previous index (GIF-specific data)
             |           +- character (GIF-specific data)
             |      \____________________/
             |                 |
             |                 +- 20-bit key
             +- next index - link to the next item with the same hash value
*/
typedef struct
{
  uint16 aIndices[GIF_HASH_TABLE_SIZE];
  uint32 aItems[GIF_HASH_TABLE_SIZE];
  uint16 ItemCount;
} tGifEncodeHashTable;

typedef struct
{
  uint8 Stream[256];
  uint8 Shift;
  uint8 Remainder;
} tGifBitstream;

typedef struct
{
  uint16 CodeClear;
  uint16 CodeEOI;
  uint16 CodeCnt;
  uint16 CurCodeSize;
} tGifCodesInfo;

typedef struct
{
  uint8* pImage;
  size_t Size;
  size_t Allocated;
} tGifCompressed;

typedef struct
{
  tGifBitstream         Bitstream;
  tGifCodesInfo         CodesInfo;
  tGifCompressed        Compressed;
} tGifEncodeContext;

typedef struct
{
  const uint8*          pGif;
  size_t                GifSize;
  size_t                Ofs;
  const uint8*          pBytestream;
  uint8                 BytestreamIndex;
  uint8                 BytestreamShift;
  uint8                 BytestreamRemainder;
  tGifCodesInfo         CodesInfo;
} tGifDecodeContext;

static const char Gif87aId[] = "GIF87a";
static const char Gif89aId[] = "GIF89a";

static
void HashTable_Insert (tGifEncodeHashTable* pHashTable,
                       uint16 PrevIndex,
                       uint8 Ch);

static
void HashTable_Init (tGifEncodeHashTable* pHashTable,
                     uint16 CodeClear)
{
  uint i;

  // Generic hash table code:

  // Set the number of items in the hash table to 0:
  pHashTable->ItemCount = 0;

  // Empty the hash-to-list map too:
  for (i = 0; i < GIF_HASH_TABLE_SIZE; i++)
  {
    pHashTable->aIndices[i] = GIF_EMPTY_HASH_LIST;
  }

  // GIF-specific hash table code:

  // Allocate and initialize the first items that
  // describe the color codes:
  for (i = 0; i < CodeClear; i++)
  {
    HashTable_Insert (pHashTable, GIF_NO_HASH_TABLE_ITEM, (uint8)i);
  }

  // Reserve 2 more items in the hash table for 2 codes:
  // CodeClear and CodeEOI. The items for these codes
  // can't be allocated for data when encoding because of
  // the special meaning of the codes associated with them,
  // hence skipping them:
  pHashTable->ItemCount += 2;
}

static
void HashTable_Insert (tGifEncodeHashTable* pHashTable,
                       uint16 PrevIndex,
                       uint8 Ch)
{
  uint16 Hash = GIF_MK_HASH (PrevIndex, Ch);
  uint16 NextIndex;

  // The last item in the list of items sharing the same hash value
  // will point to itself:
  if (pHashTable->aIndices[Hash] != GIF_EMPTY_HASH_LIST)
    NextIndex = pHashTable->aIndices[Hash];
  else
    NextIndex = pHashTable->ItemCount;

  // Allocate and push the new item onto the list:
  pHashTable->aIndices[Hash] = pHashTable->ItemCount;
  pHashTable->aItems[pHashTable->ItemCount] =
    GIF_MK_HASH_ITEM (PrevIndex, Ch, NextIndex);

  pHashTable->ItemCount++;
}

static
uint16 HashTable_Search (tGifEncodeHashTable* pHashTable,
                         uint16 PrevIndex,
                         uint8 Ch)
{
  uint16 Hash = GIF_MK_HASH (PrevIndex, Ch);

  if (pHashTable->aIndices[Hash] != GIF_EMPTY_HASH_LIST)
  {
    uint32 Item = GIF_MK_HASH_ITEM (PrevIndex, Ch, 0);
    uint16 i, j = pHashTable->aIndices[Hash];
    do
    {
      i = j;
      if (!GIF_CMP_HASH_ITEMS (pHashTable->aItems[i], Item))
        return i;
      // No match, try the next item on the list:
      j = GIF_GET_HASH_NEXT_INDEX (pHashTable->aItems[i]);
      // If the index of the next item is the same as
      // of the current item, the last item has been
      // processed and we're finished w/o a match:
    } while (j != i);
  }

  return GIF_NO_HASH_TABLE_ITEM;
}

static
tGifError BitStream_StoreBytes (tGifEncodeContext* pContext,
                                const uint8* pBytes,
                                size_t ByteCount)
{
  tGifError err = GIF_ERROR_OK;

  while (ByteCount)
  {
    size_t CanCopy;

    CanCopy = pContext->Compressed.Allocated - pContext->Compressed.Size;

    if (CanCopy)
    {
      memcpy (pContext->Compressed.pImage + pContext->Compressed.Size,
              pBytes,
              CanCopy);

      pContext->Compressed.Size += CanCopy;
      pBytes += CanCopy;
      ByteCount -= CanCopy;
    }

    if (ByteCount)
    {
      void* pNewLocation;

      pNewLocation = realloc (pContext->Compressed.pImage,
                              pContext->Compressed.Allocated + ByteCount);

      if (pNewLocation == NULL)
      {
        err = GIF_ERROR_NO_MEMORY;
        break;
      }

      pContext->Compressed.pImage = pNewLocation;
      pContext->Compressed.Allocated += ByteCount;
    }
  }

  return err;
}

static
tGifError BitStream_Store (tGifEncodeContext* pContext)
{
  tGifError err = GIF_ERROR_OK;

  if (pContext->Bitstream.Stream[0])
  {
    err = BitStream_StoreBytes (pContext,
                                pContext->Bitstream.Stream,
                                pContext->Bitstream.Stream[0] + 1);

    pContext->Bitstream.Stream[0] = 0;
  }

  return err;
}

static
tGifError BitStream_PutByte (tGifEncodeContext* pContext,
                             uint8 Data)
{
  tGifError err = GIF_ERROR_OK;

  pContext->Bitstream.Stream[++pContext->Bitstream.Stream[0]] = Data;

  if (pContext->Bitstream.Stream[0] >= 255)
    err = BitStream_Store (pContext);

  return err;
}

static
tGifError BitStream_Flush (tGifEncodeContext* pContext)
{
  tGifError err;

  if (pContext->Bitstream.Shift)
  {
    err = BitStream_PutByte (pContext, pContext->Bitstream.Remainder);
    if (err != GIF_ERROR_OK)
      return err;

    pContext->Bitstream.Shift = 0;
  }

  err = BitStream_Store (pContext);

  return err;
}

static
tGifError BitStream_PutCode (tGifEncodeContext* pContext,
                             uint16 Code)
{
  tGifError err = GIF_ERROR_OK;
  uint Cnt;

  if (!pContext->Bitstream.Shift)
    pContext->Bitstream.Remainder = 0;

  pContext->Bitstream.Remainder |=
    (Code << pContext->Bitstream.Shift) & 0xFF;

  Cnt = 8 - pContext->Bitstream.Shift;

  Code >>= Cnt; 

  while (Cnt < pContext->CodesInfo.CurCodeSize)
  {
    err = BitStream_PutByte (pContext, pContext->Bitstream.Remainder);
    if (err != GIF_ERROR_OK)
      return err;
    pContext->Bitstream.Remainder = Code & 0xFF;
    Code >>= 8;
    Cnt += 8;
  }

  pContext->Bitstream.Shift += pContext->CodesInfo.CurCodeSize;
  pContext->Bitstream.Shift &= 7;

  if (!pContext->Bitstream.Shift)
    err = BitStream_PutByte (pContext, pContext->Bitstream.Remainder);

  return err;
}

/*
  tbd:
  - option to find out compressed size
  - option to choose whether to save thru callback or keep in memory
  - option to specify custom malloc/free/realloc
*/
tGifError Gif_Encode (const uint8* pImage,
                      int TightPixelPacking,
                      uint16 Width,
                      uint16 Height,
                      uint16 ColorCount,
                      uint8 BackGroundColor,
                      tGifScreenDescriptor* pScreenDescriptor,
                      tGifImageDescriptor* pImageDescriptor,
                      uint8** ppCompressedImage,
                      size_t* pCompressedSize)
{
  tGifError err = GIF_ERROR_BAD_ARGUMENT;
  uint32 Area;
  uint BitsPerPixel;
  uint8 CodeSize;
  tGifEncodeHashTable* pHashTable = NULL;
  tGifEncodeContext Context;
  uint16 PrevIndex;
  uint8 Shift;

  memset (&Context, 0, sizeof(Context));
  Context.Compressed.pImage = NULL;

  if ((pImage == NULL) ||
      (!Width) || (!Height) ||
      (ColorCount < 2) || (ColorCount > 256) ||
      (ColorCount & (ColorCount - 1)) ||
//      (BackGroundColor >= ColorCount) ||
      (pScreenDescriptor == NULL) ||
      (pImageDescriptor == NULL) ||
      (ppCompressedImage == NULL) ||
      (pCompressedSize == NULL))
  {
    goto lerr;
  }

  if (ColorCount == 256)
    TightPixelPacking = 0;

  Area = (uint32)Width * Height;

  if ((!Area) || (Area / Width != Height))
  {
    goto lerr;
  }

  pHashTable = malloc (sizeof (*pHashTable));

  if (pHashTable == NULL)
  {
    err = GIF_ERROR_NO_MEMORY;
    goto lerr;
  }

  BitsPerPixel = 0;
  while ((1 << BitsPerPixel) < ColorCount)
    BitsPerPixel++;

  // GIF screen descriptor:

  memcpy (pScreenDescriptor->Id, Gif87aId, sizeof(pScreenDescriptor->Id));
  pScreenDescriptor->Width = Width;
  pScreenDescriptor->Height = Height;
  pScreenDescriptor->BackGroundColor = BackGroundColor;
  pScreenDescriptor->Res = 0;
  pScreenDescriptor->Info =
    0x80 |                      // Global color map is present
    ((BitsPerPixel-1)<<4) |     // Color resolution
    (BitsPerPixel-1);           // Bits per pixel

  // GIF global color map isn't used or stored here

  // GIF image descriptor:

  pImageDescriptor->Id = 0x2C;
  pImageDescriptor->LeftOffset =
  pImageDescriptor->TopOffset = 0;
  pImageDescriptor->Width = Width;
  pImageDescriptor->Height = Height;
  pImageDescriptor->Info = 0;   // Global color map & sequential order

  // GIF raster data stream initial code size:
  CodeSize = BitsPerPixel + (BitsPerPixel == 1); // extra 1 is neded :(

  err = BitStream_StoreBytes (&Context, &CodeSize, 1);
  if (err != GIF_ERROR_OK)
    goto lerr;

  // Setting up the needed work variables:

  Context.CodesInfo.CodeClear = 1 << CodeSize;
  Context.CodesInfo.CodeEOI = Context.CodesInfo.CodeClear + 1;
  Context.CodesInfo.CodeCnt = Context.CodesInfo.CodeClear + 2;
  Context.CodesInfo.CurCodeSize = CodeSize + 1;

  HashTable_Init (pHashTable, Context.CodesInfo.CodeClear);

  // Let's start encoding, first code must be CodeClear:

  err = BitStream_PutCode (&Context, Context.CodesInfo.CodeClear);
  if (err != GIF_ERROR_OK)
    goto lerr;

  // Begin with an empty sequence of pixels:
  PrevIndex = GIF_NO_HASH_TABLE_ITEM;

  Shift = 0;

  // Process all pixels of the image:
  do
  {
    uint16 i;
    uint8 Data, Pixel;

    // Get the next pixel and append to the current sequence of pixels:
    if (!TightPixelPacking)
    {
      Pixel = (*pImage++) & (ColorCount - 1);
    }
    else
    {
      if (!Shift)
      {
        Data = *pImage++ & 0xFF;
        Shift = 8;
      }

      if (Shift >= BitsPerPixel)
      {
        Pixel = Data & (ColorCount - 1);
        Data >>= BitsPerPixel;
        Shift -= BitsPerPixel;
      }
      else
      {
        Pixel = Data;
        Data = *pImage++ & 0xFF;
        Pixel |= (Data << Shift) & (ColorCount - 1);
        Data >>= BitsPerPixel - Shift;
        Shift = 8 - (BitsPerPixel - Shift);
      }
    }

    Area--;

    // Find a sequence of pixels like the current one in the hash table:
    i = HashTable_Search (pHashTable, PrevIndex, Pixel);

    if (i != GIF_NO_HASH_TABLE_ITEM)
    {
      // Found, keep growing the current sequence of pixels until it's unique:
      PrevIndex = i;
    }
    else // elseof if (i != GIF_NO_HASH_TABLE_ITEM)
    {
      // The last pixel made the current pixel sequence unique, so
      // we must store the code representing the entire subsequence before
      // the last pixel and possibly store the sequence in the hash table:
      err = BitStream_PutCode (&Context, PrevIndex);
      if (err != GIF_ERROR_OK)
        goto lerr;

      // Grow the bit size of the code if needed:
      if ((Context.CodesInfo.CodeCnt == (1 << Context.CodesInfo.CurCodeSize)) &&
          (Context.CodesInfo.CurCodeSize < GIF_HASH_TABLE_SHIFT))
        Context.CodesInfo.CurCodeSize++;

#if 0
      if (Context.CodesInfo.CodeCnt >= GIF_HASH_TABLE_SIZE - 1)
      {
        // If the hash table is full, reinitialize it and start over:
        err = BitStream_PutCode (&Context, Context.CodesInfo.CodeClear);
        if (err != GIF_ERROR_OK)
          goto lerr;

        HashTable_Init (pHashTable, Context.CodesInfo.CodeClear);

        Context.CodesInfo.CodeCnt = Context.CodesInfo.CodeClear + 2;
        Context.CodesInfo.CurCodeSize = CodeSize + 1;
      }
      else // elseof if (Context.CodesInfo.CodeCnt >= GIF_HASH_TABLE_SIZE - 1)
      {
        // If there's enough space for another item in the hash table,
        // store the current pixel sequence there:
        HashTable_Insert (pHashTable, PrevIndex, Pixel);
        Context.CodesInfo.CodeCnt++;
      }
#else
      // If there's enough space for another item in the hash table,
      // store the current pixel sequence there:
      if (Context.CodesInfo.CodeCnt < GIF_HASH_TABLE_SIZE)
        HashTable_Insert (pHashTable, PrevIndex, Pixel);
// worse compression may be because of the table reinit
// that doesn't give us a chance to use the last table element
// or store extra code(s) $$$:
      if (++Context.CodesInfo.CodeCnt >= GIF_HASH_TABLE_SIZE)
      {
        // If the hash table is full, reinitialize it and start over:
        err = BitStream_PutCode (&Context, Context.CodesInfo.CodeClear);
        if (err != GIF_ERROR_OK)
          goto lerr;

        HashTable_Init (pHashTable, Context.CodesInfo.CodeClear);

        Context.CodesInfo.CodeCnt = Context.CodesInfo.CodeClear + 2;
        Context.CodesInfo.CurCodeSize = CodeSize + 1;
      }
#endif

      // Shrink the current pixel sequence down to the last pixel:
      PrevIndex = Pixel;
    } // if (i != GIF_NO_HASH_TABLE_ITEM)
  } while (Area);

  // Store the remaining data of the current pixel sequence:
  err = BitStream_PutCode (&Context, PrevIndex);
  if (err != GIF_ERROR_OK)
    goto lerr;

  // Store the End Of Image code:
  err = BitStream_PutCode (&Context, Context.CodesInfo.CodeEOI);
  if (err != GIF_ERROR_OK)
    goto lerr;

  // Flush/store all buffered bitsream data bits:
  err = BitStream_Flush (&Context);
  if (err != GIF_ERROR_OK)
    goto lerr;

  // Raster data end ID:
  CodeSize = 0;
  err = BitStream_StoreBytes (&Context, &CodeSize, 1);
  if (err != GIF_ERROR_OK)
    goto lerr;

  // GIF terminator:
  CodeSize = 0x3B;
  err = BitStream_StoreBytes (&Context, &CodeSize, 1);
  if (err != GIF_ERROR_OK)
    goto lerr;

  *ppCompressedImage = Context.Compressed.pImage;
  *pCompressedSize = Context.Compressed.Size;

  err = GIF_ERROR_OK;

lerr:

  if (err != GIF_ERROR_OK)
  {
    if ((ppCompressedImage != NULL) && (*ppCompressedImage != NULL))
    {
      free (*ppCompressedImage);
      *ppCompressedImage = NULL;
    }

    if (pCompressedSize != NULL)
    {
      *pCompressedSize = 0;
    }
  }

  if (pHashTable != NULL)
    free (pHashTable);

  return err;
}

static void write_gif(Gifdev *gif)
{	
	tGifScreenDescriptor gsd;
	tGifImageDescriptor  gid;
	
	int nbcolors = 256;
	
	uint8 *xcompressedimage = NULL;
	size_t compressed_size   = 0;
	
	uint8 *ximage = calloc(gif->X_SIZE * gif->Y_SIZE , sizeof(uint8) );

	/* Loop for each byte in the array */
	int x, y;
	
	int i = 0;
	/* Loop for each byte in the array */
	for (y = gif->Y_SIZE-1; y >= 0; y--)
	{
		for ( x = 0; x < gif->X_SIZE ; x++)
	   {
		   int index = gif->Y_SIZE * x + y ;
		   
		   ximage[i++] = *(gif->graphics_rgb + index);
	   }
	}
	
	
	tGifError rc = Gif_Encode(ximage, 0, gif->X_SIZE, gif->Y_SIZE, nbcolors, nbcolors,
									  &gsd, &gid, &xcompressedimage, &compressed_size);

	if ( rc != GIF_ERROR_OK )
	{
		fprintf(stderr,"*write_gif* error\n");
	}
	
	int c = 0;
	// header block + logical screen  
	fwrite((char*)(&gsd), 1, sizeof(tGifScreenDescriptor), gif->fp);
	
	// colors
	for (c = 0; c<nbcolors; c++)
	{
		uint8 color[3] = { gif->coltab[c].r, gif->coltab[c].g, gif->coltab[c].b };
		
		fwrite(color, 1, 3, gif->fp);
	}
	
	// image descriptor
	fwrite((char*)(&gid), 1, sizeof(tGifImageDescriptor), gif->fp);
	
	// Image data
	fwrite(xcompressedimage, sizeof(uint8), compressed_size, gif->fp);
	
	free(ximage);
	
}
	