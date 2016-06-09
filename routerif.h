#ifndef paaglrouterif
#define paaglrouterif

#include "userdata.h"

pa_routerif *pa_routerif_init (struct userdata *, const char *,
	                       const char *, const char *);
void pa_routerif_done (struct userdata *);

#endif
