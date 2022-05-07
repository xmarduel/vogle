
#include "gpp.h"
#include "vogle.h"
#include "vopl.h"

extern	float	strlength();
extern	char	*styles[];

#define D_DOT	0.01
#define D_LEN	0.1

/*
 * drawlegend
 *
 *	Draw a legend for gpp
 */
void
drawlegend()
{
	float	ydiff, x, y;
	char	buf[80];
	int	i = 0;
	graph	*p;

	ydiff = 2 * TEXTHEIGHT;
	y = YMAX;
	x =  XMAX + 3 * TEXTWIDTH;

	pushattributes();
	linestyle("");
	clipping(0);
	centertext(1);

	textsize(TEXTWIDTH * 1.2, TEXTWIDTH * 1.2);

	/*
 	 * First draw all the marker symbols
	 */
	/*
	linewidth(THICK);
	*/
	if (do_markers) {
		for (i = 0; i < ngraphs; i++) {
			color((i % 7) + 1);
			move2(x, y);
			drawchar('a' + i);
			y -= ydiff;
		}
		x += 1.4 * TEXTWIDTH;
	}

	/* 
	 * Now linestyles...
	 */
	y = YMAX;
	if (do_dash) {
		setdash(D_DOT);
		for (i = 0; i < ngraphs; i++) {
			color((i % 7) + 1);
			linestyle(styles[i % 7]);
			move2(x, y);
			rdraw2(D_LEN, 0.0);
			y -= ydiff;
		}
		x += 1.2 * D_LEN;
		linestyle("");
	}


	/*
	linewidth(THIN);
	 * Now fill in the legends
	 */

	if (fontname[0])
		font(fontname);
	else
		font("futura.l");

	textsize(TEXTWIDTH * 1.2, TEXTHEIGHT * 1.2);

	y = YMAX;
	i = 0;
 
	for (p = gp; p != (graph *)NULL; p = p->nxt) {
		color((i++ % 7) + 1);
		move2(x + strlength(p->legend) / 2, y);
		drawstr(p->legend);
		y -= ydiff;
	}

	popattributes();
}
