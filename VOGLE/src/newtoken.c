#include <stdlib.h>
#include <stdio.h>
#include "vogle.h"

static TokList		*current;

/*
 * newtokens
 *
 *	returns the space for num tokens
 */
Token *
newtokens(int num)
{
   TokList	*tl;
   Token	*addr;
   int	size;

   if (vdevice.tokens == (TokList *)NULL || num >= MAXTOKS - current->count)
   {
      tl = (TokList *)vallocate(sizeof(TokList));

      if (vdevice.tokens != (TokList *)NULL)
      {
         current->next = tl;
      }
      else
      {
         vdevice.tokens = tl;
      }
      
      tl->count = 0;
      tl->next = (TokList *)NULL;
      if (num > MAXTOKS)
      {
         size = num;
      }
      else
      {
         size = MAXTOKS;
      }
      tl->toks = (Token *)vallocate(size * sizeof(Token));

      current = tl;
   }

   addr = &current->toks[current->count];
   current->count += num;

   return(addr);
}

/*
 * Utility for "appendobj"
 *
 * Finds the end of the token list for a particular object
 * and sets current to the one before it.
 */
void
_setcurrenttoklist(TokList *l)
{
   TokList	*tl, *prev;

   for (prev = tl = l; tl != (TokList *)NULL; tl = tl->next)
   {
      prev = tl;
   }
   
   current = prev;
}

#if 0
printcurrenttoklist(void)
{
   printf("count: %d\n", current->count);
   printf("next: 0x%x\n", current->next);
   printf("toks: 0x%x\n", current->toks);
}
#endif

