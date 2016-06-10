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
#include <pulsecore/pulsecore-config.h>	/* required for "core-util.h" */
#include <pulsecore/core-util.h>	/* requred for "pa_streq" */
#include <pulsecore/device-port.h>	/* required for "card.h" */
#include <pulsecore/card.h>		/* for "struct pa_card", "struct pa_card_profile" */

#include "discover.h"
#include "node.h"
#include "utils.h"
#include "classify.h"
#include "router.h"

#define MAX_CARD_TARGET 4    /* max number of managed sinks/sources per card */
#define MAX_NAME_LENGTH 256  /* max sink/source name length */

static void handle_alsa_card (struct userdata *, pa_card *);
static void handle_alsa_card_sinks_and_sources (struct userdata *, pa_card *, agl_node *, const char *);
static void get_sinks_and_sources_from_profile (pa_card_profile *, char **, char **, char *, int);
static char *get_sink_or_source_from_str (char **, int);
static void handle_card_ports (struct userdata *, agl_node *, pa_card *, pa_card_profile *);
static const char *node_key (struct userdata *, agl_direction,
			     void *, pa_device_port *, char *, size_t);
static agl_node *create_node (struct userdata *, agl_node *, bool *);

struct pa_discover *pa_discover_init (struct userdata *u)
{
	pa_discover *discover = pa_xnew0 (pa_discover, 1);
	discover->chmin = 1;
	discover->chmax = 2;
	discover->selected = true;
	discover->nodes.byname = pa_hashmap_new (pa_idxset_string_hash_func,
		                                 pa_idxset_string_compare_func);
	discover->nodes.byptr  = pa_hashmap_new (pa_idxset_trivial_hash_func,
		                                 pa_idxset_trivial_compare_func);

	return discover;
}

void pa_discover_done (struct userdata *u)
{
	pa_discover *discover;
	void *state;
	agl_node *node;

	if (u && (discover = u->discover)) {
		/*PA_HASHMAP_FOREACH(node, discover->nodes.byname, state) {
			agl_node_destroy(u, node);
		}*/
		pa_hashmap_free (discover->nodes.byname);
		pa_hashmap_free (discover->nodes.byptr);
		pa_xfree (discover);
		u->discover = NULL;
	}
}

void pa_discover_add_card (struct userdata *u, pa_card *card)
{
	const char *bus;

	pa_assert(u);
	pa_assert(card);

	if (!(bus = pa_utils_get_card_bus (card))) {
		pa_log_debug ("ignoring card '%s' due to lack of '%s' property",
			      pa_utils_get_card_name (card), PA_PROP_DEVICE_BUS);
		return;
	}

	if (pa_streq(bus, "pci") || pa_streq(bus, "usb") || pa_streq(bus, "platform")) {
		pa_log_debug ("adding card '%s' thanks to its '%s' property",
			      pa_utils_get_card_name (card), PA_PROP_DEVICE_BUS);
		pa_log_debug ("card  type is '%s'", bus);
		handle_alsa_card (u, card);
		return;
	}
/* this never happens, because "pa_utils_get_card_bus()" never returns "bluetooth",
 * but have this here as a reminder */
#if 0
	else if (pa_streq(bus, "bluetooth")) {
		handle_bluetooth_card(u, card);
		return;
	}
#endif

	pa_log_debug ("ignoring card '%s' due to unsupported bus type '%s'",
		      pa_utils_get_card_name (card), bus);
}

void pa_discover_remove_card (struct userdata *u, pa_card *card)
{
	const char  *bus;
	pa_discover *discover;
	agl_node    *node;
	void        *state;

	pa_assert (u);
	pa_assert (card);
	pa_assert_se (discover = u->discover);

	if (!(bus = pa_utils_get_card_bus(card)))
		bus = "<unknown>";

	/*PA_HASHMAP_FOREACH(node, discover->nodes.byname, state) {
	if (node->implement == agl_device &&
	    node->pacard.index == card->index)
	{
		if (pa_streq(bus, "pci") || pa_streq(bus, "usb") || pa_streq(bus, "platform"))
			agl_constrain_destroy (u, node->paname);

		destroy_node(u, node);
	}*/

  /* this never happens, because "pa_utils_get_card_bus()" never returns "bluetooth",
  * but have this here as a reminder */
#if 0
    if (pa_streq(bus, "bluetooth"))
        agl_constrain_destroy(u, card->name);	
#endif
}

