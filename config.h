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
#ifndef paaglconfig
#define paaglconfig

#include "userdata.h"
#include "router.h"

 /* ZONES ("driver", "passenger1"...) */
typedef struct {
	const char *name;
} zone_def;

 /* ROUTING GROUPS ("default" card, "phone" card...) */
typedef struct {
	agl_direction type;		/* agl_input/agl_output */
	const char *name;
	agl_rtgroup_accept_t accept;
	agl_rtgroup_compare_t compare;
} rtgroup_def;

 /* CLASS MAP (agl_phone="phone" card routing group...) */
typedef struct {
	agl_node_type class;		/* agl_device/agl_stream */
	uint32_t zone;
	agl_direction type;		/* agl_input/agl_output */
	const char *rtgroup;
} classmap_def;

 /* TYPE MAP ("event"=agl_event, "music"=agl_player...) */
typedef struct {
	const char *id;
	agl_node_type type;
} typemap_def;

 /* PRIORITY MAP (agl_event=5, agl_phone=4, [...] agl_player=1...) */
typedef struct {
	agl_node_type class;
	int priority;
} prior_def;

const char *agl_config_file_get_path (const char *, const char *, char *, size_t);
bool agl_config_parse_file (struct userdata *, const char *);
bool agl_config_dofile (struct userdata *, const char *);

#endif
