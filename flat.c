#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

typedef struct flat_value_t {
	enum {
		FLAT_WORD,
		FLAT_INT
	} kind;
	union {
		char *as_word;
		int   as_int;
	} value;
} flat_value_t;

typedef struct flat_stack_t {
	unsigned short       size;
	flat_value_t         contents[32];
	struct flat_stack_t *next;
} flat_stack_t;

typedef enum {
	FLAT_OK,
	FLAT_ERROR_NOT_ENOUGH_ARGUMENTS,
	FLAT_ERROR_TYPE_MISMATCH,
	FLAT_ERROR_UNKNOWN_WORD
} flat_error_t;

typedef struct flat_interpreter_t {
	flat_stack_t *stack;
} flat_interpreter_t;

void flat_stack_init (flat_stack_t *stack) {
	stack->size = 0;
	stack->next = NULL;
}

int flat_stack_push (flat_stack_t **stack, flat_value_t *value) {
	if (*stack == NULL) {
		flat_stack_t *stack_new = malloc (sizeof (flat_stack_t));

		if (stack_new == NULL) return 8;
		flat_stack_init (stack_new);

		*stack = stack_new;
	}

	if ((*stack)->size == 32) {
		flat_stack_t *stack_new = malloc (sizeof (flat_stack_t));

		if (stack_new == NULL) return 8;
		flat_stack_init (stack_new);

		stack_new->next = *stack;
		*stack = stack_new;
	}

	memcpy ((*stack)->contents + ((*stack)->size++), value, sizeof (flat_value_t));
	return 0;
}

int flat_stack_pop (flat_stack_t **stack, flat_value_t *target) {
	if (*stack == NULL) return 1;

	if ((*stack)->size == 0) {
		if ((*stack)->next == NULL) {
			return 1;
		} else {
			flat_stack_t *next = (*stack)->next;
			free (*stack);
			*stack = next;
		}
	}

	memcpy (target, (*stack)->contents + (--(*stack)->size), sizeof (flat_value_t));
	return 0;
}

int flat_stack_size (flat_stack_t *stack) {
	int size = 0;

	while (stack != NULL) {
		size += stack->size;
		stack = stack->next;
	}

	return size;
}

void flat_value_word (flat_value_t *target, char *val) {
	target->kind = FLAT_WORD;
	target->value.as_word = val;
}

void flat_value_int (flat_value_t *target, int val) {
	target->kind = FLAT_INT;
	target->value.as_int = val;
}

// Note: flat_value_serialize() allocates memory but does not manage it; you must free() the string manually.
int flat_value_serialize (char **ret, flat_value_t *value) {
	switch (value->kind) {
		case FLAT_WORD:
			return asprintf (ret, "%s", value->value.as_word);
		case FLAT_INT:
			return asprintf (ret, "%i", value->value.as_int);
		default:
			*ret = "<unknown>";
			return 0;
	}
}

void flat_value_print (flat_value_t *value) {
	char *ret;
	flat_value_serialize (&ret, value);
	fputs (ret, stdout);
	free (ret);
}

char *flat_value_type_name (flat_value_t *value) {
	switch (value->kind) {
		case FLAT_WORD:
			return "word";
		case FLAT_INT:
			return "int";
		default:
			return "unknown";
	}
}

void flat_interpreter_error (flat_interpreter_t *interpreter, flat_error_t errno, ...) {
	va_list vl;

	switch (errno) {
		case FLAT_OK:
			break;
		case FLAT_ERROR_NOT_ENOUGH_ARGUMENTS:
			va_start (vl, errno);

			fprintf (stderr, "Error: Not enough arguments to `%s'. Minimum stack size: %i; actual stack size: %i.\n",
					va_arg (vl, char *), va_arg (vl, int), flat_stack_size (interpreter->stack));

			va_end (vl);
			break;
		case FLAT_ERROR_TYPE_MISMATCH:
			va_start (vl, errno);

			int   amount   = va_arg (vl, int);
			char *expected = va_arg (vl, char *);

			fprintf (stderr, "Error: Type mismatch. In stack order, expected argument types were: %s, but got: ", expected);

			int i;

			for (i = 0; i < amount; i++) {
				if (i == amount - 1) {
					fprintf (stderr, "%s.\n", flat_value_type_name (va_arg (vl, flat_value_t *)));
				} else {
					fprintf (stderr, "%s, ", flat_value_type_name (va_arg (vl, flat_value_t *)));
				}
			}

			va_end (vl);
			break;
		case FLAT_ERROR_UNKNOWN_WORD:
			va_start (vl, errno);

			vfprintf (stderr, "Error: Unknown word: `%s'.", vl);

			va_end (vl);
			break;
	}
}

int flat_interpret (flat_interpreter_t *interpreter, flat_value_t *instruction) {
	switch (instruction->kind) {
		case FLAT_WORD:
			if (strcmp ("+", instruction->value.as_word) == 0) {
				if (flat_stack_size (interpreter->stack) < 2) {
					flat_interpreter_error (interpreter, FLAT_ERROR_NOT_ENOUGH_ARGUMENTS, "+", 2);
					return 1;
				}

				flat_value_t value1;
				flat_value_t value2;

				flat_stack_pop (&interpreter->stack, &value1);
				flat_stack_pop (&interpreter->stack, &value2);

				if (value1.kind != FLAT_INT || value2.kind != FLAT_INT) {
					flat_interpreter_error (interpreter, FLAT_ERROR_TYPE_MISMATCH, 2, "int, int", &value2, &value1);
					flat_stack_push (&interpreter->stack, &value2);
					flat_stack_push (&interpreter->stack, &value1);
					return 1;
				}

				flat_value_t value3;

				flat_value_int (&value3, value2.value.as_int + value1.value.as_int);

				flat_stack_push (&interpreter->stack, &value3);
			} else {
				fprintf (stderr, "Error: Unknown word: `%s'.", instruction->value.as_word);
			}
			break;
		default:
			flat_stack_push (&interpreter->stack, instruction);
	}
	return 0;
}

int main (int argc, char **argv) {
	flat_interpreter_t interpreter;
	flat_value_t       value;

	flat_value_int (&value, 64);

	flat_interpret (&interpreter, &value);
	flat_interpret (&interpreter, &value);

	flat_value_word (&value, "+");

	flat_interpret (&interpreter, &value);

	while (flat_stack_pop (&interpreter.stack, &value) == 0) {
		flat_value_print (&value);
		printf (" ");
	}

	printf ("\n");

	return 0;
}
