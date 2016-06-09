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
#ifndef paaglswitch
#define paaglswitch

#include "userdata.h"

bool agl_switch_setup_link (struct userdata *, agl_node *, agl_node *, bool);
bool agl_switch_teardown_link (struct userdata *, agl_node *, agl_node *);

/*pa_source *setup_device_input(struct userdata *, agl_node *);*/
/*pa_sink   *setup_device_output(struct userdata *, agl_node *);*/

bool set_port (struct userdata *, agl_node *);

#endif