void pa_discover_add_sink (struct userdata *u, pa_sink *sink, bool route)
{
	pa_core *core;
	pa_discover *discover;
	pa_module *module;
	pa_card *card;
	char kbf[256];
	const char *key;
	agl_node *node;
	pa_source *null_source;

	pa_assert (u);
	pa_assert (sink);
	pa_assert_se (core = u->core);
	pa_assert_se (discover = u->discover);

	module = sink->module;
	card = sink->card;

	if (card) {
		 /* helper function verifying that sink direction is input/output */
		key = node_key (u, agl_output, sink, NULL, kbf, sizeof(kbf));
		if (!key) return;
		pa_log_debug ("Sink key: %s", key);
		node = pa_discover_find_node_by_key (u, key);
		if (!node) {	/* ALWAYS NULL, IF CALL "handle_card_ports" FROM "handle_alsa_card" !!! */
			if (u->state.profile)
				pa_log_debug ("can't find node for sink (key '%s')", key);
			else {	/* how do we get here ? not in initial setup */
				u->state.sink = sink->index;
				pa_log_debug ("BUG ! Did you call handle_card_ports() ?");
			}
			return;
		}
		pa_log_debug("node for '%s' found (key %s). Updating with sink data",
			     node->paname, node->key);
        	node->paidx = sink->index;
		node->available = true;
		pa_discover_add_node_to_ptr_hash (u, sink, node);

#if 0
		/* loopback part : it is a device node, use "module-loopback" to make its */
		if (node->implement == agl_device) {
			null_source = pa_utils_get_null_source (u);
			if (!null_source) {
				pa_log ("Can't load loopback module: no initial null source");
				return;
			}
		}
#endif
	}
}


static void handle_alsa_card (struct userdata *u, pa_card *card)
{
	agl_node data;
	const char *cnam;	/* PulseAudio name */
	const char *cid;	/* short PulseAudio name (with "alsa_name." stripped) */
	const char *alsanam;	/* ALSA name */
	const char *udd;	/* managed by udev ("1" = yes) */

	memset (&data, 0, sizeof(data));
	data.zone = pa_utils_get_zone (card->proplist, NULL);
	data.visible = true;
	data.amid = AM_ID_INVALID;
	data.implement = agl_device;		/* this node is a physical device */
	data.paidx = PA_IDXSET_INVALID;
	data.stamp = pa_utils_get_stamp ();	/* each time incremented by one */

	cnam = pa_utils_get_card_name (card);	/* PulseAudio name */
	cid = cnam + 10;			/* PulseAudio short name, with "alsa_card." prefix removed */
	alsanam = pa_proplist_gets (card->proplist, "alsa.card_name"); /* ALSA name */
	udd = pa_proplist_gets (card->proplist, "module-udev-detect.discovered");

	data.amdescr = (char *)alsanam;
	data.pacard.index = card->index;

	pa_log_debug ("Sound card zone: %s", data.zone);
	pa_log_debug ("Sound card stamp: %d", data.stamp);
	pa_log_debug ("PulseAudio card name: %s", cnam);
	pa_log_debug ("PulseAudio short card name: %s", cid);
	pa_log_debug ("ALSA card name: %s", alsanam);
	if (udd)
		pa_log_debug ("ALSA card detected by udev: %s", udd);

	/* WITH THE TIZEN MODULE, ONLY UDEV-MANAGED CARDS ARE ACCEPTED
	 * NO MATTER, TREAT STATIC CARDS THE SAME WAY HERE.. */
	/*if (!udd || (udd && !pa_streq(udd, "1"))
		pa_log_debug ("Card not accepted, not managed by udev\n");*/

	handle_alsa_card_sinks_and_sources (u, card, &data, cid);
}
                                              ;
