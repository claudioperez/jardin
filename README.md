---
title: jardin
description: Utility for taking the Cartesian product of a JSON schema file.
...

<h1><img src="main.svg" alt="" width=75></img>jardin</h1>

Utility for taking the Cartesian product of a JSON schema file.

    Usage: jardin [OPTION...] FILE
      or:  jardin [OPTION...] FILE1 FILE2
    Generate a JSON array of all possible objects that satisfy a given schema
    satisfying a subset of JSON schema draft 2019-09.

    Input formats:
      -a, --is-array             Parse input file as JSON array
      -j, --is-object            Parse input file as JSON object
      -s, --is-schema            Parse input file as JSON schema

    Operation:
      -?, --help                 Give this help list
      -d, --debug                Produce output for debugging
      -f, --flatJSON             Produce flat JSON output
      -o, --output=OUTFILE       Output to OUTFILE instead of to standard output
          --usage                Give a short usage message
      -v, --verbose              Produce verbose output
      -V, --version              Print program version

    Mandatory or optional arguments to long options are also mandatory or optional
    for any corresponding short options.

## Examples

### Arrays

    $ jardin -a array.json   #[["a", "b"], [1, 3], ["D"]]

    [
      ["a", 1, "D"], 
      ["a", 3, "D"], 
      ["b", 1, "D"], 
      ["b", 3, "D"]
    ]

### Objects

    $ jardin -j object.json  #{"seed": [1,2,3], "method": ["lhs","monte carlo"]}

    [
      {"seed": 1, "method": "lhs"},
      {"seed": 1, "method": "monte carlo"},
      {"seed": 2, "method": "lhs"},
      {"seed": 2, "method": "monte carlo"},
      {"seed": 3, "method": "lhs"},
      {"seed": 3, "method": "monte carlo"}
    ]

