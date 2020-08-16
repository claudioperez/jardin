/* 
BSD 3-Clause License

Copyright (c) 2020, Claudio Perez. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdio.h>
#include <string.h>
#include <argp.h>
#include <argz.h>
#include <stdlib.h>

#include "jansson.h"


const char *argp_program_version = "jardin-v0.0.0";
const char *argp_program_bug_address = "<claudioperezii@outlook.com>";
static char doc[] = "Generate a JSON array of all possible objects that "
                    "satisfy a given schema satisfying a subset of JSON "
                    "schema draft 2019-09.";


static char args_doc[] = "FILE\nFILE1 FILE2";
static struct argp_option options[] = { 
    {"verbose",'v', 0, 0, "Produce verbose output"},
    {"verbose",'f', 0, 0, "Produce flat JSON output"},
    {"output", 'o', "OUTFILE", 0,
    "Output to OUTFILE instead of to standard output"},
    { 0 } 
};

struct arguments {
    char *argz;
    size_t argz_len;
    int verbose, flatout;
    char *outfile;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    struct arguments *arguments = state->input;
    switch (key) {
      case 'v': arguments->verbose = 1; break;
      case 'f': arguments->flatout = 1; break;
      case 'o': arguments->outfile = arg; break;
      case ARGP_KEY_ARG: 
        argz_add(&arguments->argz,&arguments->argz_len,arg);
      break;
      case ARGP_KEY_INIT:
        arguments -> argz=0;
        arguments -> argz_len=0;
      break;
      case ARGP_KEY_END: {
          size_t count = argz_count (arguments->argz,arguments->argz_len);
          if (count>2)
            argp_failure(state,1,0,"too many arguments");
          else if (count<1)
            argp_failure(state,1,0,"too few arguments");
      }
      break;
    }   
    return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc, 0, 0, 0 };

//#####################################################################################

void cartesianProduct(json_t *prefix, json_t *pools, size_t offset, json_t *res) {
  if (offset >= json_array_size(pools)) {
    json_array_append(res ,prefix);
    return;
  }
  json_t *pool = json_array_get(pools, offset);
  int i; json_t *item;
  json_array_foreach(pool,i,item) {
    json_t *array = json_deep_copy(prefix);
    json_array_extend(array, json_pack("[o]", item));
    cartesianProduct(array, pools, offset + 1, res);
  }
}

json_t *product(json_t *pools) {
  json_t *res = json_array();
  json_t *prefix = json_array();
  cartesianProduct(prefix, pools, 0, res);
  return res;
}

json_t *objProduct(json_t * obj, json_t * parent) {
  json_t * keys = json_array();
  const char *key ; json_t * val;
  json_object_foreach(obj,key,val){json_array_append(keys,json_string(key));}

  json_t *dict_values = json_array();
  const char *k ; json_t * v;
  json_object_foreach(obj,k,v){json_array_append(dict_values,v);}
  json_t * prod = product(dict_values);
  json_t *out = json_array();

  size_t i; json_t *values;
  json_array_foreach(prod,i,values) {
    json_t *var = json_deep_copy(parent);
    size_t j; json_t *key_s;
    json_array_foreach(keys,j,key_s)
      json_object_set(var, json_string_value(key_s), json_array_get(values,j));
  
    json_array_append(out,var);
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
        if (parent_key!="\0") strcat(key,".");
        strcat(key, prop_key);
        json_object_set(variation_parent, key, json_object_get(prop_val,"const"));
      }

      json_t *prop_schemas = json_pack("[]");
      json_t *enum_props = json_pack("{}");

      json_t *prop_name; size_t j;
      json_array_foreach(dp_props,j,prop_name){
        
        const char *prop_key = json_string_value(prop_name);
        json_t *prop = json_object_get(properties, prop_key);

        if (json_equal(json_object_get(prop,"type"),json_string("object"))) 
            json_array_append(prop_schemas, json_string(prop_key));
        
        else if (json_object_get(prop,"enum"))
            json_object_set(enum_props,prop_key,json_object_get(prop,"enum"));
        
        else if (json_object_get(prop,"default")) 
            json_object_set(variation_parent,prop_key,json_object_get(prop,"default"));
      
      }
      int num_schemas = json_array_size(prop_schemas);
      if (num_schemas) {
        json_t *prop_name; int j;
        json_array_foreach(prop_schemas,j,prop_name) {
          const char *prop_key = json_string_value(prop_name);
          json_t *prop = json_object_get(properties, prop_key);

          json_t *prop_schema_variations = parseVariations(prop, variation_parent, prop_key);
          json_array_extend(variations, prop_schema_variations);
        }
      }
      else if (json_object_size(enum_props)) {
        json_t *enum_variations = objProduct(enum_props,variation_parent);
        json_array_extend(variations,enum_variations);
      }
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

      if (json_object_get(prop,"enum"))
          json_object_set(enum_props,key,json_object_get(prop,"enum"));
      
      else if (!json_equal(json_object_get(prop,"type"), json_string("object")) && json_object_get(prop,"default")) 
          json_object_set(variation_parent,key,json_object_get(prop,"default"));
      
      json_t *enum_variations = objProduct(enum_props, variation_parent);
      json_array_extend(variations, enum_variations);
    }
  }
  return variations;
}

int main(int argc, char **argv) {
  struct arguments arguments;
  FILE *outstream;
  /* default options */
  arguments.verbose = 0;
  arguments.outfile = NULL;

  /* parse arguments */
  json_t *variations;
  if (argp_parse(&argp,argc,argv,0,0,&arguments)==0 ) {
    const char *prev = NULL;
    char *filename;
    json_t *list_variations = json_pack("[{}]");
    while ((filename=argz_next(arguments.argz,arguments.argz_len,prev))) {
      const char *inputFile = filename;

      json_error_t error;
      json_t *rootSchema = json_load_file(inputFile, 0, &error);
      if (rootSchema == NULL) {
          printf("Error: Failed to open file.\n");
          exit (-1);
      }
      json_t *parent = json_pack("{}");
      const char *parent_key = "\0";
      variations = parseVariations(rootSchema, parent, parent_key);

      json_array_extend(list_variations,variations);
      if (prev){
        variations = product(list_variations);
      }
      prev = filename;
    }
    if (arguments.outfile)
      outstream = fopen (arguments.outfile, "w");
    else
      outstream = stdout;
    json_dumpf(variations, outstream, JSON_INDENT(4));
    printf("\n");
  }
  return 0;
}
