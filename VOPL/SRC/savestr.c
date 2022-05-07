
/** \file savestr.c
 *
 */

#include "vopl.h"

#include <string.h>

/*
 * savestr
 *
 *	Save a string of goddam characters somewhere
 */
char *savestr(char *old, char *string)
{
   char	*p;

   if (string == (char *)NULL)
      return ((char *)NULL);

   p = old;

   if (p != (char *)NULL)
   {
      if (strlen(string) > strlen(p))
      {
         free(p);
         p = (char *)malloc(strlen(string) + 1);
      }
   }
   else
   {
      p = (char *)malloc(strlen(string) + 1);
   }

   strcpy(p, string);

   return(p);
}
