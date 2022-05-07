
/*
 *	Driver for HP Graphics Terminals
#define BSD 1
 */
#include <stdio.h>
#ifdef BSD
#include <sgtty.h>
#else
#include <sys/termio.h>
#endif
#include "vogle.h"

#define	HPGT_X_SIZE		640
#define	HPGT_Y_SIZE		400

#include <signal.h>

static int	click, tlstx, tlsty;
static FILE	*fp;
extern FILE	*_voutfile();

/*
 * noop
 *
 *      do nothing but return -1
 */
static int
noop()
{
	return(-1);
}

/*
 * HPGT_init
 *
 *	set up the graphics mode.
 */
HPGT_init()
{
	/*
	 * Find out if we are mono or colour.
	 * and what our resolution currently is.
	 */
	char	buf[128];
	int	i, b1, b2, b3, llx, lly, urx, ury; 

	click = 0;
	fp = _voutfile();

/*
	fprintf(fp, "\033*s5^");
	fgets(buf, 128, stdin);
	sscanf(buf, "%d,%d,%d,%d\n", &llx, &lly, &urx, &ury);
	vdevice.depth = 1;
	fprintf(fp, "\033*s6^");
	fgets(buf, 128, stdin);
	sscanf(buf, "%d,%d,%d", &b1, &b2, &b3);

	if (b3 == 0)
		vdevice.depth = 1;
	else
		vdevice.depth = 4;
	*/

	vdevice.depth = 4;

	fprintf(fp, "\033*dC");
	fprintf(fp, "\033*dF");
	/*
	 * Solid area fills
	 */
	fprintf(fp, "\033*m1g");
	/*
	 * Clear graphics memory
	 */
	fprintf(fp, "\033*dA");

	vdevice.sizeSx = urx - llx;
	vdevice.sizeSy = ury - lly;
	vdevice.sizeSx = HPGT_X_SIZE;
	vdevice.sizeSy = HPGT_Y_SIZE;
	vdevice.sizeX = vdevice.sizeY = vdevice.sizeSy;
	tlstx = tlsty = -1;
	HPGT_font("5x7");
	fflush(fp);

        return(1);
}


/*
 * HPGT_exit
 *
 *	cleans up before going back to normal mode
 */
HPGT_exit()
{
	/* 
	 * Graphics display off
	 * Alpha display on.
	 */
	fprintf(fp, "\033*dD\033*dE");
}

/*
 * HPGT_draw
 *
 *	draw from the current graphics position to the new one (x, y)
 */
HPGT_draw(x, y)
	int	x, y;
{
	fprintf(fp, "\033*pa%d,%d", vdevice.cpVx, vdevice.cpVy);
	fprintf(fp, " %d,%dZ", x, y);
	tlstx = x;
	tlsty = y;
}

/*
 * HPGT_getkey
 *
 *	return the next key typed.
 */
int
HPGT_getkey()
{
#ifdef BSD
	struct sgttyb	oldtty, newtty;
	char		c;

	fflush(fp);
	ioctl(0, TIOCGETP, &oldtty);

	newtty = oldtty;
	newtty.sg_flags = RAW;

	ioctl(0, TIOCSETP, &newtty);

	read(0, &c, 1);

	ioctl(0, TIOCSETP, &oldtty);
#else
	struct termio   oldtty, newtty;
	char            c;
	  
	fflush(fp);
	ioctl(0, TCGETA, &oldtty);

	newtty = oldtty;
	newtty.c_iflag = BRKINT | IXON | ISTRIP;
	newtty.c_lflag = 0;
	newtty.c_cc[VEOF] = 1;

	ioctl(0, TCSETA, &newtty);

	read(0, &c, 1);

	ioctl(0, TCSETA, &oldtty);
#endif
	return(c);
}

/*
 * HPGT_locator
 *
 *	get the position of the crosshairs. This gets a bit sticky since
 * we have no mouse, and the crosshairs do not beahve like a mouse - even a rat!
 * In this case the keys 1 to 9 are used, with each one returning a power of
 * two.
 */
int
HPGT_locator(x, y)
	int	*x, *y;
{
	int		c, i;
	char		buf[50];

	if (click) {			/* for compatability with other devs */
		click = 0;
		return(0);
	}

	click = 1;
	c = -1;

	if (fp == stdout) {
		fprintf(fp, "\033*s4^");
		fgets(buf, 50, stdin);
		sscanf(buf, "%d,%d,%d", x, y, &c);
	}
	return(1 << (c - '1'));
}

