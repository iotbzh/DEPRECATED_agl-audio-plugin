#ifndef paagltracker
#define paagltracker

#include "userdata.h"

pa_tracker *pa_tracker_init (struct userdata *);
void pa_tracker_done (struct userdata *);

void pa_tracker_synchronize(struct userdata *);

#endif
