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
#include "loopback.h"
#include "utils.h"
#include "list.h"

agl_loopback *agl_loopback_init (void)
{
	agl_loopback *loopback = pa_xnew0 (agl_loopback, 1);

	return loopback;
}

void agl_loopback_done (struct userdata *u, agl_loopback *loopback)
{
	agl_loopnode *loop, *n;
	pa_core *core;

	pa_assert_se (core = u->core);

	PA_LLIST_FOREACH_SAFE(loop, n, loopback->loopnodes) {
		pa_module_unload_by_index (core, loop->module_index, false);
    }
}

agl_loopnode *agl_loopnode_create (struct userdata *u, agl_loopnode_type type,
				   uint32_t node_index, uint32_t source_index, uint32_t sink_index)
{
	pa_core *core;
	pa_module *module;
	pa_source *source;
	pa_sink *sink;
	const char *sonam;
	const char *sinam;
	pa_source_output *sout;
	pa_sink_input *sinp;
	agl_loopnode *loopnode;
	int idx;
	char args[256];

	pa_assert (u);
	pa_assert_se (core = u->core);

	source = pa_idxset_get_by_index (core->sources, source_index);
	sink = pa_idxset_get_by_index (core->sinks, sink_index);
	sonam = agl_utils_get_source_name (source);
	sinam = agl_utils_get_sink_name (sink);

	snprintf (args, sizeof(args), "source=\"%s\" sink=\"%s\"", sonam, sinam);
	module = pa_module_load (core, "module-loopback", args);

	if (!module) {
		pa_log ("failed to load loopback for source '%s' & sink '%s'", sonam, sinam);
		return NULL;
	}

	 /* find the sink_input/source_output couple generated but the module we just loaded */
	PA_IDXSET_FOREACH(sout, core->source_outputs, idx) {
		if (sout->module && sout->module == module)
			break;
		sout = NULL;
	}
	PA_IDXSET_FOREACH(sinp, core->sink_inputs, idx) {
		if (sinp->module && sinp->module == module)
			break;
		sinp = NULL;
	}
	if (!sout || !sinp) {
		pa_module_unload (core, module, false);
		return NULL;
	}

	loopnode = pa_xnew0 (agl_loopnode, 1);
	loopnode->module_index = module->index;
	loopnode->source_output_index = sout->index;
	loopnode->sink_input_index = sinp->index;

	return loopnode;
}

void agl_loopnode_destroy (struct userdata *u, agl_loopnode *loopnode)
{
	pa_core      *core;
	pa_module    *module;

	if (u && (core = u->core)) {
		if ((module = pa_idxset_get_by_index (core->modules, loopnode->module_index))){
			pa_log_info ("unloading loopback");
			pa_module_unload (core, module, false);
		}
		pa_xfree (loopnode);
	}
}