static void handle_alsa_card_sinks_and_sources (struct userdata *u, pa_card *card, agl_node *data, const char *cardid)
{
	pa_discover *discover; /* discovery restrictions (max channels...) */
	pa_card_profile *prof;
	void *state;
	char *sinks[MAX_CARD_TARGET+1];	  /* discovered sinks array */
	char *sources[MAX_CARD_TARGET+1]; /* discovered sources array */
	char namebuf[MAX_NAME_LENGTH+1]; /* discovered sink/source name buf.*/
	const char *alsanam;
	char paname[MAX_NAME_LENGTH+1];
	char amname[MAX_NAME_LENGTH+1];
	int i, j, k;

	pa_assert (card);
	pa_assert (card->profiles);
	pa_assert_se (discover = u->discover);

	alsanam = pa_proplist_gets (card->proplist, "alsa.card_name");
	data->paname = paname;
	data->amname = amname;
	data->amdescr = (char *)alsanam;
	data->pacard.index = card->index;

	PA_HASHMAP_FOREACH(prof, card->profiles, state) {
		 /* TODO : skip selected profile here */

		 /* skip profiles withoutqx sinks/sources */
		if (!prof->n_sinks && !prof->n_sources)
			continue;
		 /* skip profiles with too few/many channels */
		if (prof->n_sinks &&
		    (prof->max_sink_channels < discover->chmin ||
		     prof->max_sink_channels  > discover->chmax))
			continue;
		if (prof->n_sources &&
		    (prof->max_source_channels < discover->chmin ||
		     prof->max_source_channels  > discover->chmax))
			continue;

		 /* VALID PROFILE, STORE IT */
		pa_log_debug ("Discovered valid profile: %s", prof->name);
		data->pacard.profile = prof->name;
		 /* NOW FILLING SINKS/SOURCE ARRAYS WITH PROFILE DATA */
		get_sinks_and_sources_from_profile (prof, sinks, sources, namebuf, sizeof(namebuf));

		 /* OUTPUT DIRECTION, SINKS */
		data->direction = agl_output;
		data->channels = prof->max_sink_channels;
		for (i = 0; sinks[i]; i++) {
			pa_log_debug ("Discovered valid sink #%s on card %s", sinks[i], cardid);
			snprintf(paname, sizeof(paname), "alsa_output.%s.%s", cardid, sinks[i]);
			handle_card_ports(u, data, card, prof);
		}

		 /* INPUT DIRECTION, SOURCES */
		data->direction = agl_input;
		data->channels = prof->max_source_channels;
		for (i = 0; sources[i]; i++) {
			pa_log_debug ("Discovered valid source #%s on card %s", sources[i], cardid);
			snprintf(paname, sizeof(paname), "alsa_input.%s.%s", cardid, sinks[i]);
			handle_card_ports(u, data, card, prof);
		}
	}
}

static void get_sinks_and_sources_from_profile (pa_card_profile *prof, char **sinks, char **sources, char *buf, int buflen)
{
	char *p = buf;
	int i = 0;
	int j = 0;

	pa_assert (prof->name);

	strncpy (buf, prof->name, (size_t)buflen);
	buf[buflen-1] = '\0';

	memset (sinks, 0, sizeof(char *) * (MAX_CARD_TARGET+1));
	memset (sources, 0, sizeof(char *) * (MAX_CARD_TARGET+1));

	do {
		if (!strncmp (p, "output:", 7)) {
			if (i >= MAX_CARD_TARGET) {
                		pa_log_debug ("number of outputs exeeds the maximum %d in "
				              "profile name '%s'", MAX_CARD_TARGET, prof->name);
                		return;
			}
			sinks[i++] = get_sink_or_source_from_str (&p, 7);
		} else if (!strncmp (p, "input:", 6)) {
 			if (j >= MAX_CARD_TARGET) {
				pa_log_debug ("number of inputs exeeds the maximum %d in "
				              "profile name '%s'", MAX_CARD_TARGET, prof->name);
				return;
			}
			sources[j++] = get_sink_or_source_from_str (&p, 6);
		} else {
			pa_log ("%s: failed to parse profile name '%s'",
			        __FILE__, prof->name);
			return;
		}
	} while (*p);
}

static char *get_sink_or_source_from_str (char **string_ptr, int offs)
{
	char c, *name, *end;

	name = *string_ptr + offs;

	for (end = name;  (c = *end);   end++) {
		if (c == '+') {
			*end++ = '\0';
			break;
		}
	}

	*string_ptr = end;

	return name;
}

