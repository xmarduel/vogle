/*
 * This include file is intended to be used by programs that
 * interface to other X11 toolkits. It provides the prototypes
 * for the vogle/X11 toolkit interface.
 * One should include all the other needed X11 header files before
 * this one.
 */
extern int 	vo_xt_window(Display *dis, Window win, int xw, int xh);
extern int 	vo_xt_win_size(int w, int xw, int xh);
extern Display	*vo_xt_get_display(int voglewin);
extern Window	vo_xt_get_window(int voglewin);
extern Pixmap	vo_xt_get_backbuf(int voglewin);
extern GC	vo_xt_get_GC(int voglewin);
extern void	vo_xt_set_GC(int voglewin, GC gc);

/* what about the colourmap? */
