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
#include "routerif.h"

struct pa_routerif {
    int  sock;
};

pa_routerif *pa_routerif_init (struct userdata *u,
                               const char      *socktyp,
                               const char      *addr,
                               const char      *port)
{
	pa_module *m = u->module;
	pa_routerif *routerif = NULL;
    
	routerif = pa_xnew0 (pa_routerif, 1);
	routerif->sock = -1;

	return routerif;
}

void pa_routerif_done (struct userdata *u)
{
	pa_routerif *routerif;

	if (u && (routerif = u->routerif)) {
		if (routerif->sock >= 0)
			close (routerif->sock);

		pa_xfree (routerif);

		u->routerif = NULL;
	}
}
