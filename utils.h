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
#ifndef paaglutils
#define paaglutils

#include <pulsecore/core.h>

#include "userdata.h"

struct pa_null_sink;

struct pa_null_sink *pa_utils_create_null_sink (struct userdata *, const char *);
void pa_utils_destroy_null_sink (struct userdata *, struct pa_null_sink *);
pa_sink *pa_utils_get_null_sink (struct userdata *, struct pa_null_sink *);
pa_source *pa_utils_get_null_source (struct userdata *, struct pa_null_sink *);

 /* general helper functions */ 
const char *pa_utils_get_card_name (pa_card *);
const char *pa_utils_get_card_bus (pa_card *);
const char *pa_utils_get_sink_name (pa_sink *);
const char *pa_utils_get_source_name (pa_source *);
const char *pa_utils_get_sink_input_name (pa_sink_input *);
const char *pa_utils_get_source_output_name (pa_source_output *);
pa_sink *pa_utils_get_primary_alsa_sink (struct userdata *);
void pa_utils_init_stamp (void);
uint32_t pa_utils_new_stamp (void);
uint32_t pa_utils_get_stamp (void);

 /* AM-oriented helper functions */
char *pa_utils_get_zone (pa_proplist *, pa_proplist *);
bool pa_utils_set_stream_routing_properties (pa_proplist *, int, void *);
bool pa_utils_unset_stream_routing_properties (pa_proplist *);

#endif