static void handle_card_ports (struct userdata *u, agl_node *data, pa_card *card, pa_card_profile *prof) {
	agl_node *node = NULL;
	pa_device_port *port;
	void *state;
        bool created;
	bool have_ports = false;
	char key[MAX_NAME_LENGTH+1];
	const char *amname = data->amname;

	pa_assert (u);
	pa_assert (data);
	pa_assert (card);
	pa_assert (prof);

	if (card->ports) {
		PA_HASHMAP_FOREACH (port, card->ports, state) {
			if (port->profiles &&
			    pa_hashmap_get (port->profiles, prof->name) &&
			    ((port->direction == PA_DIRECTION_INPUT && data->direction == agl_input)||
			    (port->direction == PA_DIRECTION_OUTPUT && data->direction == agl_output))) {
				have_ports = true;
				snprintf (key, sizeof(key), "%s@%s", data->paname, port->name);

				data->key = key;
				data->available = (port->available != PA_AVAILABLE_NO);
				data->type = 0;
				data->amname = amname;
				data->paport = port->name;

				printf ("Key : %s\n", key);

				 /* this is needed to fill the "port->type" field */
				//pa_classify_node_by_card (data, card, prof, port);

				 /* this is needed to complete the "pa_discover_add_sink" first pass */
				node = create_node (u, data, &created);
			}
		}
	}

	amname = "";
	data->amname = amname;
}

static const char *node_key (struct userdata *u, agl_direction direction,
			     void *data, pa_device_port *port, char *buf, size_t len) {
	char *type;			/* "sink" or "source" */
	const char *name;		/* sink or source name */
	pa_card *card;
	pa_card_profile *profile;
	const char *profile_name;
	const char *bus;
	char *key = NULL;

	pa_assert (u);
	pa_assert (data);
	pa_assert (buf);
	pa_assert (direction == agl_input || direction == agl_output);

	if (direction == agl_output) {
		pa_sink *sink = data;
		type = pa_xstrdup ("sink");
		name = pa_utils_get_sink_name (sink);
		card = sink->card;
		if (!port)
			port = sink->active_port;
	} else {
		pa_source *source = data;
		type = pa_xstrdup ("source");
		name = pa_utils_get_source_name (source);
		card = source->card;
		if (!port)
			port = source->active_port;
	}

	pa_log_debug ("Node type (sink/source): %s", type);
	pa_log_debug ("Node name: %s", name);

	if (!card)
		return NULL;

	pa_assert_se (profile = card->active_profile);
	if (!u->state.profile) {
		pa_log_debug ("profile is now '%s'", profile->name);
		profile_name = profile->name;
	} else {
		pa_log_debug ("state.profile is not null. '%s' supresses '%s'",
			      u->state.profile, profile->name);
		profile_name = u->state.profile;
	}

	if (!(bus = pa_utils_get_card_bus (card))) {
		pa_log_debug ("ignoring card '%s' due to lack of '%s' property",
			      pa_utils_get_card_name (card), PA_PROP_DEVICE_BUS);
		return NULL;
	}

	if (pa_streq(bus, "pci") || pa_streq(bus, "usb") || pa_streq(bus, "platform")) {
		if (!port)
			key = (char *)name;
		else {
			key = buf;
			snprintf (buf, len, "%s@%s", name, port->name);
		}
	}
	/* we do not handle Bluetooth yet, and the function never returns it */
	/*else if (pa_streq(bus, "bluetooth")) {
	}*/
	
	return (const char *)key;
}

agl_node *pa_discover_find_node_by_key (struct userdata *u, const char *key)
{
	pa_discover *discover;
	agl_node *node;

	pa_assert (u);
	pa_assert_se (discover = u->discover);

	if (key)
		node = pa_hashmap_get (discover->nodes.byname, key);
	else
		node = NULL;

	return node;
}

void pa_discover_add_node_to_ptr_hash (struct userdata *u, void *ptr, agl_node *node)
{
	pa_discover *discover;

	pa_assert (u);
	pa_assert (ptr);
	pa_assert (node);
	pa_assert_se (discover = u->discover);

	pa_hashmap_put (discover->nodes.byptr, ptr, node);
}

