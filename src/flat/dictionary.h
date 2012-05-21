#ifndef FLAT_DICTIONARY_H
#define FLAT_DICTIONARY_H

#include "flat.h"

typedef struct flat_dictionary_node flat_dictionary_node_t;

typedef flat_dictionary_node_t *flat_dictionary_t;

int flat_dictionary_lookup (flat_dictionary_t *dictionary, char *key, flat_program_t *target);

int flat_dictionary_insert (flat_dictionary_t *dictionary, char *key, flat_program_t *value);

int flat_dictionary_delete (flat_dictionary_t *dictionary, char *key);

#endif
