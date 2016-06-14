#ifndef paagltracker
#define paagltracker

#include "userdata.h"

agl_tracker *agl_tracker_init (struct userdata *);
void agl_tracker_done (struct userdata *);

void agl_tracker_synchronize(struct userdata *);

#endif
