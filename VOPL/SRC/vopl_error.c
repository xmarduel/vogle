/*
 *  vopl_error.c
 *  VOPL-2.0
 *
 *  Created by xavier on Thu Jan 01 1970.
 *  Copyright (c) 1970 __MyCompanyName__. All rights reserved.
 *
 */

#include "vopl_error.h"

#include <stdio.h>
#include <stdlib.h>

#include <setjmp.h>
#include <ctype.h>
#include <unistd.h>

/*----------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------*/

static	int	err_flag = VOPL_EF_JUMP  ;
static	int    warn_flag = VOPL_EF_SILENT;

jmp_buf	vopl_restart;

/*----------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------*/

extern jmp_buf* vopl_jmp_buf__get(void)
{
   return &vopl_restart;
}

/*----------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------*/

/* vopl_set_err_flag -- sets err_flag -- returns old err_flag */
extern  int vopl_set_err_flag(int flag)
{
   int	tmp = err_flag;
   err_flag = flag;

   return tmp;
}

/* vopl_set_warn_flag -- sets warn_flag -- returns old warn_flag */
extern  int vopl_set_warn_flag(int flag)
{
   int	tmp = warn_flag;
   warn_flag = flag;

   return tmp;
}

/*----------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------*/

/* ev_err -- reports error (err_num) in file "file" at line "line_num" and returns to user error handler; */
int vopl_ev_err(char *file, int err_num, int line_num, char *fn_name)
{
   if ( err_num < 0 ) err_num = 0;

   switch ( err_flag )
   {
      case VOPL_EF_SILENT:
         fprintf(stdout,"--> err_flag = EF_SILENT -> ....  \n");
         longjmp(vopl_restart,(err_num==0)? -1 : err_num);
         break;
         
      case VOPL_EF_ABORT:
         fprintf(stdout,"--> err_flag = EF_ABORT -> abort \n");
         fprintf(stderr,"\n\"%s\", line %d: %s in function %s()\n",
               file, line_num, vopl_err_msg[err_num], isascii(*fn_name) ? fn_name : "???");
         if ( ! isatty(fileno(stdout)) )
         {
            fprintf(stdout,"\n\"%s\", line %d: %s in function %s()\n",
                       file, line_num, vopl_err_msg[err_num], isascii(*fn_name) ? fn_name : "???");
            abort();
         }
         break;

      case VOPL_EF_JUMP:
         fprintf(stderr,"\n\"%s\", line %d: %s in function %s()\n",
               file, line_num, vopl_err_msg[err_num], isascii(*fn_name) ? fn_name : "???");
         if ( ! isatty(fileno(stdout)) )
         {
            fprintf(stdout,"\n\"%s\", line %d: %s in function %s()\n",
                  file, line_num, vopl_err_msg[err_num], isascii(*fn_name) ? fn_name : "???");
         }
         longjmp(vopl_restart,(err_num==0)? -1 : err_num);
         break;

      default:
         fprintf(stdout,"--> err_flag = EF_DEFAULT -> exit  \n");
         fprintf(stderr,"\n\"%s\", line %d: %s in function %s()\n\n",
               file, line_num, vopl_err_msg[err_num], isascii(*fn_name) ? fn_name : "???");
         if ( ! isatty(fileno(stdout)) )
         {
            fprintf(stdout,"\n\"%s\", line %d: %s in function %s()\n\n",
                       file, line_num, vopl_err_msg[err_num], isascii(*fn_name) ? fn_name : "???");
         }
         break;
   }

   return 0;
}

/*----------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------*/

extern int vopl_ev_warn(char *file, int warn_num, int line_num, char *fn_name)
{
   if ( warn_num < 0 ) warn_num = 0;

   switch ( warn_flag )
   {
      case VOPL_EF_JUMP:
   
         fprintf(stderr,"\n\"%s\", line %d: **warning** %s in function %s()\n",
                 file, line_num, vopl_err_msg[warn_num], isascii(*fn_name) ? fn_name : "???");

         if ( ! isatty(fileno(stdout)) )
            fprintf(stdout,"\n\"%s\", line %d: %s in function %s()\n",
                    file, line_num, vopl_err_msg[warn_num], isascii(*fn_name) ? fn_name : "???");

            longjmp(vopl_restart,(warn_num==0)? -1 : warn_num);
         break;

      case VOPL_EF_SILENT:

         fprintf(stderr,"\n\"%s\", line %d: **warning** %s in function %s()\n\n",
                 file, line_num, vopl_err_msg[warn_num], isascii(*fn_name) ? fn_name : "???");

         if ( ! isatty(fileno(stdout)) )
         {
            fprintf(stdout,"\n\"%s\", line %d: **warning** %s in function %s()\n\n",
                    file, line_num, vopl_err_msg[warn_num], isascii(*fn_name) ? fn_name : "???");
         }
            break;

      default:
         /* fprintf(stdout,"--> warn_flag = EF_DEFAULT -> nix  \n"); */
         break;
   }

   return 0;
}

/*----------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------*/
