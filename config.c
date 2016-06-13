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
#include "config.h"

const char *pa_config_file_get_path (const char *dir, const char *file, char *buf, size_t len)
{
	pa_assert (file);
	pa_assert (buf);
	pa_assert (len > 0);

	snprintf (buf, len, "%s/%s", dir, file);

	return buf;
}

bool pa_config_parse_file (struct userdata *u, const char *path)
{
	bool success;

	pa_assert (u);

	if (!path)
		return false;
	else {
		pa_log_info ("parsing configuration file '%s'", path);
		success = pa_config_dofile (u, path);
	}

	if (!success) {
		pa_log_info ("applying builtin default configuration");
		//success = use_default_configuration (u);
	}

	return success;
}

bool pa_config_dofile (struct userdata *u, const char *path)
{
	/* TODO */
	return false;
}


 /* DEFAULT CONFIGURATION PART */

static zone_def zones[] = {
	{ "driver" },
	{ "passenger1" },
	{ "passenger2" },
	{ "passenger3" },
	{ "passenger4" },
	{ NULL }
};


static rtgroup_def rtgroups[] = {
	{ agl_input,
	  "phone",
	  agl_router_phone_accept,
	  agl_router_phone_compare
	},

	{ 0, NULL, NULL, NULL }
};

static classmap_def classmap[] = {
	{ agl_phone,	0, agl_output, "phone" },
	{ agl_player,	0, agl_output, "default" },
	{ agl_radio,	0, agl_output, "default" },
	{ agl_navigator,0, agl_output, "default" },
	{ agl_event,	0, agl_output, "default" },
	{ agl_node_type_unknown, 0, agl_direction_unknown, NULL }
};

static typemap_def typemap[] = {
	{ "phone", agl_phone },
	{ "music", agl_player },
	{ "radio", agl_radio },
	{ "navi", agl_navigator },
	{ "event", agl_event },
	{ NULL, agl_node_type_unknown }
};

static prior_def priormap[] = {
	{ agl_event,     5 },
	{ agl_phone,     4 },
	{ agl_navigator, 2 },
	{ agl_radio,	 1 },
	{ agl_player,	 1 },
	{ agl_node_type_unknown, 0}
};

bool use_default_configuration (struct userdata *u)
{
	zone_def *z;
	rtgroup_def *r;
	classmap_def *c;
	typemap_def *t;
	prior_def *p;

	return true;
}
