
/** \file err.c
 *
 */

#include "vopl.h"


/*
 * vopl_error
 *
 *	Prints an error message.
 */
void vopl_error(char *str)
{
   if (vdevice.initialised)
      vexit();

   fprintf(stderr,"vopl: %s\n", str);
   exit(1);
}
