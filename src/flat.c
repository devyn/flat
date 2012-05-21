#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "flat.h"

void flat_stack_init (flat_stack_t *stack) {
	stack->size = 0;
	stack->next = NULL;
}

void flat_stack_destroy (flat_stack_t *stack) {
	flat_stack_t *stack_p;

	while (stack != NULL) {
		int i;
		for (i = 0; i < stack->size; i++) {
			flat_value_free_refs (&stack->contents[i]);
		}
		stack_p = stack->next;
		free (stack);
		stack = stack_p;
	}
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

int flat_stack_peek (flat_stack_t *stack, unsigned int index, flat_value_t *target) {
	while (stack != NULL) {
		if (index >= stack->size) {
			index -= stack->size;
			stack = stack->next;
		} else {
			memcpy (target, stack->contents + stack->size - index - 1, sizeof (flat_value_t));
			return 0;
		}
	}

	return 1;
}

int flat_stack_size (flat_stack_t *stack) {
	int size = 0;

	while (stack != NULL) {
		size += stack->size;
		stack = stack->next;
	}

	return size;
}

void flat_stack_print (flat_stack_t *stack) {
	int first;

	while (stack != NULL) {
		int n;

		for (n = stack->size - 1; n >= 0; n--) {
			if (first) first = !first; else printf (" ");
			flat_value_print (&stack->contents[n]);
		}

		stack = stack->next;
	}
	printf ("\n");
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

void flat_value_free_refs (flat_value_t *value) {
	switch (value->kind) {
		case FLAT_WORD:
			free (value->value.as_word);
			break;
		case FLAT_INT:
			break;
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

			vfprintf (stderr, "Error: Unknown word: `%s'.\n", vl);

			va_end (vl);
			break;
	}
}

int flat_interpret (flat_interpreter_t *interpreter, flat_value_t *instruction) {
	switch (instruction->kind) {
		case FLAT_WORD:
			if (strcmp ("drop", instruction->value.as_word) == 0) {
				if (flat_stack_size (interpreter->stack) < 1) {
					flat_interpreter_error (interpreter, FLAT_ERROR_NOT_ENOUGH_ARGUMENTS, "drop", 1);
					return 1;
				}

				flat_value_t discard;
				flat_stack_pop (&interpreter->stack, &discard);

				flat_value_free_refs (&discard);
			} else if (strcmp ("clear", instruction->value.as_word) == 0) {
				flat_stack_destroy (interpreter->stack);
				interpreter->stack = NULL;
			} else if (strcmp ("+", instruction->value.as_word) == 0) {
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
				flat_interpreter_error (interpreter, FLAT_ERROR_UNKNOWN_WORD, instruction->value.as_word);
			}
			flat_value_free_refs (instruction);
			break;
		default:
			flat_stack_push (&interpreter->stack, instruction);
	}
	return 0;
}

void flat_read_eval_print (flat_interpreter_t *interpreter) {
	char                 buffer[4096];
	flat_parser_state_t  state = FLAT_PARSER_STATE_ZERO;
	unsigned int         item_start;
	char                *item_partial = NULL;

	printf (">> ");

	while (!feof (stdin)) {
		if (fgets (buffer, 4096, stdin) == NULL) break;

		int  c;
		char ch;

		for (c = 0, ch = buffer[c]; c < 4096 && ch != '\0'; c++, ch = buffer[c]) {
			switch (state) {
				case FLAT_PARSER_STATE_ZERO:
					if (ch == ' ' || ch == '\t' || ch == '\n') {
					} else if (ch >= '0' && ch <= '9') {
						item_start = c;
						state = FLAT_PARSER_STATE_READ_INT;
					} else {
						item_start = c;
						state = FLAT_PARSER_STATE_READ_WORD;
					}
					break;
				case FLAT_PARSER_STATE_READ_WORD:
					if (ch == ' ' || ch == '\t' || ch == '\n') {
						flat_value_t word;
						char *str;
						
						if (item_partial == NULL) {
							str = malloc (c - item_start + 1);
							memcpy (str, buffer + item_start, c - item_start);
							str[c - item_start] = '\0';
						} else {
							unsigned int len = strlen (item_partial);

							str = malloc (c - item_start + len + 1);
							memcpy (str, item_partial, len);
							memcpy (str + len, buffer + item_start, c - item_start);
							str[c - item_start + len] = '\0';

							free (item_partial);
							item_partial = NULL;
						}

						flat_value_word (&word, str);

						item_start = 0;
						state = FLAT_PARSER_STATE_ZERO;

						flat_interpret (interpreter, &word);
					}
					break;
				case FLAT_PARSER_STATE_READ_INT:
					if (ch == ' ' || ch == '\t' || ch == '\n') {
						flat_value_t intv;
						char *str;
						
						if (item_partial == NULL) {
							str = malloc (c - item_start + 1);
							memcpy (str, buffer + item_start, c - item_start);
							str[c - item_start] = '\0';
						} else {
							unsigned int len = strlen (item_partial);

							str = malloc (c - item_start + len + 1);
							memcpy (str, item_partial, len);
							memcpy (str + len, buffer + item_start, c - item_start);
							str[c - item_start + len] = '\0';

							free (item_partial);
							item_partial = NULL;
						}

						flat_value_int (&intv, atoi (str));
						free (str);

						item_start = 0;
						state = FLAT_PARSER_STATE_ZERO;

						flat_interpret (interpreter, &intv);
					}
					break;
			}

			if (ch == '\n') goto got_newline;
		}

		if (state != FLAT_PARSER_STATE_ZERO) {
			item_partial = malloc (c - item_start);
			memcpy (item_partial, buffer + item_start, c - item_start - 1);
			item_partial[c - item_start - 1] = '\0';
		}
	}

got_newline:

	if (flat_stack_size (interpreter->stack) > 0) {
		printf ("=> ");
		flat_stack_print (interpreter->stack);
	}
}

int main (int argc, char **argv) {
	flat_interpreter_t interpreter;

	while (!feof (stdin)) {
		flat_read_eval_print (&interpreter);
	}

	return 0;
}
