/*******************************************************************************
Modified BSD License

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

4. Trademarks. This License does not grant permission to use the trade names, 
   trademarks, service marks, or product names of the Licensor, except as 
   required for reasonable and customary use in describing the origin of the 
   Work and reproducing the content of the NOTICE file.

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
*******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <argp.h>
#include <argz.h>
#include <stdlib.h>

#include "libjardin.h"



const char *argp_program_version = "jardin-v0.0.0";
const char *argp_program_bug_address = "<claudioperezii@outlook.com>";
static char doc[] = "Generate a JSON array of all possible objects that "
                    "satisfy a given schema satisfying a subset of JSON "
                    "schema draft 2019-09.";


static char args_doc[] = "FILE\nFILE1 FILE2";
static struct argp_option options[] = { 
    {"verbose",   'v', 0, 0, "Produce verbose output"},
    {"debug",     'd', 0, 0, "Produce output for debugging"},
    {"is-schema", 's', 0, 0, "Parse input file as JSON schema"},
    {"is-object", 'j', 0, 0, "Parse input file as JSON object"},
    {"is-array",  'a', 0, 0, "Parse input file as JSON array"},
    {"flatJSON",  'f', 0, 0, "Produce flat JSON output"},
    {"output",    'o', "OUTFILE", 0,
    "Output to OUTFILE instead of to standard output"},
    { 0 } 
};

struct arguments {
    char *argz;
    size_t argz_len;
    int verbose, flatout, debug, is_schema, is_object, is_array;
    char *outfile;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    struct arguments *arguments = state->input;
    switch (key) {
      case 'v': arguments->verbose = 1; break;
      case 'd': arguments->debug = 1; break;
      case 's': arguments->is_schema = 1; break;
      case 'j': arguments->is_object = 1; break;
      case 'a': arguments->is_array = 1; break;
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



int main(int argc, char **argv) {
  struct arguments arguments;
  FILE *outstream;
  /* default options */
  arguments.verbose = 0;
  arguments.outfile = NULL;
  arguments.debug = 0;
  arguments.is_schema = 1;
  arguments.is_object = 0;
  arguments.is_array = 0;

  /* parse arguments */
  json_t *variations;
  argp_parse(&argp,argc,argv,0,0,&arguments);

  const char *prev = NULL;
  char *filename;
  // json_t *list_variations = json_pack("[{}]");
  while ((filename=argz_next(arguments.argz,arguments.argz_len,prev))) {
    const char *inputFile = filename;
    json_t *parent = json_pack("{}");

    json_error_t error;
    json_t *rootSchema = json_load_file(inputFile, 0, &error);
    if (!rootSchema) {
        printf("Error: JSON loader failed to open file.\n");
        exit(-1);
    }
    if (arguments.is_object) {
      printf("OBJECT\n");
      // json_dumpf(rootSchema, stdout, JSON_INDENT(4));
      variations = objProduct(rootSchema, parent);
    }
    else if (arguments.is_array) {
      printf("ARRAY\n");
      variations = product(rootSchema);
    }
    else if (arguments.is_schema) {
      printf("SCHEMA\n");
      const char *parent_key = "\0";
      variations = parseVariations(rootSchema, parent, parent_key);
      // json_decref(parent);
    }

    if (arguments.outfile) {
      /* output to file provided */
      outstream = fopen (arguments.outfile, "w");
      if (arguments.debug) 
        printf("DEBUG: Output array size: %ld", json_array_size(variations));
      else
        json_dumpf(variations, outstream, JSON_INDENT(4));
      fclose(outstream);
    }
    else {
      /* output to stdout stream */
      outstream = stdout;
      if (arguments.debug) 
        printf("DEBUG: Output array size: %ld", json_array_size(variations));
      else
        json_dumpf(variations, outstream, JSON_INDENT(4));
      printf("\n");
    }
    json_decref(variations);
    json_decref(parent);
    json_decref(rootSchema);
    prev = filename;
  }
  free (arguments.argz);
    // json_decref(variations);
  // json_decref(list_variations);


  return 0;
}

