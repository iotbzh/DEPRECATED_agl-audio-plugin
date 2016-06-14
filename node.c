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
#include "node.h"

#include <pulsecore/idxset.h>

agl_nodeset *agl_nodeset_init (struct userdata *u)
{
	agl_nodeset *ns;

	pa_assert (u);

	ns = pa_xnew0 (agl_nodeset, 1);
	ns->nodes = pa_idxset_new (pa_idxset_trivial_hash_func,
		                   pa_idxset_trivial_compare_func);
	ns->roles = pa_hashmap_new (pa_idxset_string_hash_func,
		                    pa_idxset_string_compare_func);
	ns->binaries = pa_hashmap_new (pa_idxset_string_hash_func,
		                       pa_idxset_string_compare_func);
	return ns;
}

void agl_nodeset_done(struct userdata *u)
{
	agl_nodeset *ns;
	agl_nodeset_map *role, *binary;
	void *state;
	int i;

	if (u && (ns = u->nodeset)) {
		pa_idxset_free (ns->nodes, NULL);

		PA_HASHMAP_FOREACH(role, ns->roles, state) {
			pa_xfree ((void *)role->name);
			pa_xfree ((void *)role->resdef);
		}
		pa_hashmap_free (ns->roles);

		PA_HASHMAP_FOREACH(binary, ns->binaries, state) {
			pa_xfree ((void *)binary->name);
			pa_xfree ((void *)binary->resdef);
		}
		pa_hashmap_free (ns->binaries);

		for (i = 0;  i < APCLASS_DIM;  i++)
			pa_xfree((void *)ns->class_name[i]);

		free(ns);
	}
}

agl_node *agl_node_create (struct userdata *u, agl_node *data)
{
	agl_nodeset *ns;
	agl_node *node;

	pa_assert (u);
	pa_assert_se (ns = u->nodeset);

	node = pa_xnew0 (agl_node, 1);

	pa_idxset_put (ns->nodes, node, &node->index);

	if (data) {
		node->key = pa_xstrdup (data->key);
		node->direction = data->direction;
		node->implement = data->implement;
		node->channels = data->channels;
		node->location = data->location;
		node->privacy = data->privacy;
		node->type = data->type;
		node->zone = pa_xstrdup (data->zone);
		node->visible = data->visible;
		node->available = data->available;
		node->amname = pa_xstrdup (data->amname ? data->amname : data->paname);
		node->amdescr = pa_xstrdup(data->amdescr ? data->amdescr : "");
		node->amid = data->amid;
		node->paname = pa_xstrdup (data->paname);
		node->stamp = data->stamp;
		node->rset.id = data->rset.id ? pa_xstrdup(data->rset.id) : NULL;
		node->rset.grant = data->rset.grant;

		if (node->implement == agl_device) {
			node->pacard.index = data->pacard.index;
			if (data->pacard.profile)
				node->pacard.profile = pa_xstrdup (data->pacard.profile);
			if (data->paport)
				node->paport = data->paport;
		}
	}

	 /* TODO : register the node to the router */
	/* agl_router_register_node (u, node); */

	return node;
}

const char *agl_node_type_str (agl_node_type type)
{
	switch (type) {
		case agl_node_type_unknown: return "Unknown";
		case agl_radio:             return "Radio";
		case agl_player:            return "Player";
		case agl_navigator:         return "Navigator";
		case agl_game:              return "Game";
		case agl_browser:           return "Browser";
                case agl_camera:            return "Camera";
		case agl_phone:             return "Phone";
		case agl_alert:             return "Alert";
		case agl_event:             return "Event";
		case agl_system:            return "System";
		default:                    return "<user defined>";
	}
}

agl_node *agl_node_get_from_data (struct userdata *u, agl_direction type, void *data)
{
	pa_sink_input_new_data *sinp_data;
	pa_source_output_new_data *sout_data;
	agl_nodeset *nodeset;
	agl_node *node;
	uint32_t index;

	pa_assert (u);
	pa_assert (data);
	pa_assert (nodeset = u->nodeset);

	pa_assert (type == agl_input || type == agl_output);

	/* input (= sink_input) */
	if (type == agl_input) {
		sinp_data = (pa_sink_input_new_data *) data;
		PA_IDXSET_FOREACH(node, nodeset->nodes, index) {
			if (node->client == sinp_data->client)
				return node;
		}
	/* output (= source_output) TODO */
	/*} else {*/
	}

	return NULL;
}

agl_node *agl_node_get_from_client (struct userdata *u, pa_client *client)
{
	agl_nodeset *nodeset;
	agl_node *node;
	uint32_t index;

	pa_assert (u);
	pa_assert (client);
	pa_assert (nodeset = u->nodeset);

	PA_IDXSET_FOREACH(node, nodeset->nodes, index) {
		if (node->client == client)
			return node;
	}

	return NULL;
}
