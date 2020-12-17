/* *****************************************************************************
Copyright (c) 2016-2017, The Regents of the University of California (Regents).
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of the FreeBSD Project.

REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS
PROVIDED "AS IS". REGENTS HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT,
UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*************************************************************************** */ 
  
// Author: Claudio Perez

#include <stdio.h>
#include <string.h>

#include "jansson.h"


void cartesianProduct(json_t *prefix, json_t *pools, size_t index, json_t *res)
{
    if (index >= json_array_size(pools))
        json_array_append(res, prefix);
    else {
        json_t *pool = json_array_get(pools, index);

        int i;
        json_t *item;  // loop variables
        json_array_foreach(pool, i, item) {
            json_t *array = json_copy(prefix);
            json_array_append(array, item);
            cartesianProduct(array, pools, index + 1, res);
            json_decref(array);
        }
    }
}

json_t *product(json_t *pools) 
{
    json_t *res = json_array();
    json_t *prefix = json_array();
    cartesianProduct(prefix, pools, 0, res);
    json_decref(prefix);
    return res;
}

json_t *objProduct(json_t *obj, json_t *parent)
{
    json_t *keys = json_array();
    json_t *val;
    const char *key;
    json_object_foreach(obj, key, val) json_array_append_new(keys, json_string(key));

    json_t *hash_values = json_array();
    json_t *v;
    const char *k;
    json_object_foreach(obj, k, v) json_array_append(hash_values, v);
    json_t *prod = product(hash_values);
    json_decref(hash_values);

    json_t *out = json_array();

    size_t i;
    json_t *values;
    json_array_foreach(prod, i, values) {
        json_t *var = json_deep_copy(parent);
        size_t j;
        json_t *key_s;
        json_array_foreach(keys, j, key_s)
            json_object_set(var, json_string_value(key_s), json_array_get(values, j));

        json_array_append(out, var);
        json_decref(var);
    }
    json_decref(prod);
    json_decref(keys);
    return out;
}

json_t *parseVariations(json_t *schema, json_t *parent, const char *parent_key)
{
    json_t *variations = json_pack("[]");
    json_t *properties = json_object_get(schema, "properties");
    json_t *schema_variations = json_object_get(schema, "oneOf");

    int i;
    json_t *schema_variation;
    if (schema_variations)
        json_array_foreach(schema_variations, i, schema_variation) {
            json_t *variation_parent = json_deep_copy(parent);
            json_t *id_props = json_object_get(schema_variation, "properties");
            json_t *dp_props = json_object_get(schema_variation, "required");

            const char *prop_key;
            json_t *prop_val;
            json_object_foreach(id_props, prop_key, prop_val) {
                char key[strlen(parent_key) + strlen(prop_key) + 2];
                int l = 0;
                while( key[l] = *(parent_key+l) ) l++;
                if (parent_key != "\0") strcat(key, ".");
                strcat(key, prop_key);
                json_object_set(variation_parent, key, json_object_get(prop_val, "const"));
            }

            json_t *prop_schemas = json_pack("[]");
            json_t *enum_props = json_pack("{}");

            size_t j;
            json_t *prop_name;
            json_array_foreach(dp_props, j, prop_name) {
                const char *prop_key = json_string_value(prop_name);
                json_t *prop = json_object_get(properties, prop_key);
               
                char key[strlen(parent_key) + strlen(prop_key) + 2];
                int l = 0;
                while( key[l] = *(parent_key+l) ) l++;
                if (parent_key != "\0") strcat(key, ".");
                strcat(key, prop_key);

                json_t *STR_OBJECT = json_string("object");
                if (json_equal(json_object_get(prop, "type"), STR_OBJECT))
                    json_array_append(prop_schemas, prop_name);

                else if (json_is_array(json_object_get(prop, "enum")))
                    json_object_set(enum_props, key, json_object_get(prop, "enum"));

                else if (json_object_get(prop, "default"))
                    json_object_set(variation_parent, key, json_object_get(prop, "default"));
                else {
                    printf("ERROR: Ambiguous Schema.");
                    exit(-1);
                }
                json_decref(STR_OBJECT);
            }
            int num_schemas = json_array_size(prop_schemas);
            if (num_schemas) {
                json_t *prop_name;
                int j;
                json_array_foreach(prop_schemas, j, prop_name) {
                    const char *prop_key = json_string_value(prop_name);
                    json_t *prop = json_object_get(properties, prop_key);

                    json_t *prop_schema_variations = parseVariations(prop, variation_parent, prop_key);
                    json_array_extend(variations, prop_schema_variations);
                    json_decref(prop_schema_variations);
                }
            } else if (json_object_size(enum_props)) {
                json_t *enum_variations = objProduct(enum_props, variation_parent);
                json_array_extend(variations, enum_variations);
                json_decref(enum_variations);
            } else
                json_array_append(variations, variation_parent);

            json_decref(variation_parent);
            json_decref(prop_schemas);
            json_decref(enum_props);
        }
    else {
        json_t *variation_parent = json_deep_copy(parent);
        json_t *enum_props = json_pack("{}");

        const char *prop_key;
        json_t *prop;
        json_object_foreach(properties, prop_key, prop) {
            char key[strlen(parent_key) + strlen(prop_key) + 2];
            strcpy(key, parent_key);
            if (parent_key) strcat(key, ".");
            strcat(key, prop_key);
            json_t *STR_OBJECT = json_string("object");
            if (json_is_array(json_object_get(prop, "enum")))
                json_object_set(enum_props, key, json_object_get(prop, "enum"));
            else if (!json_equal(json_object_get(prop, "type"), STR_OBJECT) && json_object_get(prop, "default"))
                json_object_set(variation_parent, key, json_object_get(prop, "default"));
            json_decref(STR_OBJECT);
        }
        json_t *enum_variations = objProduct(enum_props, variation_parent);
        json_array_extend(variations, enum_variations);

        json_decref(enum_variations);
        json_decref(enum_props);
        json_decref(variation_parent);
    }
    return variations;
}

