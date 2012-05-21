#include "flat.h"
#include "flat/dictionary.h"

struct flat_dictionary_node {
	struct flat_dictionary_node *parent;
	struct flat_dictionary_node *left;
	struct flat_dictionary_node *right;

	char *key;
	flat_program_t *value;

	unsigned int height;
};

int flat_dictionary_lookup (flat_dictionary_t *dictionary, char *key, flat_program_t *target) {
	return -1;
}

int flat_dictionary_insert (flat_dictionary_t *dictionary, char *key, flat_program_t *value) {
	return -1;
}

int flat_dictionary_delete (flat_dictionary_t *dictionary, char *key) {
	return -1;
}

void flat_dictionary_node_rotate_left (flat_dictionary_node_t **node_ptr) {
}

void flat_dictionary_node_rotate_right (flat_dictionary_node_t **node_ptr) {
}
