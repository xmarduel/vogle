/*
 * This include file is intended to be used by programs that
 * interface to APPLE CARBON toolkit. It provides the prototypes
 * for the vogle/CARBON toolkit interface.
 * One should include all the other needed CARBON header files before
 * this one.
 */
#ifdef CARBON

extern int 	      vo_cb_window(WindowRef win, int xw, int xh);
extern int      	vo_cb_win_size(int w, int xw, int xh);
extern WindowRef	vo_cb_get_window(int voglewin);

/* what about the colourmap? */

#endif

