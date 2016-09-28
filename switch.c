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
#include <pulsecore/pulsecore-config.h>	/* required for headers below */
#include <pulsecore/core-util.h>	/* requred for "pa_streq" */
#include <pulsecore/namereg.h>		/* for PA_NAMEREG_SOURCE */

#include "utils.h"
#include "switch.h"
#include "node.h"

bool agl_switch_setup_link (struct userdata *u, agl_node *from, agl_node *to)
{
	pa_core *core;
	pa_sink *sink;
	pa_source *source;

	pa_assert (u);
	pa_assert_se (core = u->core);

	/* EXPLICIT ROUTES/DEFAULT ROUTES */

	/* 1) EXPLICIT ROUTES : "FROM" AND "TO" ARE DEFINED */
	if (from && to) {
		pa_assert (from);
		pa_assert (to);

		switch (from->implement) {
			/* STREAM SOURCE */
			case agl_stream:
			switch (to->implement) {
				/* STREAM TO STREAM : NOT IMPLEMENTED */
				case agl_stream:
					pa_log_debug ("routing to streams not implemented");
					break;
				/* STREAM TO DEVICE : OK */
				case agl_device:
					//if (!setup_explicit_stream2dev_link (u, from, to))
					//	return false;
					sink = agl_utils_get_alsa_sink (u, to->paname);
					if (!sink) {
						pa_log("sink output not found!!!!");
						sink = agl_utils_get_primary_alsa_sink (u);
						//break;
					}
					source = agl_utils_get_null_source (u, from->nullsink);
					from->loopnode = agl_loopnode_create (u, AGL_LOOPNODE_SINK, from->index, source->index, sink->index);
					break;
				/* DEFAULT */
				default:
					pa_log ("can't setup link: invalid sink node");
					return false;
			}
			break;

			/* DEVICE SOURCE : NOT IMPLEMENTED */
			case agl_device:
				pa_log_debug("input device routing is not implemented yet");
				break;

			/* DEFAULT */
			default:
				pa_log ("can't setup link: invalid sink node");
				return false;
		}
	}

	/* 2) DEFAULT ROUTES : EITHER ONE OF "FROM" AND "TO" ARE DEFINED */
	else {
		pa_assert (from || to);

		/* "TO" IS DEFINED */
		if (to) {
			switch (to->implement) {
				/* STREAM DESTINATION */
				case agl_stream:
				switch (from->implement) {
					/* STREAM TO STREAM : NOT IMPLEMENTED */
					case agl_stream:
						pa_log_debug ("routing to streams not implemented");
						break;
					/* DEVICE TO STREAM : OK */
					case agl_device:
						//if (!setup_default_dev2stream_link(u, from, to))
 						//	return false;
						break;
					/* DEFAULT */
					default:
						pa_log ("can't setup link: invalid sink node");
						return false;
				}
				break;

				/* DEVICE DESTINATION */
				case agl_device:
				switch (from->implement) {
					/* STREAM TO DEVICE : OK */
					case agl_stream:
						sink = agl_utils_get_alsa_sink (u, to->paname);
						if (!sink) break;
						source = agl_utils_get_null_source (u, from->nullsink);

						from->loopnode = agl_loopnode_create (u, AGL_LOOPNODE_SINK, from->index, source->index, sink->index);
						break;
					/* DEVICE TO DEVICE : OK */
					case agl_device:
						//if (!setup_default_dev2dev_link (u, from, to))
						//	return false;
						break;
					/* DEFAULT */
					default:
						pa_log ("can't setup link: invalid source node");
						return false;
				}
				break;
				/* } */

				/* DEFAULT DESTINATION : NULL */
				default:
					pa_log ("can't setup link");
					return false;
			}
		}

		/* ONLY "FROM" IS DEFINED */
		else {
			/* only works with a stream, use default input prerouting */
			if (from->implement == agl_device) {
				pa_log_debug ("default routing for a device input is not supported yet");
				return false;
			}
			/* (the rest supposes "from->implement == agl_stream") */
			/* refuse unknown node types for default routing */
			if (from->type == agl_node_type_unknown) {
				pa_log_debug ("default routing for unknown node type is refused");
				return false;
			}

			sink = agl_utils_get_primary_alsa_sink (u);
			source = agl_utils_get_null_source (u, from->nullsink);
			from->loopnode = agl_loopnode_create (u, AGL_LOOPNODE_SINK, from->index, source->index, sink->index);
		}
	}

	//pa_log_debug ("link %s => %s is established", from->amname, to->amname);

	return true;
}

bool agl_switch_teardown_link (struct userdata *u, agl_node *from, agl_node *to)
{
	pa_core *core;

	pa_assert (u);
	pa_assert_se (core = u->core);

	pa_assert (from || to);

	/* "TO" IS DEFINED */
	if (to) {

	}
	/* ONLY "FROM" IS DEFINED */
	else {
		/* only works with a stream */
		if (from->implement == agl_device) {
			pa_log_debug ("default routing for a device input is not supported");
			return false;
		}
		/* (the rest supposes "from->implement == agl_stream") */
		if (from->loopnode)
			agl_loopnode_destroy (u, from->loopnode);
		if (from->nullsink)
			agl_utils_destroy_null_sink (u, from->nullsink);
	}

	//pa_log_debug("link %s => %s is torn down", from->amname, to->amname);

	return true;
}


bool set_port (struct userdata *u, agl_node *node)
{
	pa_core *core;
	pa_sink *sink;
	pa_source *source;
	pa_device_port *port;
	void *data = NULL;
	uint32_t paidx = PA_IDXSET_INVALID;

	pa_assert (u);
	pa_assert (node);
	pa_assert (node->paname);
	pa_assert_se (core = u->core);

	if (node->direction != agl_input && node->direction != agl_output)
		return false;
	if (node->implement != agl_device)
		return true;
	if (!node->paport)
		return true;

	if (node->direction == agl_input) {
		source = pa_namereg_get (core, node->paname, PA_NAMEREG_SOURCE);
		if (!source) {
			pa_log ("cannot set port for node '%s': source not found", node->paname);
			return false;
		}

		port = source->active_port;
		/* active port and wanted port already match */
		if (pa_streq (node->paport, port->name))
			return true;

		/* ACTIVE CODE */
		if (pa_source_set_port (source, node->paport, false) < 0)
			return false;

		data = source;
		paidx = source->index;
	}

	if (node->direction == agl_output) {
		sink = pa_namereg_get (core, node->paname, PA_NAMEREG_SINK);
		if (!sink) {
			pa_log ("cannot set port for node '%s': source not found", node->paname);
			return false;
		}

		port = sink->active_port;
		/* active port and wanted port already match */
		if (pa_streq (node->paport, port->name))
			return true;

		/* ACTIVE CODE */
		if (pa_sink_set_port (sink, node->paport, false) < 0)
			return false;

		data = sink;
		paidx = sink->index;
	}

	return true;
}
