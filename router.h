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
#ifndef paaglrouter
#define paaglrouter

#include "userdata.h"
#include "list.h"
#include "node.h"

#define AGL_ZONE_MAX 8	/* max 8 zones, demo is using 5 */ /* DEFINED IN MURPHY */

typedef bool (*agl_rtgroup_accept_t)(struct userdata *, agl_rtgroup *, agl_node *);
typedef int (*agl_rtgroup_effect_t)(struct userdata *, agl_rtgroup *, agl_node *, bool new);

struct agl_rtgroup {
	char *name;          /**< name of the rtgroup */
	agl_dlist entries;   /**< listhead of ordered rtentries */
	agl_node *node;      /**< final node */
	agl_rtgroup_accept_t accept; /**< function pointer, whether to accept a node or not */
	agl_rtgroup_effect_t effect; /**< function pointer, custom action such as volume up, down */
};

typedef struct {
	pa_hashmap *input;
	pa_hashmap *output;
} agl_rtgroup_hash;

typedef struct {
	agl_rtgroup **input[AGL_ZONE_MAX];
	agl_rtgroup **output[AGL_ZONE_MAX];
} agl_rtgroup_classmap;

struct agl_router {
	agl_rtgroup_hash rtgroups;
	size_t maplen;		      /**< length of the class */
	agl_rtgroup_classmap classmap; /**< map device node types to rtgroups */
	int *priormap;                /**< stream node priorities */
	agl_dlist nodlist;            /**< priorized list of the stream nodes
                                        (entry in node: rtprilist) */
	agl_dlist connlist;           /**< listhead of the connections */
};

struct agl_connection {
	agl_dlist link;    /**< list of connections */
	bool blocked;      /**< true if this conflicts with another route */
	uint16_t amid;     /**< audio manager connection id */
	uint32_t from;     /**< source node index */
	uint32_t to;       /**< destination node index */
	uint32_t stream;   /**< index of the sink-input to be routed */
};

agl_router *agl_router_init (struct userdata *);
void agl_router_done (struct userdata *);

bool agl_router_default_accept (struct userdata *, agl_rtgroup *, agl_node *);
bool agl_router_phone_accept (struct userdata *, agl_rtgroup *, agl_node *);
int agl_router_default_effect (struct userdata *, agl_rtgroup *, agl_node *, bool);
int agl_router_phone_effect (struct userdata *, agl_rtgroup *, agl_node *, bool);

agl_rtgroup *agl_router_create_rtgroup (struct userdata *, agl_direction, const char *, const char *, agl_rtgroup_accept_t, agl_rtgroup_effect_t);
void agl_router_destroy_rtgroup (struct userdata *, agl_direction, const char *);
bool agl_router_assign_class_to_rtgroup (struct userdata *, agl_node_type, uint32_t, agl_direction, const char *);
agl_rtgroup * agl_router_get_rtgroup_from_class (struct userdata *u, agl_node_type class, uint32_t zone, agl_direction type);
void agl_router_assign_class_priority (struct userdata *, agl_node_type, int);
int agl_router_get_node_priority (struct userdata *, agl_node *);
bool agl_router_apply_node_priority_effect (struct userdata *, agl_node *, bool);

void agl_router_register_node (struct userdata *, agl_node *);
void agl_router_unregister_node (struct userdata *, agl_node *);
agl_node *agl_router_make_prerouting (struct userdata *, agl_node *);
void agl_router_make_routing (struct userdata *);

void implement_default_route (struct userdata *, agl_node *, agl_node *, uint32_t);
agl_node *find_default_route (struct userdata *, agl_node *, uint32_t);
void remove_routes (struct userdata *, agl_node *, agl_node*, uint32_t);

#endif
