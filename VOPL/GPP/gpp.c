/*
 *   gpp
 *  
 *   Graph plotting programme. 
 *
 */
#include "vogle.h"
#include "gpp.h"

char	*styles[] = {
	"1100",
	"11110",
	"11010",
	"10",
	"10010",
	"10110",
	"0101100"
};

/*
 * main driver
 */
main(argc, argv)
	int	argc;
	char	**argv;
{
	int	i;
	char	cm[2];
	graph	*p;

	doenv();
	doargs(argc, argv);


	if (do_legend && !(do_dash || do_markers))
		do_markers = 1;

	markerspacing(do_markers);

	prefsize(1000, 750);
	vinit(device);

	viewport(-wholescale, wholescale, -wholescale, wholescale);

	color(BLACK);
	clear();

	color(WHITE);

	if (fontname[0])
		font(fontname);

	readgraphs();
	
	if (!uxscale || !uyscale /* || !uzscale */) 
		for (p = gp; p != (graph *)NULL; p = p->nxt) {
			if (!uxscale)
				adjustscale(p->x, p->npnts, 'x');

			if (!uyscale)
				adjustscale(p->y, p->npnts, 'y');

			/*if (!uzscale)
				adjustscale(p->z, p->npnts, 'z');*/
		}

	axistitle(xlabel, 'x');
	axistitle(ylabel, 'y');
	/*axistitle(zlabel, 'z');*/

	drawaxes();
	drawtitle();

	i = 0;
	cm[1] = '\0';

	font("markers");
	linewidth(THICK);


	for (p = gp; p != (graph *)NULL; p = p->nxt) {
		if (do_dash)
			linestyle(styles[i % 7]);

		color((i % 7) + 1);
		cm[0] = 'a' + (i++);
		marker(cm);
		plot2(p->x, p->y, p->npnts);
	}

	linewidth(THIN);

	if (do_legend)
		drawlegend();


	getkey();

	vexit();
}
