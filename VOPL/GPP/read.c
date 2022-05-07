#include "gpp.h"

char	*fmttable[] = {
	"",
	"%f ",
	"%f ",
	"%f %f ",
	"%f ",
	"%f %f ",
	"%f %f ",
	"%f %f %f "
};

extern	float	*newm1();

/*
 * getline
 *
 *	reads in a file from file in.
 */
char *
getline(in)
	FILE	*in;
{
	char	*p, line[BUFSIZ]; 
	int	c;

	p = line;

	while ((c = getc(in)) != '\n') {
		if (feof(in))
			return((char *)NULL);
		*p++ = c;
	}

	*p = 0;

	p = (char *)malloc(p - line + 1);
	strcpy(p, line);

	return(p);
}

/*
 * decode
 *
 *	converts string type into its X, Y, and Z code.
 */
int
decode(type)
	char	*type;
{
	int	itype;
	char	*sp;

	itype = 0;

	for (sp = type; *sp; sp++)
		switch (*sp) {
		case 'x':
			itype |= X;
			break;
		case 'y':
			itype |= Y;
			break;
		case 'z':
			itype |= Z;
			break;
		default:
			fprintf(stderr, "gpp: bad axes/type descriptor\n");
			exit(1);
		}

	return(itype);
}

/*
 * readgraphs
 *
 *	read in the graphs
 */
void
readgraphs()
{
	int		type;
	graph		*p, *lp;
	char		stype[4], *fmt, *title;
	int		atype, c;
	float		x, y, z, *xaxis, *yaxis, *zaxis;

	ngraphs = 0;

	/*  First we read the title */

	if ((title = getline(infile)) == NULL) {
		fprintf(stderr,"gpp: EOF when title was expected.\n");
		exit(1);
	}

	graphtitle(title);

	/* next the type */

	if (fscanf(infile, "type %s", stype) != 1) {
		fprintf(stderr,"gpp: EOF when type expected.\n");
		exit(1);
	}

	getc(infile);		/* skip single newline */

	type = decode(stype);

	if ((type & X) && (xlabel = getline(infile)) == NULL) {
		fprintf(stderr,"gpp: EOF when x-axis label expected.\n");
		exit(1);
	}
	if ((type & Y) && (ylabel = getline(infile)) == NULL) {
		fprintf(stderr,"gpp: EOF when y-axis label was expected.\n");
		exit(1);
	}
	if ((type & Z) && (zlabel = getline(infile)) == NULL) {
		fprintf(stderr,"gpp: EOF when z-axis label was expected.\n");
		exit(1);
	}

	/*  Now we should be reading coordinates */

	xaxis = (double *)NULL;
	yaxis = (double *)NULL;
	zaxis = (double *)NULL;

	p = gp = (graph *)malloc(sizeof(graph));
	if (gp == (graph *)NULL) {
		fprintf(stderr, "gpp: malloc returns NULL.\n");
		exit(1);
	}

	if ((gp->legend = getline(infile)) == NULL) {
		fprintf(stderr,"gpp: EOF when legend was expected.\n");
		exit(1);
	}

	while (!feof(infile)) {
		if (fscanf(infile, "axes %s ", stype) != 1) {
			fprintf(stderr,"EOF when axes expected.\n");
			exit(1);
		}

		p->x = xaxis;
		p->y = yaxis;
		p->z = zaxis;

		atype = decode(stype);

		fmt = fmttable[atype];

		switch (atype) {
		case X:
			xaxis = p->x = newm1(maxpnts);
			p->npnts = 0;
			while (fscanf(infile, fmt, &x) != 0) {
				p->x[p->npnts++] = x;
			}
			break;
		case Y:
			yaxis = p->y = newm1(maxpnts);
			p->npnts = 0;
			while (fscanf(infile, fmt, &y) != 0) {
				p->y[p->npnts++] = y;
			}
			break;
		case Z:
			zaxis = p->z = newm1(maxpnts);
			p->npnts = 0;
			while (fscanf(infile, fmt, &z) != 0) {
				p->z[p->npnts++] = z;
			}
			break;
		case X | Y:
			xaxis = p->x = newm1(maxpnts);
			yaxis = p->y = newm1(maxpnts);
			p->npnts = 0;
			while (fscanf(infile, fmt, &x, &y) != 0) {
				p->x[p->npnts] = x;
				p->y[p->npnts++] = y;
			}
			break;
		case X | Z:
			xaxis = p->x = newm1(maxpnts);
			zaxis = p->z = newm1(maxpnts);
			p->npnts = 0;
			while (fscanf(infile, fmt, &x, &z) != 0) {
				p->x[p->npnts] = x;
				p->z[p->npnts++] = z;
			}
			break;
		case Y | Z:
			yaxis = p->y = newm1(maxpnts);
			zaxis = p->z = newm1(maxpnts);
			p->npnts = 0;
			while (fscanf(infile, fmt, &y, &z) != 0) {
				p->y[p->npnts] = y;
				p->z[p->npnts++] = z;
			}
			break;
		case X | Y | Z:
			xaxis = p->x = newm1(maxpnts);
			yaxis = p->y = newm1(maxpnts);
			zaxis = p->z = newm1(maxpnts);
			p->npnts = 0;
			while (fscanf(infile, fmt, &x, &y, &z) != 0) {
				p->x[p->npnts] = x;
				p->y[p->npnts++] = y;
			}
			break;
		default:
			fprintf(stderr, "gpp: readgraphs - internal error.\n");
			exit(1);
		}
 
		if (fscanf(infile, "plot %s", stype) == 1) {
			p->type = decode(stype);
			if ((p->nxt = (graph *)malloc(sizeof(graph))) == (graph *)NULL) {
				fprintf(stderr, "readgraphs: malloc returns NULL\n");
				exit(1);
			}
			lp = p;
			p = p->nxt;

			/*
			 * skip newline after plot command
			 */
			while ((c = getc(infile)) != '\n' && !feof(infile))
				;

			p->legend = getline(infile);

			ngraphs++;

			/*
			 * skip extra characters before next axis keyword
			 */
			while ((c = getc(infile)) != 'a' && !feof(infile))
				;

			if (!feof(infile))
				ungetc(c, infile);
		}
	}

	/* finish off the last one */
	lp->nxt = NULL;
}
