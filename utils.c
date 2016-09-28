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
#include "userdata.h"
#include "utils.h"
#include "node.h"

#define DEFAULT_NULL_SINK_NAME "null.agl"

#ifndef PA_PROP_PROCESS_ENVIRONMENT
#define PA_PROP_PROCESS_ENVIRONMENT "application.process.environment"
#endif
#define PA_ZONE_NAME_DEFAULT "driver"
#define PA_PROP_ZONE_NAME "zone.name"
#define PA_PROP_ENV_ZONE  PA_PROP_PROCESS_ENVIRONMENT ".AUDIO_ZONE"

#define PA_PROP_ROUTING_CLASS_NAME "routing.class.name"
#define PA_PROP_ROUTING_CLASS_ID   "routing.class.id"
#define PA_PROP_ROUTING_METHOD     "routing.method"

static uint32_t stamp;

struct agl_null_sink {
	char      *name;
	uint32_t   module_index;
	uint32_t   sink_index;
};

agl_null_sink *agl_utils_create_null_sink (struct userdata *u, const char *name)
{
	pa_core      *core;
	pa_module    *module;
	pa_sink      *s, *sink;
	agl_null_sink *ns;
	uint32_t      idx;
	char          args[256];

	pa_assert (u);
	pa_assert_se (core = u->core);

	if (!name)
		name = DEFAULT_NULL_SINK_NAME;	/* default is "null.agl" */

	snprintf (args, sizeof(args), "sink_name=\"%s.%d\" channels=2", name, agl_utils_new_stamp ());
	module = pa_module_load (core, "module-null-sink", args);
	sink = NULL;

	if (!module) {
		pa_log ("failed to load null sink '%s'", name);
		return NULL;
	} else {
		PA_IDXSET_FOREACH(s, core->sinks, idx) {
			if (s->module && s->module == module) {
				sink = s;
				pa_log_info("created agl null sink named '%s'", name);
				break;
			}
		}
	}

	ns = pa_xnew0 (agl_null_sink, 1);
	ns->name = pa_xstrdup (name);
	ns->module_index = module ? module->index : PA_IDXSET_INVALID;
	ns->sink_index = sink ? sink->index : PA_IDXSET_INVALID;

	return ns;
}

void agl_utils_destroy_null_sink (struct userdata *u, agl_null_sink *ns)
{
	pa_core      *core;
	pa_module    *module;

	if (u && (core = u->core)) {
		if ((module = pa_idxset_get_by_index (core->modules, ns->module_index))){
			pa_log_info ("unloading null sink '%s'", ns->name);
			pa_module_unload (module, false);
		}

		pa_xfree (ns->name);
		pa_xfree (ns);
	}
}

pa_sink *agl_utils_get_null_sink (struct userdata *u, struct agl_null_sink *ns)
{
	pa_core *core;
	pa_sink *sink;

	pa_assert (u);
	pa_assert_se ((core = u->core));

	return pa_idxset_get_by_index (core->sinks, ns->sink_index);
}

pa_source *agl_utils_get_null_source (struct userdata *u, struct agl_null_sink *ns)
{
	pa_sink *sink;

	sink = agl_utils_get_null_sink (u, ns);

	return sink ? sink->monitor_source : NULL;
}

void agl_utils_volume_ramp (struct userdata *u, struct agl_null_sink *ns, bool up)
{
	pa_core *core;
	pa_sink *sink;
	pa_sink_input *sinp;
	uint32_t index;
	pa_cvolume_ramp rampvol;
	pa_volume_t newvol;
	long time;

	if (up) {
		newvol = PA_VOLUME_NORM;
		time = 5000;
	} else {
		newvol = PA_VOLUME_NORM *10/100;
		time = 3000;
	}

	pa_assert (u);
	pa_assert_se ((core = u->core));

	sink = agl_utils_get_null_sink (u, ns);
	PA_IDXSET_FOREACH(sinp, core->sink_inputs, index) {
		if (sinp->sink && sinp->sink == sink)
			break;
		sinp = NULL;
	}
	if (!sinp) return;

	pa_cvolume_ramp_set (&rampvol, sinp->volume.channels, PA_VOLUME_RAMP_TYPE_LINEAR,
			     time, newvol);
	pa_sink_input_set_volume_ramp (sinp, &rampvol, true);
}

const char *agl_utils_get_card_name (pa_card *card)
{
	return (card && card->name) ? card->name : "<unknown>";
}

const char *agl_utils_get_card_bus (pa_card *card)
{
	const char *bus = NULL;
	const char *name;

	if (card && !(bus = pa_proplist_gets (card->proplist,PA_PROP_DEVICE_BUS))) {
		name = agl_utils_get_card_name (card);
		if (!strncmp (name, "alsa_card.", 10)) {
			if (!strncmp (name + 10, "pci-", 4))
				bus = "pci";
			else if (!strncmp (name + 10, "platform-", 9))
				bus = "platform";
			else if (!strncmp (name + 10, "usb-", 4))
				bus = "usb";
		}
	}

	return (char *)bus;
}

