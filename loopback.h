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

typedef struct agl_loopnode agl_loopnode;
struct agl_loopnode {
	PA_LLIST_FIELDS(agl_loopnode);
	uint32_t   module_index;
	uint32_t   node_index;
	uint32_t   sink_input_index;
	uint32_t   source_output_index;
};

typedef struct agl_loopback {
    PA_LLIST_HEAD(agl_loopnode, loopnodes);
} agl_loopback;

typedef enum {
	AGL_LOOPNODE_TYPE_UNKNOWN = 0,
	AGL_LOOPNODE_SOURCE,
	AGL_LOOPNODE_SINK,
} agl_loopnode_type;

agl_loopback *agl_loopback_init (void);
void agl_loopback_done (struct userdata *, agl_loopback *);

agl_loopnode *agl_loopnode_create (struct userdata *, agl_loopnode_type,
				   uint32_t, uint32_t, uint32_t);
void agl_loopnode_destroy (struct userdata *, agl_loopnode *);

#endif