static agl_node *create_node (struct userdata *u, agl_node *data, bool *created_ret)
{
	pa_discover *discover;
	agl_node *node;
	bool created;

	pa_assert (u);
	pa_assert (data);
    	pa_assert (data->key);
	pa_assert (data->paname);
	pa_assert_se (discover = u->discover);

	if ((node = pa_hashmap_get (discover->nodes.byname, data->key))) {
		pa_log_debug ("No need to create this node");
		created = false;
	} else {
		pa_log_debug ("Creating new node");

		node = agl_node_create (u, data);
		pa_hashmap_put (discover->nodes.byname, node->key, node);

		 /* TODO: registering the new node to the Optional router daemon */
		/* if (node->available)
			pa_audiomgr_register_node (u, node); */

		created = true;
	}

	if (created_ret)
		*created_ret = created;

	return node;
}

void pa_discover_add_source (struct userdata *u, pa_source *source)
{
	static pa_nodeset_resdef def_resdef = {0, {0, 0}};

	pa_core *core;
	pa_discover *discover;
	pa_module *module;
	pa_card *card;
	const char *key;
	char kbf[256];
	agl_node *node;

	pa_assert (u);
	pa_assert (source);
	pa_assert_se (core = u->core);
	pa_assert_se (discover = u->discover);

	module = source->module;
	card = source->card;

	if (card) {
		 /* helper function verifying that sink direction is input/output */
		key = node_key (u, agl_input, source, NULL, kbf, sizeof(kbf));
		if (!key) return;
		pa_log_debug ("Source key: %s", key);
		node = pa_discover_find_node_by_key (u, key);
		if (!node) {	/* VERIFY IF THIS WORKS */
			if (u->state.profile)
				pa_log_debug ("can't find node for source (key '%s')", key);
			else {	/* how do we get here ? not in initial setup */
				u->state.source = source->index;
				pa_log_debug ("BUG !");
			}
			return;
		}
		pa_log_debug("node for '%s' found (key %s). Updating with source data",
			     node->paname, node->key);
        	node->paidx = source->index;
		node->available = true;
		pa_discover_add_node_to_ptr_hash (u, source, node);
	}
}

bool pa_discover_preroute_sink_input(struct userdata *u, pa_sink_input_new_data *data)
{
	pa_core *core;
	pa_sink *sink;
	pa_source *source;
	pa_discover *discover;
	pa_nodeset *nodeset;
	pa_proplist *pl;
	agl_node *node;

	pa_assert (u);
	pa_assert (data);
	pa_assert_se (core = u->core);
	pa_assert_se (discover = u->discover);
	pa_assert_se (nodeset = u->nodeset);
	pa_assert_se (pl = data->proplist);

	 /* is this a valid sink input ? */
	if (!data->client)
		return true;

	 /* is there an existing matching node ? */
	node = agl_node_get_from_data (u, agl_input, data);

	if (!node) {
		 /* create node */
		node = agl_node_create (u, NULL);
		node->direction = agl_input;
		node->implement = agl_stream;
		node->type = pa_classify_guess_stream_node_type (u, pl);
		node->visible = true;
		node->available = true;
		node->ignore = true; /* gets ignored initially */
		node->paname = pa_proplist_gets (pl, PA_PROP_APPLICATION_NAME);
		node->client = data->client;
		 /* add to global nodeset */
		pa_idxset_put (nodeset->nodes, node, &node->index);
	}

	 /* create NULL sink */
	if (!node->nullsink)
		node->nullsink = pa_utils_create_null_sink (u, u->nsnam);
	if (!node->nullsink)
		return false;

	 /* redirect sink input to NULL sink */
	sink = pa_utils_get_null_sink (u, node->nullsink);

	if (pa_sink_input_new_data_set_sink (data, sink, false))
		pa_log_debug ("set sink %u for new sink-input", sink->index);
	else
		pa_log ("can't set sink %u for new sink-input", sink->index);

	return true;
}

