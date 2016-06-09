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
#include <pulsecore/pulsecore-config.h> /* required for headers below */
#include <pulsecore/core-util.h>        /* required for "pa_streq" */

#include "classify.h"

agl_node_type pa_classify_guess_stream_node_type (struct userdata *u,  pa_proplist *pl)
{
	agl_node_type type;
	const char *role;

	pa_assert (u);
	pa_assert (pl);

	role = pa_proplist_gets (pl, PA_PROP_MEDIA_ROLE);

	if (!role)
		type = agl_node_type_unknown;
	else if (pa_streq (role, "radio"))
		type = agl_radio;
	else if (pa_streq (role, "music"))
		type = agl_player;
	else if (pa_streq (role, "navi"))
		type = agl_navigator;
	else if (pa_streq (role, "game"))
		type = agl_game;
	else if (pa_streq (role, "browser"))
		type = agl_browser;
	else if (pa_streq (role, "camera"))
		type = agl_camera;
	else if (pa_streq (role, "phone"))
		type = agl_phone;
	else if (pa_streq (role, "alert"))
		type = agl_alert;
	else if (pa_streq (role, "event"))
		type = agl_event;
	else if (pa_streq (role, "system"))
		type = agl_system;
	else
		type = agl_player;

	return type;
}
