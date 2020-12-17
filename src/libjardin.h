#include "jansson.h"


void cartesianProduct(json_t *prefix, json_t *pools, size_t offset, json_t *res);

json_t *product(json_t *pools);

json_t *objProduct(json_t * obj, json_t * parent);

json_t *parseVariations(json_t *schema, json_t *parent, const char *parent_key);
