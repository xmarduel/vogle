/*
**	VOGLE Driver for UNIX "plot" format output
*/

#include <stdio.h>
#include "vogle.h"

#define SPACE_SIZE	1024
#define	POINT(x, y)	(0x10000 * (y) + (x))
#define MAXCOLOR	7

extern FILE	*_voutfile();

static int	uplot_first_time = 1, drawn = 0,
		curcol = 0,			/* black */
		uplotlstx = -1, uplotlsty = -1;	/* last (x, y) drawn */

#undef MAP_TO_LINESTYLES	/* This works, but doesn't look great on text */

/*
 * Line style map for our standard colours
 */
static char	*colormap[MAXCOLOR + 1] = {
	"solid",
	"dotted",
	"shortdashed",
	"longdashed",
	"dotdashed",
	"solid",
	"solid",
	"solid"
};

static FILE	*fp;

/*
 * noop
 *
 *	do nothing but return -1
 */
static int
noop()
{
	return(-1);
}

/*
 * putpt
 *
 *	Put a point out to the file.  Two two-byte values, little-endian.
 *	NOTE:  This assumes 8-bit chars and 16-bit shorts.
 */
static void
putpt(x, y, fp)
int x;
int y;
FILE *fp;
{
	short sx, sy;

	sx = (short) x;
	sy = (short) y;

	putc((sx & 0xff), fp);
	putc(((sx >> 8) & 0xff), fp);
	putc((sy & 0xff), fp);
	putc(((sy >> 8) & 0xff), fp);
}

/*
 * uplot_init
 *
 *	Set up the unixplot environment. Returns 1 on success.
 */
static
uplot_init()
{
	fp = _voutfile();

	if (!uplot_first_time)
		return(1);

	putc('s', fp);
	putpt(0, 0, fp);
	putpt(SPACE_SIZE, SPACE_SIZE, fp);

	vdevice.sizeSx = vdevice.sizeSy = SPACE_SIZE; 
	vdevice.sizeX = vdevice.sizeY = SPACE_SIZE; 

	vdevice.depth = 1;

	return(1);
}


/*
 * uplot_exit
 *
 *	Flush remaining data and close the output file if neccessary.
 */
static
uplot_exit()
{
	fflush(fp);
	if (fp != stdout)
	{
		fclose(fp);
	}
}

/*
 * uplot_draw
 *
 *	draw to an x, y point.
 */
static
uplot_draw(x, y)
	int	x, y;
{
	if (uplotlstx != vdevice.cpVx || uplotlsty != vdevice.cpVy)
	{
		putc('m', fp);
		putpt(vdevice.cpVx, vdevice.cpVy, fp);
	}

	putc('n', fp);
	putpt(x, y, fp);
	uplotlstx = x;
	uplotlsty = y;
	drawn = 1;
}

/*
 * uplot_font
 *
 *	There's not much we can do for this.  We can't even know
 *	the width or height!
 */
static
uplot_font(font)
	char	*font;
{
	if (strcmp(font, "large") == 0 || strcmp(font, "small") == 0)
	{
		vdevice.hwidth = 1.0;
		vdevice.hheight = 1.0;
		return(1);
	}
	else
	{
		return(0);
	}
}

/*
 * uplot_clear
 *
 *	Erase the plot
 */
static
uplot_clear()
{
	if (drawn)
	{
		putc('e', fp);
	}

	drawn = 0;
}

/*
 * uplot_color
 *
 *	Change the linestyle of the lines
 */
static
uplot_color(col)
	int	col;
{
	if (col > MAXCOLOR)
		return;

	curcol = col;

#ifdef MAP_TO_LINESTYLES
	fprintf(fp, "f%s\n", colormap[curcol]);
#endif
}
	
/*
 * uplot_char
 *
 *	Output a character
 */
static
uplot_char(c)
	char	c;
{
	if (uplotlstx != vdevice.cpVx || uplotlsty != vdevice.cpVy)
	{
		putc('m', fp);
		putpt(vdevice.cpVx, vdevice.cpVy, fp);
	}

	fprintf(fp, "t%c\n", c);

	drawn = 1;
	uplotlstx = uplotlsty = -1;
}

/*
 * uplot_string
 *
 *	output a string one char at a time.
 */
static
uplot_string(s)
	char	*s;
{
	if (uplotlstx != vdevice.cpVx || uplotlsty != vdevice.cpVy)
	{
		putc('m', fp);
		putpt(vdevice.cpVx, vdevice.cpVy, fp);
	}

	fprintf(fp, "t%s\n", s);

	drawn = 1;
	uplotlstx = uplotlsty = -1;
}

/*
 * uplot_fill
 *
 *	Should do a fill, but we can't so just draw the polygon
 */
static
uplot_fill(n, x, y)
	int     n, x[], y[];
{
	int     i;

	putc('m', fp);
	putpt(x[0], y[0], fp);

	for (i = 1; i < n; i++)
	{
		putc('n', fp);
		putpt(x[i], y[i], fp);
	}
	putc('n', fp);
	putpt(x[0], y[0], fp);

	vdevice.cpVx = x[n - 1];
	vdevice.cpVy = y[n - 1];

	uplotlstx = uplotlsty = -1;
}

static DevEntry uplotdev = {
	"unixplot",
	"large",
	"small",
	noop,
	uplot_char,
	noop,
	uplot_clear,
	uplot_color,
	uplot_draw,
	uplot_exit,
	uplot_fill,
	uplot_font,
	noop,
	noop,
	uplot_init,
	noop,
	noop,
	noop,
	uplot_string,
	noop,
	noop
};

/*
 * _unixplot_devcpy
 *
 *	copy the unixplot device into vdevice.dev.
 */
_unixplot_devcpy()
{
	vdevice.dev = uplotdev;
}
