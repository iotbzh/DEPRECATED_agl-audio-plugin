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
#ifndef paaglnode
#define paaglnode

#include <stdbool.h>
#include <stdint.h>

#include "userdata.h"
#include "list.h"
#include "loopback.h"

#define APCLASS_DIM  (agl_application_class_end - agl_application_class_begin + 1)

struct pa_nodeset {
	pa_idxset  *nodes;
	pa_hashmap *roles;
	pa_hashmap *binaries;
	const char *class_name[APCLASS_DIM]; /* as much elements as app.classes (see in "userdata.h") */
};

struct pa_nodeset_resdef {
	uint32_t priority;
	struct {
		uint32_t rset;
		uint32_t audio;
	} flags;
};

struct pa_nodeset_map {
	const char *name;
	agl_node_type type;
	const char *role;
	pa_nodeset_resdef *resdef;
};

struct pa_node_card {
	uint32_t  index;
	char     *profile;
};

struct pa_node_rset {
	char     *id;               /**< resource set id, if any */
	bool      grant;            /**< permission to play/render etc */
};

struct agl_node {
	uint32_t       index;     /**< index into nodeset->idxset */
	char          *key;       /**< hash key for discover lookups */
	agl_direction  direction; /**< agl_input | agl_output */
	agl_implement  implement; /**< agl_device | agl_stream */
	pa_client     *client;    /**< matching client pointer (for agl_input nodes only) */
	pa_null_sink  *nullsink;  /**< associated null sink (for agl_input nodes only) */
	pa_loopnode   *loopnode;  /**< associated loopback */
	uint32_t       channels;  /**< number of channels (eg. 1=mono, 2=stereo) */
	agl_location   location;  /**< mir_internal | mir_external */
	agl_privacy    privacy;   /**< mir_public | mir_private */
	agl_node_type  type;      /**< mir_speakers | mir_headset | ...  */
	char          *zone;      /**< zone where the node belong */
	bool           visible;   /**< internal or can appear on UI  */
	bool           available; /**< eg. is the headset connected?  */
	bool           ignore;    /**< do not consider it while routing  */
	bool           localrset; /**< locally generated resource set */
	const char    *amname;    /**< audiomanager name */
	const char    *amdescr;   /**< UI description */
	uint16_t       amid;      /**< handle to audiomanager, if any */
	const char    *paname;    /**< sink|source|sink_input|source_output name */
	uint32_t       paidx;     /**< sink|source|sink_input|source_output index*/
	pa_node_card   pacard;    /**< pulse card related data, if any  */
	const char    *paport;    /**< sink or source port if applies */
	/*pa_muxnode    *mux;*/       /**< for multiplexable input streams only */
    	/*pa_loopnode   *loop;*/      /**< for looped back sources only */
	agl_dlist      rtentries; /**< in device nodes: listhead of nodchain */
	agl_dlist      rtprilist; /**< in stream nodes: priority link (head is in
                                                                   pa_router)*/
	agl_dlist      constrains; /**< listhead of constrains */
	/*mir_vlim       vlim;*/      /**< volume limit */
	pa_node_rset   rset;      /**< resource set info if applies */
	uint32_t       stamp;
	/*scripting_node *scripting;*/ /** scripting data, if any */
};

pa_nodeset *pa_nodeset_init (struct userdata *);
void pa_nodeset_done (struct userdata *);

agl_node *agl_node_create (struct userdata *, agl_node *);
const char *agl_node_type_str (agl_node_type);

agl_node *agl_node_get_from_data (struct userdata *, agl_direction, void *);
agl_node *agl_node_get_from_client (struct userdata *, pa_client *);

#endif