const char *agl_utils_get_sink_name (pa_sink *sink)
{
	return (sink && sink->name) ? sink->name : "<unknown>";
}

const char *agl_utils_get_source_name (pa_source *source)
{
	return (source && source->name) ? source->name : "<unknown>";
}

const char *agl_utils_get_sink_input_name (pa_sink_input *sinp)
{
	char *name = NULL;

	if (sinp && sinp->proplist) {
		name = (char *)pa_proplist_gets (sinp->proplist, PA_PROP_APPLICATION_NAME);
		if (!name)
			name = (char *)pa_proplist_gets (sinp->proplist, PA_PROP_APPLICATION_PROCESS_BINARY);
		if (!name)
			name = "<unknown>";
	}

        return (const char *)name;
}

const char *agl_utils_get_source_output_name (pa_source_output *sout)
{
	char *name = NULL;

	if (sout && sout->proplist) {
		name = (char *)pa_proplist_gets (sout->proplist, PA_PROP_APPLICATION_NAME);
		if (!name)
			name = (char *)pa_proplist_gets (sout->proplist, PA_PROP_APPLICATION_PROCESS_BINARY);
		if (!name)
			name = "<unknown>";
	}

        return (const char *)name;
}

pa_sink *agl_utils_get_primary_alsa_sink (struct userdata *u)
{
	pa_core *core;
	pa_sink *sink;
	uint32_t idx;

	pa_assert (u);
	pa_assert_se ((core = u->core));

        PA_IDXSET_FOREACH(sink, core->sinks, idx) {
		if (sink->name && strstr (sink->name, "alsa_output"))
			return sink;
        }

	return NULL;
}

pa_sink *agl_utils_get_alsa_sink (struct userdata *u, const char *name)
{
	pa_core *core;
	pa_sink *sink;
	uint32_t idx;

	pa_assert (u);
	pa_assert_se ((core = u->core));

        PA_IDXSET_FOREACH(sink, core->sinks, idx) {
		if (sink->name && strstr (sink->name, "alsa_output")
		               && strstr (sink->name, name))
				return sink;
        }

	return NULL;
}

void agl_utils_init_stamp (void)
{
	stamp = 0;
}

uint32_t agl_utils_new_stamp (void)
{
	return ++stamp;
}

uint32_t agl_utils_get_stamp (void)
{
	return stamp;
}



char *agl_utils_get_zone (pa_proplist *pl, pa_proplist *client_props)
{
	const char *zone;

	pa_assert (pl);

	 /* grab the "zone.name" PA_PROP environment variable ;
	  * otherwise just fall back to default "driver" zone */
	zone = pa_proplist_gets (pl, PA_PROP_ZONE_NAME);
	if (!zone) {
		if (!client_props || !(zone = pa_proplist_gets (client_props, PA_PROP_ENV_ZONE)))
			zone = PA_ZONE_NAME_DEFAULT;		
	}

	return (char *)zone;
}

bool agl_utils_set_stream_routing_properties (pa_proplist *pl, int styp, void *target)
{
	const char *clnam;	/* will become "agl_player" e.g. */
	char clid[32];		/* will become "1" e.g. */
	const char *method;	/* will become "default" (it target is NULL) or "explicit" */

	pa_assert (pl);
	pa_assert (styp >= 0);	/* type different from "agl_node_type_unknown" */

	clnam = agl_node_type_str (styp);
	snprintf (clid, sizeof(clid), "%d", styp);
	method = target ? "explicit" : "default";

	if (pa_proplist_sets (pl, PA_PROP_ROUTING_CLASS_NAME, clnam ) < 0 ||
	    pa_proplist_sets (pl, PA_PROP_ROUTING_CLASS_ID  , clid  ) < 0 ||
	    pa_proplist_sets (pl, PA_PROP_ROUTING_METHOD    , method) < 0)
	{
		pa_log ("failed to set some stream property");
		return false;
	}

	return true;
}

bool agl_utils_unset_stream_routing_properties (pa_proplist *pl)
{
	pa_assert (pl);

	if (pa_proplist_unset (pl, PA_PROP_ROUTING_CLASS_NAME) < 0 ||
	    pa_proplist_unset (pl, PA_PROP_ROUTING_CLASS_ID  ) < 0 ||
	    pa_proplist_unset (pl, PA_PROP_ROUTING_METHOD    ) < 0)
	{
		pa_log ("failed to unset some stream property");
		return false;
	}

	return true;	
}
