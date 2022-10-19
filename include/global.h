/*
 * DO NOT MODIFY THE CONTENTS OF THIS FILE.
 * IT WILL BE REPLACED DURING GRADING
 */
#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdio.h>

/*
 * Options info, set by validargs.
 *   If -h is specified, then the HELP_OPTION bit is set.
 *   If -v is specified, then the VALIDATE_OPTION bit is set.
 *   If -c is specified, then the CANONICALIZE_OPTION bit is set.
 *   If -p is specified, then the PRETTY_PRINT_OPTION bit is set.
 *   If PRETTY_PRINT_OPTION is set, then CANONICALIZE_OPTION must also be set.
 *   The least-significant byte contains the number of additional spaces
 *   to add at the beginning of each output line, for each increase
 *   in the indentation level of the value being output.
 */
int global_options;

#define HELP_OPTION (0x80000000)
#define VALIDATE_OPTION (0x40000000)
#define CANONICALIZE_OPTION (0x20000000)
#define PRETTY_PRINT_OPTION (0x10000000)

/*
 * Variables that keep track of the current amount of input data that has been
 * read.  Variable "argo_lines_read" starts at zero and is incremented each time
 * a newline character is read by function argo_read_char().  Variable
 * "argo_chars_read" starts at zero, is reset to zero at the beginning of each
 * line, and is incremented each time a character is read by function argo_read_char().
 * These variables are intended to be used for error messages and debugging.
 * They can be assigned to if it is necessary to reset their values for some reason,
 * such as if reading from multiple sources is done.
 */
int argo_lines_read;
int argo_chars_read;

/*
 * Variable that keeps track of the current indent level while pretty printing.
 * See the assignment handout for a description of how the indent level is to
 * be maintained and used.
 */
int indent_level;

/*
 * The following array contains statically allocated space for Argo values.
 * You *must* use the elements of this array to store your values.
 * The "argo_next_value" variable contains the index of the next unused slot
 * in the array.  As you use the elements of the array, you should increment
 * this value.  Pay attention to when all elements are used, so that you don't
 * use a "value" beyond the end of the array and corrupt something else in memory!
 */
#define NUM_ARGO_VALUES 100000
ARGO_VALUE argo_value_storage[NUM_ARGO_VALUES];
int argo_next_value;

/*
 * The following array contains storage to hold digits of an integer during
 * output conversion (the digits are naturally generated in the reverse order
 * from which they will be output).  You *must* use this array to hold the
 * digits; you may not declare your own arrays!
 */

#define ARGO_MAX_DIGITS 10
ARGO_CHAR argo_digits[ARGO_MAX_DIGITS];

/*
 * Prototypes for functions that you must implement.
 * For detailed specifications of these functions, refer to the
 * stubs in the file "argo.c", as well as to the assignment handout.
 * Note that you will almost certainly want to implement additional
 * functions besides those specified here.
 */
ARGO_VALUE *argo_read_value(FILE *);
int argo_read_string(ARGO_STRING *s, FILE *);
int argo_read_number(ARGO_NUMBER *n, FILE *);

int argo_write_value(ARGO_VALUE *, FILE *);
int argo_write_string(ARGO_STRING *, FILE *);
int argo_write_number(ARGO_NUMBER *, FILE *);

int validargs(int argc, char **argv);

#endif
