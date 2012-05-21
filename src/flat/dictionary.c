#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "flat.h"
#include "flat/dictionary.h"

struct flat_dictionary_node {
	struct flat_dictionary_node *parent;
	struct flat_dictionary_node *left;
	struct flat_dictionary_node *right;

	char *key;
	flat_program_t *value;

	int height;
};

int flat_dictionary_node_create (flat_dictionary_node_t **node_ptr, char *key, flat_program_t *value, flat_dictionary_node_t *parent) {
	*node_ptr = malloc (sizeof (flat_dictionary_node_t));

	if (*node_ptr == NULL) return 8;

	(*node_ptr)->parent = parent;
	(*node_ptr)->left   = NULL;
	(*node_ptr)->right  = NULL;

	(*node_ptr)->key    = key;
	(*node_ptr)->value  = value;

	(*node_ptr)->height = 1;

	return 0;
}

void flat_dictionary_node_rotate_left (flat_dictionary_node_t **node_ptr) {
	flat_dictionary_node_t *node = *node_ptr;

	*node_ptr = node->right;
	node->right->parent = node->parent;
	node->parent = node->right;
	node->right = node->right->left;
	(*node_ptr)->left = node;
}

void flat_dictionary_node_rotate_right (flat_dictionary_node_t **node_ptr) {
	flat_dictionary_node_t *node = *node_ptr;

	*node_ptr = node->left;
	node->left->parent = node->parent;
	node->parent = node->left;
	node->left = node->left->right;
	(*node_ptr)->right = node;
}

int flat_dictionary_node_balance_factor (flat_dictionary_node_t *node) {
	int bf = 0;

	if (node->left  != NULL) bf += node->left->height;
	if (node->right != NULL) bf -= node->right->height;

	return bf;
}

int flat_dictionary_node_balance (flat_dictionary_node_t *node, flat_dictionary_t *dictionary) {
	while (node != NULL) {
		int balance_factor =     (node->left == NULL ? 0 : node->left->height) - (node->right == NULL ? 0 : node->right->height);
		node->height       = 1 + (node->left == NULL ? 0 : node->left->height) + (node->right == NULL ? 0 : node->right->height);

		if (balance_factor < -1) {
			int right_balance_factor = flat_dictionary_node_balance_factor (node->right);

			if (right_balance_factor > 0) {
				flat_dictionary_node_rotate_right (&node->right);
			}

			if (node->parent == NULL) {
				flat_dictionary_node_rotate_left (dictionary);
				return 0;
			} else if (node->parent->left == node) {
				flat_dictionary_node_rotate_left (&node->parent->left);
			} else if (node->parent->right == node) {
				flat_dictionary_node_rotate_left (&node->parent->right);
			}
		} else if (balance_factor > 1) {
			int left_balance_factor = flat_dictionary_node_balance_factor (node->left);

			if (left_balance_factor < 0) {
				flat_dictionary_node_rotate_left (&node->left);
			}

			if (node->parent == NULL) {
				flat_dictionary_node_rotate_right (dictionary);
				return 0;
			} else if (node->parent->left == node) {
				flat_dictionary_node_rotate_right (&node->parent->left);
			} else if (node->parent->right == node) {
				flat_dictionary_node_rotate_right (&node->parent->right);
			}
		}

		node->height = 1 + (node->left == NULL ? 0 : node->left->height) + (node->right == NULL ? 0 : node->right->height);

		node = node->parent;
	}

	return 0;
}

int flat_dictionary_lookup (flat_dictionary_t *dictionary, char *key, flat_program_t **target) {
	flat_dictionary_node_t *node = *dictionary;

	while (node != NULL) {
		int cmp = strcmp (key, node->key);

		if (cmp == 0) {
			*target = node->value;
			return 1;
		} else if (cmp < 0) {
			node = node->left;
		} else {
			node = node->right;
		}
	}

	return 0; // false
}

int flat_dictionary_insert (flat_dictionary_t *dictionary, char *key, flat_program_t *value) {
	flat_dictionary_node_t *node = *dictionary;

	if (node == NULL) {
		return flat_dictionary_node_create (dictionary, key, value, NULL);
	}

	while (1) {
		int cmp = strcmp (key, node->key);
		
		if (cmp == 0) {
			node->value = value;
			return 0;
		} else if (cmp < 0) {
			if (node->left == NULL) {
				flat_dictionary_node_t *node2;
				flat_dictionary_node_create (&node2, key, value, node);
				node->left = node2;
				flat_dictionary_node_balance (node, dictionary);
				return 0;
			} else {
				node = node->left;
			}
		} else {
			if (node->right == NULL) {
				flat_dictionary_node_t *node2;
				flat_dictionary_node_create (&node2, key, value, node);
				node->right = node2;
				flat_dictionary_node_balance (node, dictionary);
				return 0;
			} else {
				node = node->right;
			}
		}
	}
}

int flat_dictionary_delete (flat_dictionary_t *dictionary, char *key) {
	return -1; // not yet implemented.
}
