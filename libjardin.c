
#include <stdio.h>
#include <string.h>
#include "jansson.h"

// void cartesianProduct(json_t *prefix, json_t *pools, size_t offset, json_t *res) {
//     //3
//   if (offset >= json_array_size(pools)) {
//     json_array_append(res ,prefix);
//     return;
//   }
//   json_t *pool = json_array_get(pools, offset);

//   int i; json_t *item;
//   json_array_foreach(pool,i,item) {
//     json_t *array = json_copy(prefix);

//     json_array_append_new(array,item);
//     // json_t *sub = json_pack("[o]", item);
//     // int k; json_t *item2;
//     // json_array_foreach(sub,k,item2)
//     //     json_array_append(array, item2);
//     // json_decref(sub);

//     cartesianProduct(array, pools, offset + 1, res);
//   }
// }


void cartesianProduct(json_t *prefix, json_t *pools, size_t offset, json_t *res) {
    //3
  if (offset >= json_array_size(pools)) {
    json_array_append(res ,prefix);
    return;
  }
  json_t *pool = json_array_get(pools, offset);

  int i; json_t *item;
  json_array_foreach(pool,i,item) {
    json_t *array = json_copy(prefix);
    json_array_append_new(array,item);
    cartesianProduct(array, pools, offset + 1, res);
  }
}

json_t *product(json_t *pools) {
  json_t *res = json_array();
  json_t *prefix = json_array();
  cartesianProduct(prefix, pools, 0, res);
  json_decref(prefix);
  return res;
}


json_t *objProduct(json_t * obj, json_t * parent) {
  json_t * keys = json_array();
  const char *key ; json_t * val;
  json_object_foreach(obj,key,val) json_array_append(keys,json_string(key));

  json_t *dict_values = json_array();
  const char *k ; json_t * v;
  json_object_foreach(obj,k,v) json_array_append(dict_values,v);
  json_t *prod = product(dict_values);
  json_t *out = json_array();

  size_t i; json_t *values;
  json_array_foreach(prod,i,values) {
    json_t *var = json_deep_copy(parent);
    size_t j; json_t *key_s;
    json_array_foreach(keys,j,key_s) {
      json_object_set(var, json_string_value(key_s), json_array_get(values,j));
    //   json_object_set_new(var, json_string_value(key_s), json_array_get(values,j));
    } 
    json_array_append(out,var);
    json_decref(var);
  }
  return out;
}

json_t *parseVariations(json_t *schema, json_t *parent, const char *parent_key){
  json_t *variations = json_pack("[]");;
  json_t *properties = json_object_get(schema, "properties");
  json_t *schema_variations = json_object_get(schema,"oneOf");

  json_t *schema_variation; int i;  //loop variables
  if (schema_variations)
    json_array_foreach(schema_variations,i,schema_variation) {
      json_t *variation_parent = json_deep_copy(parent);
      json_t *id_props = json_object_get(schema_variation,"properties");
      json_t *dp_props = json_object_get(schema_variation, "required");

      const char *prop_key; json_t *prop_val; 
      json_object_foreach(id_props, prop_key, prop_val){
        char key[strlen(parent_key) + strlen(prop_key) + 2];
        strcpy(key, parent_key);
        if (parent_key) strcat(key,".");
        strcat(key, prop_key);
        json_object_set(variation_parent, key, json_object_get(prop_val,"const"));
      }

      json_t *prop_schemas = json_pack("[]");
      json_t *enum_props = json_pack("{}");

      json_t *prop_name; size_t j;
      json_array_foreach(dp_props,j,prop_name) {
        const char *prop_key = json_string_value(prop_name);
        json_t *prop = json_object_get(properties, prop_key);

        json_t *STR_OBJECT = json_string("object");
        if (json_equal(json_object_get(prop,"type"),STR_OBJECT))
            json_array_append(prop_schemas, prop_name);
        
        else if (json_is_array(json_object_get(prop,"enum")))
            json_object_set(enum_props,prop_key,json_object_get(prop,"enum"));
        
        else if (json_object_get(prop,"default")) 
            json_object_set(variation_parent,prop_key,json_object_get(prop,"default"));
        else {
          printf("ERROR: Ambiguous Schema.");
          exit(-1);
        }
       // json_decref(prop); 
        json_decref(STR_OBJECT);
        // json_decref(prop_key)
      }
      int num_schemas = json_array_size(prop_schemas);
      if (num_schemas) {
        json_t *prop_name; int j;
        json_array_foreach(prop_schemas,j,prop_name) {
          const char *prop_key = json_string_value(prop_name);
          json_t *prop = json_object_get(properties, prop_key);

          json_t *prop_schema_variations = parseVariations(prop, variation_parent, prop_key);
          json_array_extend(variations, prop_schema_variations);
          json_decref(prop_schema_variations);
          // json_decref(prop);
        }
      }
      else if (json_object_size(enum_props)) {
        json_t *enum_variations = objProduct(enum_props,variation_parent);
        json_array_extend(variations,enum_variations);
      }
      else json_array_append(variations, variation_parent);

      json_decref(variation_parent);
      json_decref(prop_schemas);
      json_decref(enum_props);
    }
  else {
    json_t *variation_parent = json_deep_copy(parent);
    json_t *enum_props = json_pack("{}");

    const char *prop_key; json_t * prop;
    json_object_foreach(properties,prop_key,prop) {
      char key[strlen(parent_key) + strlen(prop_key) + 2];
      strcpy(key, parent_key);
      if (parent_key) strcat(key, ".");
      strcat(key, prop_key);

      json_t *STR_OBJECT = json_string("object");
      if (json_is_array(json_object_get(prop,"enum")))
          json_object_set(enum_props,key,json_object_get(prop,"enum"));
      
      else if (!json_equal(json_object_get(prop,"type"), STR_OBJECT) && json_object_get(prop,"default")) 
          json_object_set(variation_parent,key,json_object_get(prop,"default"));
      
      json_decref(STR_OBJECT);
    }
    json_t *enum_variations = objProduct(enum_props, variation_parent);
    json_array_extend(variations, enum_variations);
    // json_decref(enum_variations);
    // json_decref(enum_props);
    json_decref(variation_parent);
  }
  return variations;
}

