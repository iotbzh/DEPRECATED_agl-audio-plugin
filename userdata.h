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
#ifndef paagluserdata
#define paagluserdata

#include <pulsecore/pulsecore-config.h>	/* required for "core.h" and "module.h" */
#include <pulsecore/core.h>
#include <pulsecore/module.h>

#define AM_ID_INVALID 65535		/* invalid state in several places */


typedef struct agl_null_sink agl_null_sink;
typedef struct agl_zoneset agl_zoneset;
typedef struct agl_nodeset agl_nodeset;
typedef struct agl_audiomgr agl_audiomgr;
typedef struct agl_routerif agl_routerif;
typedef struct agl_router agl_router;
typedef struct agl_discover agl_discover;
typedef struct agl_tracker agl_tracker;

typedef struct agl_nodeset_resdef agl_nodeset_resdef;
typedef struct agl_nodeset_map agl_nodeset_map;
typedef struct agl_node_card agl_node_card;
typedef struct agl_node_rset agl_node_rset;

typedef struct pa_card_hooks pa_card_hooks;
typedef struct pa_port_hooks pa_port_hooks;
typedef struct pa_sink_hooks pa_sink_hooks;
typedef struct pa_source_hooks pa_source_hooks;
typedef struct pa_sink_input_hooks pa_sink_input_hooks;
typedef struct pa_source_output_hooks pa_source_output_hooks;

typedef struct agl_zone agl_zone;
typedef struct agl_node agl_node;
typedef struct agl_rtgroup agl_rtgroup;
typedef struct agl_connection agl_connection;

typedef struct {
    char *profile;
    uint32_t sink;
    uint32_t source;
} pa_agl_state;

struct userdata {
	pa_core       *core;
	pa_module     *module;
	char          *nsnam;
	agl_zoneset    *zoneset;
	agl_nodeset    *nodeset;
	agl_audiomgr   *audiomgr;
	agl_routerif   *routerif;
	agl_router     *router;
	agl_discover   *discover;
	agl_tracker    *tracker;
	pa_agl_state state;
};

 /* application classes */
typedef enum agl_node_type {
	agl_node_type_unknown = 0,

	agl_application_class_begin,
	agl_radio = agl_application_class_begin,
	agl_player,
	agl_navigator,
	agl_game,
	agl_browser,
	agl_camera,
	agl_phone,                  /**< telephony voice */
	agl_alert,                  /**< ringtone, alarm */
	agl_event,                  /**< notifications */
	agl_system,                 /**< always audible system notifications, events */
	agl_application_class_end,
} agl_node_type;

typedef enum agl_direction {
	agl_direction_unknown = 0,
	agl_input,
	agl_output
} agl_direction;

typedef enum agl_implement {
	agl_implementation_unknown = 0,
	agl_device,
	agl_stream
} agl_implement;

typedef enum agl_location {
	agl_location_unknown = 0,
	agl_internal,
	agl_external
} agl_location;

typedef enum agl_privacy {
	agl_privacy_unknown = 0,
	agl_public,
	agl_private
} agl_privacy;

#endif
