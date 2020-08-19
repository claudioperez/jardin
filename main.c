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
#include <argp.h>
#include <argz.h>
#include <stdlib.h>

#include "libjardin.h"

const char *argp_program_version = "jardin-v0.0.1";
const char *argp_program_bug_address = "<claudioperezii@outlook.com>";
static char doc[] = "Generate a JSON array of all possible objects that "
                    "satisfy a given schema satisfying a subset of JSON "
                    "schema draft 2019-09.";

static char args_doc[] = "FILE\nFILE1 FILE2";
static struct argp_option options[] = {
    {0, 0, 0, 0, "Input formats:", 7},
    {"is-schema", 's', 0, 0, "Parse input file as JSON schema"},
    {"is-object", 'j', 0, 0, "Parse input file as JSON object"},
    {"is-array",  'a', 0, 0, "Parse input file as JSON array"},
    {0, 0, 0, 0, "Operation:", -1},
    {"verbose", 'v', 0, 0, "Produce verbose output"},
    {"debug", 'd', 0, 0, "Produce output for debugging"},
    {"flatJSON", 'f', 0, 0, "Produce flat JSON output"},
    {"output", 'o', "OUTFILE", 0,
     "Output to OUTFILE instead of to standard output"},
    {0}};

struct arguments {
    char *argz;
    size_t argz_len;
    int verbose, flatout, debug, is_schema, is_object, is_array;
    char *outfile;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
  struct arguments *arguments = state->input;
  switch (key) {
  case 's': arguments->is_schema = 1; break;
  case 'j': arguments->is_object = 1; break;
  case 'a': arguments->is_array = 1; break;

  case 'v': arguments->verbose = 1; break;
  case 'd': arguments->debug = 1; break;
  case 'f': arguments->flatout = 1; break;
  case 'o': arguments->outfile = arg; break;
  case ARGP_KEY_ARG:
    argz_add(&arguments->argz, &arguments->argz_len, arg);
    break;
  case ARGP_KEY_INIT:
    arguments->argz = 0;
    arguments->argz_len = 0;
    break;
  case ARGP_KEY_END: {
    size_t count = argz_count(arguments->argz, arguments->argz_len);
    if (count > 2)
      argp_failure(state, 1, 0, "too many arguments");
    else if (count < 1)
      argp_failure(state, 1, 0, "too few arguments");
  }
  break;
  }
  return 0;
}

static struct argp argp = {options, parse_opt, args_doc, doc, 0, 0, 0};

//#####################################################################################

int main(int argc, char **argv) 
{
    struct arguments arguments;
    FILE *outstream;
    arguments.verbose = 0;
    arguments.outfile = NULL;
    arguments.debug = 0;

    arguments.is_schema = 1;
    arguments.is_object = 0;
    arguments.is_array = 0;

    json_t *variations;

    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    const char *prev = NULL;
    char *filename;
    while ((filename = argz_next(arguments.argz, arguments.argz_len, prev))) {
        const char *inputFile = filename;
        json_t *parent = json_pack("{}");

        json_error_t error;
        json_t *rootSchema = json_load_file(inputFile, 0, &error);
        if (!rootSchema) {
            printf("Error: JSON loader failed to open file.\n");
            exit(-1);
        }
        if (arguments.is_object) {
            if (arguments.verbose) printf("OBJECT\n");
            variations = objProduct(rootSchema, parent);
        } else if (arguments.is_array) {
            if (arguments.verbose) printf("ARRAY\n");
            variations = product(rootSchema);
        } else if (arguments.is_schema) {
            if (arguments.verbose) printf("SCHEMA\n");
            const char *parent_key = "\0";
            variations = parseVariations(rootSchema, parent, parent_key);
        }

        if (arguments.outfile) {
            outstream = fopen(arguments.outfile, "w");
            if (arguments.debug)
                printf("DEBUG: Output array size: %ld \n", json_array_size(variations));
            else
                json_dumpf(variations, outstream, JSON_INDENT(4));
            fclose(outstream);
        } else {
            outstream = stdout;
            if (arguments.debug)
                printf("DEBUG: Output array size: %ld \n", json_array_size(variations));
            else
                json_dumpf(variations, outstream, JSON_INDENT(4));
            printf("\n");
        }
        json_decref(variations);
        json_decref(parent);
        json_decref(rootSchema);
        prev = filename;
    }
    free(arguments.argz);
    return 0;
}

