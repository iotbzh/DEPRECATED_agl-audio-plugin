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
#include "audiomgr.h"

#define DS_DOWN 255

typedef struct {
	const char *name;
	uint16_t    id;
	uint16_t    state;
} domain_t;

typedef struct {
	uint16_t    fromidx;
	uint16_t    toidx;
	uint32_t    channels;
} link_t;

typedef struct {
	int          maxlink;
	int          nlink;
	link_t      *links;
} routes_t;

struct agl_audiomgr {
	domain_t      domain;
	pa_hashmap   *nodes;        /**< nodes ie. sinks and sources */
	pa_hashmap   *conns;        /**< connections */
	routes_t      defrts;       /**< default routes */
};

struct agl_audiomgr *agl_audiomgr_init (struct userdata *u)
{
	agl_audiomgr *am;

	pa_assert (u);

	am = pa_xnew0 (agl_audiomgr, 1);
	am->domain.id = AM_ID_INVALID;
	am->domain.state = DS_DOWN;
	am->nodes = pa_hashmap_new (pa_idxset_trivial_hash_func,
		                    pa_idxset_trivial_compare_func);
	am->conns = pa_hashmap_new (pa_idxset_trivial_hash_func,
		                    pa_idxset_trivial_compare_func);
	return am;
}

void agl_audiomgr_done (struct userdata *u)
{
	agl_audiomgr *am;

	if (u && (am = u->audiomgr)) {
		//if (u->routerif && am->domain.id != AM_ID_INVALID)
		//	pa_routerif_unregister_domain (u, am->domain.id);

		pa_hashmap_free (am->nodes);
		pa_hashmap_free (am->conns);
		pa_xfree ((void *)am->domain.name);
		pa_xfree (am);
		u->audiomgr = NULL;
	}
}

