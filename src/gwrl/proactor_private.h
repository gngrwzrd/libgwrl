
//unused at the moment, but in place for future use if needed.
typedef struct gwprbufctl {
	size_t ____unused;
} gwprbufctl;

gwprbufctl * gwprbufctl_create();

#define _gwpr(o) ((gwpr *)o)
#define _gwprdata(o) ((gwprdata *)o)
