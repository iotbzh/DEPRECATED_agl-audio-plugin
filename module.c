/*
 * module-agl-audio -- PulseAudio module for providing audio routing support
 * (forked from "module-murphy-ivi" - https://github.com/otcshare )
 * Copyright (c) 2012, Intel Corporation.
 * Copyright (c) 2016, IoT.bzh
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU Lesser General Public License,
 * version 2.1, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St - Fifth Floor, Boston,
 * MA 02110-1301 USA.
 *
 */

/* Load the module with :
$ pulseaudio --system --dl-search-path=/mypath
$ pactl load-module mypamodule
 (PS : /mypath must be world-readable, because PulseAudio drops
  its own privileges after initial startup)
*/

 /* THIS IS PROVIDED BY "pulseaudio-module-devel" package */

#include <pulsecore/pulsecore-config.h>	/* required for "module.h" */
#include <pulsecore/module.h>
#include <pulsecore/modargs.h>		/* for "pa_modargs" */

#include "userdata.h"			/* for "struct userdata" */
#include "config.h"			/* for "pa_config_...()" */
#include "utils.h"			/* for "struct pa_null_sink", "pa_utils_create_null_sink()"... */
#include "loopback.h"			/* for "struct pa_loopback/loopnode" */
#include "zone.h"			/* for "struct pa_zoneset" */
#include "node.h"			/* for "struct pa_nodeset" */
#include "audiomgr.h"			/* for "struct pa_audiomgr" */
#include "routerif.h"			/* for "struct pa_routerif" */
#include "discover.h"			/* for "struct pa_discover" */
#include "tracker.h"			/* for "struct pa_tracker" */
#include "router.h"			/* for "struct pa_router" */

#ifndef DEFAULT_CONFIG_DIR
#define DEFAULT_CONFIG_DIR "/etc/pulse"
#endif

#ifndef DEFAULT_CONFIG_FILE
#define DEFAULT_CONFIG_FILE "pulseaudio-agl.cfg"
#endif


 /* VALID ARGUMENTS */

static const char* const valid_modargs[] = {
	"config_dir",
	"config_file",
	"null_sink_name",
	"audiomgr_socktype",
	"audiomgr_address",
	"audiomgr_port",
	NULL
};


 /* INITIALIZATION FUNCTION */

int pa__init (pa_module *m)
{
	pa_log ("Initializing \"pulseaudio-agl\" module");

	struct userdata *u = NULL;
	pa_modargs      *ma = NULL;	/* will contain module arguments */
	const char      *cfgdir;
	const char      *cfgfile;
	const char      *nsnam;		/* NULL sink name (default = "null.agl") */
	const char      *amsocktype;	/* Optional external routing daemon: socket type ("unix"/"tcp") */
	const char      *amaddr;	/* Optional external routing daemon: socket address (path/ip address) */
	const char      *amport;	/* Optional external routing daemon: socket port ("tcp" type only) */
	const char	*cfgpath;
	char buf[4096];

	pa_assert (m);

	 /* treat module arguments */

	ma = pa_modargs_new (m->argument, valid_modargs);

	cfgdir     = pa_modargs_get_value (ma, "config_dir", DEFAULT_CONFIG_DIR);
	cfgfile    = pa_modargs_get_value (ma, "config_file", DEFAULT_CONFIG_FILE);
	nsnam      = pa_modargs_get_value (ma, "null_sink_name", NULL);
	amsocktype = pa_modargs_get_value (ma, "audiomgr_socktype", NULL);
	amaddr     = pa_modargs_get_value (ma, "audiomgr_address", NULL);
	amport     = pa_modargs_get_value (ma, "audiomgr_port", NULL);

	pa_log ("cfgdir : %s", cfgdir);
	pa_log ("cfgfile : %s", cfgfile);

	pa_utils_init_stamp ();

	 /* initialize userdata */

	u = pa_xnew0 (struct userdata, 1);
	u->core      = m->core;
	u->module    = m;
	u->nsnam     = pa_xstrdup (nsnam) ;
	u->zoneset   = pa_zoneset_init (u);
	u->nodeset   = pa_nodeset_init (u);
	u->audiomgr  = pa_audiomgr_init (u);
	u->routerif  = pa_routerif_init (u, amsocktype, amaddr, amport);
	u->router    = pa_router_init (u);
	u->discover  = pa_discover_init (u);
	u->tracker   = pa_tracker_init (u);

	m->userdata = u;

	 /* apply the config file */

	cfgpath = pa_config_file_get_path (cfgdir, cfgfile, buf, sizeof(buf));
	pa_config_parse_file (u, cfgpath);

	 /* really initialize the module's core logic */

	pa_tracker_synchronize (u);

	 /* end */

	pa_modargs_free(ma);

	return 0;
}


 /* CLOSEUP FUNCTION */
void pa__done (pa_module *m)
{
	pa_log ("Closing \"pulseaudio-agl\" module");

	struct userdata *u;

	pa_assert (m);

	if (u = m->userdata) {
		pa_tracker_done (u);
		pa_discover_done (u);
		pa_routerif_done (u);
		pa_audiomgr_done (u);
		pa_nodeset_done (u);
		pa_zoneset_done (u);
		pa_xfree (u->nsnam);
		pa_xfree (u);
	}	
}