/*
 * HPGT_clear
 *
 *	clear the screen.
 *
 */
HPGT_clear()
{
	/*
	 * A rectangular fill absoloute
	if (vdevice.depth == 4)
		fprintf(fp, "\033*m0,0,%d,%de", vdevice.sizeSx, vdevice.sizeSy);
	else
	 */
		/*
		 * Clear graphics memory
		 */
		fprintf(fp, "\033*dA");

	tlstx = tlsty = -1;

}

HPGT_color(i)
	int	i;
{
	/* Select 1 of the standard colours from the system pallete */
	if (vdevice.depth == 1)
		return;

	i %= 8;
	fprintf(fp, "\033*m%dx", i);
	/*... and the graphics text */
	fprintf(fp, "\033*n%dx", i);
	fprintf(fp, "\033*e%dx", i);
	fflush(fp);
}
/*
 * HPGT_font
 *
 *	set for large or small mode.
 */
int
HPGT_font(font)
	char	*font;
{
	int	size;
	vdevice.hwidth = 5.0;
	vdevice.hheight = 7.0;
	size = 2;

	if (strcmp(font, "5x7") == 0)
		size = 1;
	else if ((strcmp(font, "10x14") == 0) || (strcmp(font, "small") == 0))
		size = 2;
	else if ((strcmp(font, "15x21") == 0) || (strcmp(font, "large") == 0))
		size = 3;
	else if (strcmp(font, "20x28") == 0)
		size = 4;
	else if (strcmp(font, "25x35") == 0)
		size = 5;
	else if (strcmp(font, "30x42") == 0)
		size = 6;
	else if (strcmp(font, "35x49") == 0)
		size = 7;
	else if (strcmp(font, "40x56") == 0)
		size = 8;

	vdevice.hwidth *= size;
	vdevice.hheight *= size;
	fprintf(fp, "\033*m%dm", size);
	tlstx = tlsty = -1;
	fflush(fp);

	return(1);
}

/*
 * HPGT_char
 *
 *	outputs one char
 */
HPGT_char(c)
	char	c;
{
	/*
	 * Output a label.
	 */
	if (tlstx != vdevice.cpVx || tlsty != vdevice.cpVy)
		fprintf(fp, "\033*pa%d,%dZ", vdevice.cpVx, vdevice.cpVy);

	fprintf(fp, "\033*l%c\015", c);
	tlstx = vdevice.cpVx += (int)vdevice.hwidth;
	fflush(fp);
}

/*
 * HPGT_string
 *
 *	outputs a string
 */
HPGT_string(s)
	char	*s;
{
	if (tlstx != vdevice.cpVx || tlsty != vdevice.cpVy)
		fprintf(fp, "\033*pa%d,%dZ", vdevice.cpVx, vdevice.cpVy);

	fprintf(fp, "\033*l%s\015", s);
	fflush(fp);
	tlstx = vdevice.cpVx += strlen(s) * (int)vdevice.hwidth;
}

/*
 * HPGT_fill
 *
 *      "fill" a polygon
 */
HPGT_fill(n, x, y)
	int     n, x[], y[];
{
	int     i;

	fprintf(fp, "\033*pas");
	for (i = 0; i < n; i++)
		fprintf(fp, "%d,%d ", x[i], y[i]);

	fprintf(fp, "T");
	fflush(fp);
}

/*
 * Flush/sync the graphics
 */
HPGT_sync()
{
	fflush(fp);
}

/*
 * the device entry
 */
static DevEntry	hpgtdev = {
	"hpgt",
	"large",
	"small",
	noop,
	HPGT_char,
	noop, 		/* HPGT_checkkey, */
	HPGT_clear,
	HPGT_color,
	HPGT_draw,
	HPGT_exit,
	HPGT_fill,
	HPGT_font,
	noop,
	HPGT_getkey,
	HPGT_init,
	HPGT_locator,
	noop,
	noop,
	HPGT_string,
	noop,
	HPGT_sync
};

/*
 * _HPGT_devcpy
 *
 *      copy the tektronix device into vdevice.dev.
 */
_HPGT_devcpy()
{
        vdevice.dev = hpgtdev;
}

