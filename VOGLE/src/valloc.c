#include <stdlib.h>
#include "vogle.h"


/*
 * vallocate
 *
 *	Allocate some memory, barfing if malloc returns NULL.
 */
void *
vallocate(unsigned size)
{
   void	*p;
   char	buf[60];

   if ((p = malloc(size)) == NULL)
   {
      sprintf(buf,"vallocate: request for %d bytes returned NULL", size);
      verror(VERR_MALLOC, buf);
   }

   return (p);
}
