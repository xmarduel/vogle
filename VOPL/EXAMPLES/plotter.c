/* plotter.c module */
/* contains routines to plot a data file produced by another program  */
/* 2d data plotted in this version                                    */
/**********************************************************************/

#include "externals.h"

static FILE *plot1,*plot2,*plot3,*ashell;

static char *startplot1 = "plot [] [0:1.1]'plot11.dat' with lines, 'plot12.dat' with lines\n";
static char *startplot2 = "plot 'plot21.dat' with lines, 'plot22.dat' with lines\n";

static char *replot = "replot\n";
static char *command1= "/sw/bin/gnuplot> dump1";
static char *command2= "/sw/bin/gnuplot> dump2";
static char *deletefiles = "rm plot11.dat plot12.dat plot21.dat plot22.dat";
static char *set_term = "set terminal x11\n";

static char *command3     = "meshtvx";
static char *open_file    = "open \"SolApproch2D.silo\"\n";
static char *prepare_plot = "surf surfcol=z,legend=on,var=\"vitesse\"\n";
static char *make_plot    = "plot surf\n";


void
StartPlot(void)
{
   plot1 = popen(command1, "w");
   fprintf(plot1, "%s", set_term);
   fflush(plot1);
   if (plot1 == NULL)
      exit(2);

   /*plot2 = popen(command2, "w");
   fprintf(plot2, "%s", set_term);
   fflush(plot2);
   if (plot2 == NULL)
      exit(2);*/

   plot3 = popen(command3, "w");
   fprintf(plot3, "%s", open_file);
   fprintf(plot3, "%s", prepare_plot);
   fprintf(plot3, "%s", make_plot);
   fflush(plot3);

   if (plot3 == NULL)
      exit(2);
}

void
RemoveDat(void)
{
   ashell = popen(deletefiles, "w");
   exit(0);
}

void
StopPlot(void)
{
   pclose(plot1);
   /*pclose(plot2);*/
}

void
PlotOne(void)
{
   fprintf(plot1, "%s", startplot1);
   fflush(plot1);

 /*  fprintf(plot2, "%s", startplot2);
   fflush(plot2);*/
}

void
RePlot(void)
{
   fprintf(plot1, "%s", replot);
   fflush(plot1);
}