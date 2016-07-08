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
#include "config.h"
#include "zone.h"

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <json/json.h>

#include <pulsecore/core-util.h>
#include <pulsecore/pulsecore-config.h>

bool use_default_configuration (struct userdata *);

const char *agl_config_file_get_path (const char *dir, const char *file, char *buf, size_t len)
{
	pa_assert (file);
	pa_assert (buf);
	pa_assert (len > 0);

	snprintf (buf, len, "%s/%s", dir, file);

	return buf;
}

bool agl_config_parse_file (struct userdata *u, const char *path)
{
	bool success;

	pa_assert (u);

	if (!path)
		return false;
	else {
		pa_log_info ("parsing configuration file '%s'", path);
		success = agl_config_dofile (u, path);
	}

	if (!success) {
		pa_log_info ("applying builtin default configuration");
		success = use_default_configuration (u);
	}

	return success;
}

bool agl_config_dofile (struct userdata *u, const char *path)
{
	int filefd;
	struct stat filestat;
	void *filemap;
	struct json_object *fjson, *root, *sct, *elt, *selt;
	const char *val;
	int len, i;

	pa_assert (u);
	pa_assert (path);

	filefd = open (path, O_RDONLY);
	if (filefd == -1) {
		pa_log_info ("could not find configuration file '%s'", path);
		return false;
	}
	fstat (filefd, &filestat);

	filemap = mmap (NULL, filestat.st_size, PROT_READ, MAP_PRIVATE, filefd, 0);
	if (filemap == MAP_FAILED) {
		pa_log_info ("could not map configuration file in memory");
		return false;
	}

	 /* is the file a JSON file, and if it is, does it have a "config" root ? */
	fjson = json_tokener_parse (filemap);
	root = json_object_object_get (fjson, "config");
	if (!fjson || !root) {
		pa_log_info ("could not parse JSON configuration file");
		return false;
	}

	 /* [zones] section */
	sct = json_object_object_get (root, "zones");
	if (!sct) return false;
	len = json_object_array_length (sct);
	for (i = 0; i < len; i++) {
		elt = json_object_array_get_idx (sct, i);
		val = json_object_get_string (elt);
		agl_zoneset_add_zone (u, val, (uint32_t)i);
	}

	 /* [rtgroups] section */
	sct = json_object_object_get (root, "rtgroups");
	if (!sct) return false;
	len = json_object_array_length (sct);
	for (i = 0; i < len; i++) {
		const char *name, *type, *card, *accept_fct, *effect_fct;
		elt = json_object_array_get_idx (sct, i);
		 name = json_object_get_string (json_object_object_get (elt, "name"));
		 type = json_object_get_string (json_object_object_get (elt, "type"));
		 card = json_object_get_string (json_object_object_get (elt, "card"));
		 accept_fct = json_object_get_string (json_object_object_get (elt, "accept_fct"));
		 effect_fct = json_object_get_string (json_object_object_get (elt, "effect_fct"));
		agl_router_create_rtgroup (u, pa_streq(type, "OUTPUT") ? agl_output : agl_input,
					      name, card,
					      pa_streq(type, "phone") ? agl_router_phone_accept : NULL,
					      pa_streq(type, "phone") ? agl_router_phone_effect : NULL);
	}

	 /* [classmap] section */
	sct = json_object_object_get (root, "classmap");
	if (!sct) return false;
	len = json_object_array_length (sct);
	for (i = 0; i < len; i++) {
		const char *class, *type, *rtgroup;
		int zone;
		elt = json_object_array_get_idx (sct, i);
		 class = json_object_get_string (json_object_object_get (elt, "class"));
		 type = json_object_get_string (json_object_object_get (elt, "type"));
		 zone = json_object_get_int (json_object_object_get (elt, "zone"));
		 rtgroup = json_object_get_string (json_object_object_get (elt, "rtgroup"));
		agl_router_assign_class_to_rtgroup (u, agl_node_type_from_str (class),
						       zone,
						       pa_streq(type, "OUTPUT") ? agl_output : agl_input,
						       rtgroup);
	}

	 /* [typemap] section */
	sct = json_object_object_get (root, "typemap");
	if (!sct) return false;
	len = json_object_array_length (sct);
	for (i = 0; i < len; i++) {
		const char *id, *type;
		elt = json_object_array_get_idx (sct, i);
		 id = json_object_get_string (json_object_object_get (elt, "id"));
		 type = json_object_get_string (json_object_object_get (elt, "type"));
		agl_nodeset_add_role (u, id, agl_node_type_from_str (type), NULL);
	}

	 /* [priormap] section */
	sct = json_object_object_get (root, "priormap");
	if (!sct) return false;
	len = json_object_array_length (sct);
	for (i = 0; i < len; i++) {
		const char *class;
		int priority;
		elt = json_object_array_get_idx (sct, i);
		 class = json_object_get_string (json_object_object_get (elt, "class"));
		 priority = json_object_get_int (json_object_object_get (elt, "priority"));
		agl_router_assign_class_priority (u, agl_node_type_from_str (class), priority);
	}

	json_object_object_del (fjson, "");
	munmap (filemap, filestat.st_size);
	close (filefd);

	return true;
}


 /* DEFAULT CONFIGURATION PART */

static zone_def zones[] = {
	{ "driver" },
	{ "passenger1" },
	{ "passenger2" },
	{ "passenger3" },
	{ "passenger4" },
	{ NULL }
};

static rtgroup_def rtgroups[] = {
	{ agl_input,
	  "Phone",
	  "PhoneCard",
	  agl_router_phone_accept,
	  agl_router_phone_effect
	},

	{ agl_input,
	  "default",
	  "pci",
	  NULL,
	  NULL
	},

	{ 0, NULL, NULL, NULL, NULL }
};

static classmap_def classmap[] = {
	{ agl_phone,	0, agl_input, "Phone" },
	{ agl_player,	0, agl_input, "default" },
	{ agl_radio,	0, agl_input, "default" },
	{ agl_navigator,0, agl_input, "default" },
	{ agl_event,	0, agl_input, "default" },
	{ agl_node_type_unknown, 0, agl_direction_unknown, NULL }
};

static typemap_def typemap[] = {
	{ "phone", agl_phone },
	{ "music", agl_player },
	{ "radio", agl_radio },
	{ "navi", agl_navigator },
	{ "event", agl_event },
	{ NULL, agl_node_type_unknown }
};

static prior_def priormap[] = {
	{ agl_event,     5 },
	{ agl_phone,     4 },
	{ agl_navigator, 2 },
	{ agl_radio,	 1 },
	{ agl_player,	 1 },
	{ agl_node_type_unknown, 0}
};

bool use_default_configuration (struct userdata *u)
{
	zone_def *z;
	rtgroup_def *r;
	classmap_def *c;
	typemap_def *t;
	prior_def *p;

	pa_assert (u);

	for (z = zones; z->name; z++)
		agl_zoneset_add_zone (u, z->name, (uint32_t)(z - zones));

	for (r = rtgroups; r->name; r++)
		agl_router_create_rtgroup (u, r->type, r->name, r->node_desc,
					      r->accept, r->effect);

	for (c = classmap; c->rtgroup; c++)
		agl_router_assign_class_to_rtgroup (u, c->class, c->zone,
						       c->type, c->rtgroup);

	for (t = typemap; t->id; t++) 
		agl_nodeset_add_role (u, t->id, t->type, NULL);

	for (p = priormap; p->class; p++)
		agl_router_assign_class_priority (u, p->class, p->priority);

	return true;
}
