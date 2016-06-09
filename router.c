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
#include "router.h"
#include "switch.h"
#include "utils.h"

pa_router *pa_router_init (struct userdata *u)
{
	pa_router *router;
	size_t num_classes;

	num_classes = agl_application_class_end;

	router = pa_xnew0 (pa_router, 1);
	router->rtgroups.input = pa_hashmap_new (pa_idxset_string_hash_func,
		                                 pa_idxset_string_compare_func);
	router->rtgroups.output = pa_hashmap_new (pa_idxset_string_hash_func,
		                                  pa_idxset_string_compare_func);
	router->maplen = num_classes;
        router->priormap = pa_xnew0 (int, num_classes);

	AGL_DLIST_INIT (router->nodlist);
	AGL_DLIST_INIT (router->connlist);

	return router;
}

void pa_router_done (struct userdata *u)
{
	pa_router *router;
	agl_node *e,*n;
	agl_connection *conn, *c;
	agl_rtgroup *rtg;
	agl_rtgroup **map;
	int i;

	if (u && (router = u->router)) {
		AGL_DLIST_FOR_EACH_SAFE(agl_node, rtprilist, e,n, &router->nodlist)
			AGL_DLIST_UNLINK(agl_node, rtprilist, e);
		AGL_DLIST_FOR_EACH_SAFE(agl_connection, link, conn,c, &router->connlist) {
			AGL_DLIST_UNLINK(agl_connection, link, conn);
			pa_xfree (conn);
		}
		/*
		PA_HASHMAP_FOREACH(rtg, router->rtgroups.input, state) {
			rtgroup_destroy(u, rtg);
		}
		PA_HASHMAP_FOREACH(rtg, router->rtgroups.output, state) {
			rtgroup_destroy(u, rtg);
		}*/
		pa_hashmap_free (router->rtgroups.input);
		pa_hashmap_free (router->rtgroups.output);

		for (i = 0;  i < AGL_ZONE_MAX;  i++) {
			if ((map = router->classmap.input[i]))
				pa_xfree(map);
			if ((map = router->classmap.output[i]))
				pa_xfree(map);
		}

		pa_xfree (router->priormap);
		pa_xfree (router);

		u->router = NULL;
	}
}

agl_node *agl_router_make_prerouting (struct userdata *u, agl_node *data)
{
	pa_router *router;
	int priority;
	static bool done_prerouting;
	uint32_t stamp;
	agl_node *start, *end;
	agl_node *target;

	pa_assert (u);
	pa_assert_se (router = u->router);
	pa_assert_se (data->implement == agl_stream);

	//priority = node_priority (u, data);

	done_prerouting = false;
	target = NULL;
	stamp = pa_utils_new_stamp ();

	//make_explicit_routes (u, stamp);

	//pa_audiomgr_delete_default_routes(u);

	AGL_DLIST_FOR_EACH_BACKWARDS(agl_node, rtprilist, start, &router->nodlist) {
		//if ((start->implement == agl_device) &&
		//    (!start->loop))	/* only manage looped real devices */
		//	continue;

		/*if (priority >= node_priority (u, start)) {
			target = find_default_route (u, data, stamp);
			if (target)
				implement_preroute (u, data, target, stamp);
			else
				done_prerouting = true;
		}*/

		if (start->stamp >= stamp)
			continue;

		//end = find_default_route (u, start, stamp);
		//if (end)
		//	implement_default_route(u, start, end, stamp);
	}

	if (!done_prerouting) {
		pa_log_debug ("Prerouting failed, trying to find default route as last resort");

		//target = find_default_route (u, data, stamp);
		//if (target)
		//	implement_preroute (u, data, target, stamp);
	}

	return target;
}

void agl_router_make_routing (struct userdata *u)
{
	pa_router *router;
	static bool ongoing_routing;	/* true while we are actively routing */
	uint32_t stamp;
	agl_node *start, *end;

	pa_assert (u);
	pa_assert_se (router = u->router);

	if (ongoing_routing)		/* already routing, canceling */
		return;
	ongoing_routing = true;
	stamp = pa_utils_new_stamp ();

	pa_log_debug("stamp for routing: %d", stamp);

	// make_explicit_routes (u, stamp);

	// pa_audiomgr_delete_default_routes (u);

	AGL_DLIST_FOR_EACH_BACKWARDS(agl_node, rtprilist, start, &router->nodlist) {
		//if ((start->implement == agl_device) &&
		//   (!start->loop))	/* only manage looped real devices */
		//	continue;

		if (start->stamp >= stamp)
			continue;

		end = find_default_route (u, start, stamp);
		if (end)
			implement_default_route (u, start, end, stamp);
	}

	// pa_audiomgr_send_default_routes (u);

	ongoing_routing = false;
}

void implement_default_route (struct userdata *u,
                              agl_node *start, agl_node *end,
                              uint32_t stamp)
{
	if (start->direction == agl_input) {
		agl_switch_setup_link (u, start, end, false);
		//agl_volume_add_limiting_class(u, end, volume_class(start), stamp);
	} else {
		agl_switch_setup_link (u, end, start, false);
	}
}

agl_node *find_default_route (struct userdata *u, agl_node *start, uint32_t stamp)
{
	/* TODO */

	return NULL;
}
