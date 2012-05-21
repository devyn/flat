#ifndef FLAT_H
#define FLAT_H

typedef struct flat_program flat_program_t;

#include "flat/dictionary.h"

typedef struct flat_value {
	enum {
		FLAT_WORD,
		FLAT_INT
	} kind;
	union {
		char *as_word;
		int   as_int;
	} value;
} flat_value_t;

typedef struct flat_value_list {
	flat_value_t *item;
	struct flat_value_list *next;
} flat_value_list_t;

typedef struct flat_stack {
	unsigned short       size;
	flat_value_t         contents[32];
	struct flat_stack *next;
} flat_stack_t;

typedef enum {
	FLAT_OK,
	FLAT_ERROR_NOT_ENOUGH_ARGUMENTS,
	FLAT_ERROR_TYPE_MISMATCH,
	FLAT_ERROR_UNKNOWN_WORD
} flat_error_t;

typedef enum {
	FLAT_PARSER_STATE_ZERO,
	FLAT_PARSER_STATE_READ_WORD,
	FLAT_PARSER_STATE_READ_INT
} flat_parser_state_t;

typedef struct flat_interpreter {
	flat_stack_t *stack;
	flat_dictionary_t dictionary;
} flat_interpreter_t;

struct flat_program {
	enum {
		FLAT_PROGRAM_NATIVE,
		FLAT_PROGRAM_INTERPRETED
	} kind;
	union {
		int (*as_native) (flat_interpreter_t *);
		flat_value_list_t *as_interpreted;
	} value;
};

void flat_stack_init (flat_stack_t *stack);

void flat_stack_destroy (flat_stack_t *stack);

int flat_stack_push (flat_stack_t **stack, flat_value_t *value);

int flat_stack_pop (flat_stack_t **stack, flat_value_t *target);

int flat_stack_peek (flat_stack_t *stack, unsigned int index, flat_value_t *target);

int flat_stack_size (flat_stack_t *stack);

void flat_stack_print (flat_stack_t *stack);

void flat_value_word (flat_value_t *target, char *val);

void flat_value_int (flat_value_t *target, int val);

// Note: flat_value_serialize() allocates memory but does not manage it; you must free() the string manually.
int flat_value_serialize (char **ret, flat_value_t *value);

void flat_value_print (flat_value_t *value);

char *flat_value_type_name (flat_value_t *value);

void flat_value_free_refs (flat_value_t *value);

int flat_program_native (flat_program_t **program_ptr, int (*native) (flat_interpreter_t *));

int flat_program_interpreted (flat_program_t **program_ptr, flat_value_list_t *interpreted);

void flat_interpreter_error (flat_interpreter_t *interpreter, flat_error_t errno, ...);

int flat_interpret (flat_interpreter_t *interpreter, flat_value_t *instruction);

int flat_interpreter_register_native (flat_interpreter_t *interpreter, char *word, int (*native) (flat_interpreter_t *));

void flat_read_eval_print (flat_interpreter_t *interpreter);

#endif
