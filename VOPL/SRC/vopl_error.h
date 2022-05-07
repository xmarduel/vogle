#ifndef VOPL_ERROR_H_
#define VOPL_ERROR_H_

/*
 *  vopl_error.h
 *  VOPL-2.0
 *
 *  Created by xavier on Thu Jan 01 1970.
 *  Copyright (c) 1970 __MyCompanyName__. All rights reserved.
 *
 */

#ifdef __cplusplus
extern "C"
{
#endif

#define VOPL_ERR_BAD_SUBP		1
#define VOPL_ERR_AXIS_NB		2
#define VOPL_ERR_Z_AXIS			3
#define VOPL_ERR_UNKNOWN_FIT_TYPE	4
#define VOPL_ERR_TICKMARKS		5

static char *vopl_err_msg[] = {
   "",
   "Choice of sub-panel",
   "Unknown axis",
   "No z-axis",
   "unknown fit type",
   "number of tickmarks on axis must be >= 0"
};


int vopl_ev_err (char *, int, int, char *);  /* main error handler */
int vopl_ev_warn(char *, int, int, char *);  /* main error handler */

int vopl_set_err_flag(int flag);         /* for different ways of handling errors, returns old value */
int vopl_set_warn_flag(int flag);

/* error flags */
#define	VOPL_EF_EXIT	0	/* exit on error */
#define	VOPL_EF_ABORT	1	/* abort (dump core) on error */
#define	VOPL_EF_JUMP	2	/* jump on error */
#define	VOPL_EF_SILENT	3	/* jump, but don't print message */

#define	VOPL_ERREXIT()		vopl_set_err_flag(VOPL_EF_EXIT)
#define	VOPL_ERRABORT()		vopl_set_err_flag(VOPL_EF_ABORT)
/* don't print message */
#define	VOPL_SILENTERR()	if ( ! setjmp(vopl_restart) ) vopl_set_err_flag(VOPL_EF_SILENT)
/* return here on error */
#define	VOPL_ON_ERROR()		if ( ! setjmp(vopl_restart) ) vopl_set_err_flag(VOPL_EF_JUMP)


#define	vopl_error(err_num,fn_name)      vopl_ev_err(__FILE__,err_num,__LINE__,fn_name)
#define vopl_warning0(err_num,fn_name)   vopl_ev_warn(__FILE__,err_num,__LINE__,fn_name)

#ifdef __cplusplus
}
#endif
   
#endif