void pa_discover_register_sink_input (struct userdata *u, pa_sink_input *sinp)
{
	pa_core *core;
	pa_discover *discover;
	pa_proplist *pl, *client_proplist;
	const char *media;
	const char *name;
	const char *role;
	agl_node_type type;
	agl_node node_data, *node;
	char key[256];
	pa_sink *sink;

	pa_assert (u);
	pa_assert (sinp);
	pa_assert_se (core = u->core);
	pa_assert_se (discover = u->discover);
	pa_assert_se (pl = sinp->proplist);

	media = pa_proplist_gets (sinp->proplist, PA_PROP_MEDIA_NAME);
	if (media) {
		/* special treatment for combine/loopback streams */
		pa_log_debug ("Stream may me combine/loopback, in this case we should ignore it, but use it for now");
		/* return; */
	}

	name = pa_utils_get_sink_input_name (sinp);
	client_proplist = sinp->client ? sinp->client->proplist : NULL;

	pa_log_debug ("registering input stream '%s'", name);

	/* we could autodetect sink type by using: 
	 *	- PA_PROP_APPLICATION_PROCESS_ID;
	 *	- PA_PROP_APPLICATION_PROCESS_BINARY;
	 *	- PA_PROP_APPLICATION_NAME;
	 *	- ...
	but let us assume "agl_player" for now */
	type = agl_player;

	/* this sets our routing properties on the sink, which are :
	 *	#define PA_PROP_ROUTING_CLASS_NAME "routing.class.name"
	 *	#define PA_PROP_ROUTING_CLASS_ID   "routing.class.id"
	 *	#define PA_PROP_ROUTING_METHOD     "routing.method" */
	pa_utils_set_stream_routing_properties (pl, type, NULL);

	/* we now create a new node for this sink */
	memset (&node_data, 0, sizeof(node_data));
	node_data.key = key;
	node_data.direction = agl_input;
	node_data.implement = agl_stream;
	node_data.channels  = sinp->channel_map.channels;
	node_data.type      = type;
	node_data.zone      = pa_utils_get_zone (sinp->proplist, client_proplist);
	node_data.visible   = true;
	node_data.available = true;
	node_data.amname    = pa_proplist_gets (pl, "resource.set.appid");
	node_data.amdescr   = pa_proplist_gets (pl, PA_PROP_MEDIA_NAME);
	node_data.amid      = AM_ID_INVALID;
	node_data.paname    = (char *)name;
	node_data.paidx     = sinp->index;
	/*node_data.rset.id   = pa_utils_get_rsetid (pl, idbuf, sizeof(idbuf));*/

	role = pa_proplist_gets (sinp->proplist, PA_PROP_MEDIA_ROLE);
	/* MAIN PREROUTING DONE HERE ! (this was failing in the samples) */
	/*sink = mir_router_make_prerouting (u, &data, &sinp->channel_map, role, &target);*/

	node = create_node (u, &node_data, NULL);
	pa_assert (node);
	pa_discover_add_node_to_ptr_hash (u, sinp, node);
/*
	if (sink && target) {
		pa_log_debug ("move stream to sink %u (%s)", sink->index, sink->name);

		if (pa_sink_input_move_to (sinp, sink, false) < 0)
			pa_log ("failed to route '%s' => '%s'",node->amname,target->amname);
		else
			pa_audiomgr_add_default_route (u, node, target);
	}*/
}

void pa_discover_register_source_output (struct userdata *u, pa_source_output *sout)
{
	pa_core *core;
	pa_discover *discover;
	pa_proplist *pl, *client_proplist;
	const char *media;
	const char *name;
	const char *role;
	agl_node_type type;
	agl_node node_data, *node;
	char key[256];
	pa_source *source;

	pa_assert (u);
	pa_assert (sout);
	pa_assert_se (core = u->core);
	pa_assert_se (discover = u->discover);
	pa_assert_se (pl = sout->proplist);

	media = pa_proplist_gets (sout->proplist, PA_PROP_MEDIA_NAME);
	if (media) {
		/* special treatment for loopback streams (not combine as with sinks !) */
		pa_log_debug ("Stream may me loopback, in this case we should ignore it, but use it for now");
		/* return; */
	}

	 /* TODO : contrary to "pa_discover_register_sink_input", this function is always called
	  * even if we do not find PA_PROP_MEDIA_NAME (see above). */
	//pa_utils_set_stream_routing_properties (pl, type, NULL);

	name = pa_utils_get_source_output_name (sout);
	client_proplist = sout->client ? sout->client->proplist : NULL;

	pa_log_debug("registering output stream '%s'", name);
}

void pa_discover_add_sink_input (struct userdata *u, pa_sink_input *sinp) 
{
	pa_core *core;
	agl_node *node;

	pa_assert (u);
	pa_assert (sinp);
	pa_assert_se (core = u->core);

	if (!sinp->client)
		return;

	 /* is there an existing matching node ? */
	node = agl_node_get_from_client (u, sinp->client);
	if (!node) return;

	 /* start the default routing */
	implement_default_route (u, node, NULL, pa_utils_new_stamp ());
}
