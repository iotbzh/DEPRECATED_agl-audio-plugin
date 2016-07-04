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
#include "zone.h"

struct agl_zoneset {
	struct {
		pa_hashmap     *hash;
		agl_zone       *index[AGL_ZONE_MAX];
	} zones;
};

agl_zoneset *agl_zoneset_init (struct userdata *u)
{
	agl_zoneset *zs;

	pa_assert (u);

	zs = pa_xnew0 (agl_zoneset, 1);
	zs->zones.hash = pa_hashmap_new (pa_idxset_string_hash_func,
		                         pa_idxset_string_compare_func);

	return zs;
}

void agl_zoneset_done (struct userdata *u)
{
	agl_zoneset *zs;
	void *state;
	agl_zone *zone;

	if (u && (zs = u->zoneset)) {

		PA_HASHMAP_FOREACH(zone, zs->zones.hash, state) {
			pa_xfree ((void *)zone->name);
		}
		pa_hashmap_free (zs->zones.hash);
		free (zs);
	}    
}

int agl_zoneset_add_zone (struct userdata *u, const char *name, uint32_t index)
{
	agl_zoneset *zs;
	agl_zone *zone;

	pa_assert (u);
	pa_assert (name);
	pa_assert (index < AGL_ZONE_MAX);
	pa_assert_se (zs = u->zoneset);

	zone = pa_xnew (agl_zone, 1);
	zone->name = pa_xstrdup (name);
	zone->index = index;

	pa_hashmap_put (zs->zones.hash, (void *)zone->name, zone);

	zs->zones.index[index] = zone;

	return 0;
}

agl_zone *agl_zoneset_get_zone_by_name (struct userdata *u, const char *name)
{
	agl_zoneset *zs;
	agl_zone *zone;

	pa_assert (u);
	pa_assert_se (zs = u->zoneset);

	if (name && zs->zones.hash)
		zone = pa_hashmap_get (zs->zones.hash, name);
	else
		zone = NULL;

	return zone;
}

agl_zone *agl_zoneset_get_zone_by_index (struct userdata *u, uint32_t index)
{
	agl_zoneset *zs;
	agl_zone *zone;

	pa_assert (u);
	pa_assert_se (zs = u->zoneset);

	if (index < AGL_ZONE_MAX)
		zone = zs->zones.index[index];
	else
		zone = NULL;

	return zone;
}
