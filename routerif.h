#ifndef paaglrouterif
#define paaglrouterif

#include "userdata.h"

agl_routerif *agl_routerif_init (struct userdata *, const char *,
	                         const char *, const char *);
void agl_routerif_done (struct userdata *);

#endif
