#define VERR_UNINIT	1
#define VERR_STACKUF	2
#define VERR_STACKOF	3
#define VERR_NUMSEGS	4
#define VERR_NUMPNTS	5
#define VERR_FILEIO	6
#define VERR_NOFONT	7
#define VERR_NOHFONT	8
#define VERR_NUMCRVS	9
#define VERR_CLIPERR	10
#define VERR_BADAXIS	11
#define VERR_BADASPECT	12
#define VERR_BADPLANE	13
#define VERR_BADFOV	14
#define VERR_BADDEV	15
#define VERR_MALLOC	16
#define VERR_NOOBJECT	17
#define VERR_MAXPNTS	18
#define VERR_BADCOORD	19
#define VERR_BADVECTOR	20
#define VERR_BADVP	21
#define VERR_BADWIN	22

static char	*errmsg[] = {
	"",
	"vogle not initialised",
	"stack underflow",
	"stack overflow",
	"number of segments <= 0",
	"not enough points in geometry matrix",
	"unable to open file",
	"no font loaded",
	"no software font loaded",
	"number of patch curves <= 0",
	"impossible error in the clipping routine",
	"illegal axis of rotation",
	"can't have zero aspect ratio!",
	"bad clipping plane specification",
	"bad field of view",
	"no device, or error opening device",
	"malloc returns NULL",
	"no in an object"
	"too many vertices",
	"bad coordinate specified",
	"bad vector specified",
	"bad viewport specified",
	"bad window id"
};
