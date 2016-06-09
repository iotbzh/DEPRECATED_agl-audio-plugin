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
#ifndef paaglloopback
#define paaglloopback

#include <stdint.h>

#include <pulsecore/pulsecore-config.h>	/* required for "core.h" */
#include <pulsecore/core.h>		/* required for "PA_LLIST_*" */

#include "userdata.h"

typedef struct pa_loopnode pa_loopnode;
struct pa_loopnode {
	PA_LLIST_FIELDS(pa_loopnode);
	uint32_t   module_index;
	uint32_t   node_index;
	uint32_t   sink_input_index;
	uint32_t   source_output_index;
};

typedef struct pa_loopback {
    PA_LLIST_HEAD(pa_loopnode, loopnodes);
} pa_loopback;

typedef enum {
	PA_LOOPNODE_TYPE_UNKNOWN = 0,
	PA_LOOPNODE_SOURCE,
	PA_LOOPNODE_SINK,
} pa_loopnode_type;

pa_loopback *pa_loopback_init (void);
void pa_loopback_done (struct userdata *, pa_loopback *);

pa_loopnode *pa_loopnode_create (struct userdata *, pa_loopnode_type,
				 uint32_t, uint32_t, uint32_t);
void pa_loopnode_destroy (struct userdata *, pa_loopnode *);

#endif
