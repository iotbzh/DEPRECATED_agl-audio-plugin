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
#ifndef paagldiscover
#define paagldiscover

#include "userdata.h"

struct pa_discover {
    /* criteria for filtering sinks and sources */
	unsigned chmin;    /**< minimum of max channels */
	unsigned chmax;    /**< maximum of max channels */
	bool selected;     /**< for alsa cards: whether to consider the
                                     selected profile alone.
                                     for bluetooth cards: no effect */
	struct {
		pa_hashmap *byname;
		pa_hashmap *byptr;
	} nodes;
};

struct pa_discover *pa_discover_init (struct userdata *);
void pa_discover_done (struct userdata *);

void pa_discover_add_card (struct userdata *, pa_card *);
void pa_discover_remove_card (struct userdata *, pa_card *);
void pa_discover_add_sink (struct userdata *, pa_sink *, bool);
void pa_discover_remove_sink (struct userdata *, pa_sink *);
void pa_discover_add_source (struct userdata *, pa_source *);
void pa_discover_remove_source (struct userdata *, pa_source *);
void pa_discover_add_sink_input (struct userdata *, pa_sink_input *);
void pa_discover_remove_sink_input (struct userdata *, pa_sink_input *);
bool pa_discover_preroute_sink_input (struct userdata *, pa_sink_input_new_data *);
void pa_discover_register_sink_input (struct userdata *, pa_sink_input *);
void pa_discover_register_source_output (struct userdata *, pa_source_output *);

agl_node *pa_discover_find_node_by_key (struct userdata *, const char *);
void pa_discover_add_node_to_ptr_hash (struct userdata *, void *, agl_node *);

#endif
