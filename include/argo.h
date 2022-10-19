/*
 * DO NOT MODIFY THE CONTENTS OF THIS FILE.
 * IT WILL BE REPLACED DURING GRADING
 */
#ifndef ARGO_H
#define ARGO_H

/*
 * Definitions for "Argo" (aka JSON).
 */

/*
 * USAGE macro to be called from main() to print a help message and exit
 * with a specified exit status.
 */
#define USAGE(program_name, retcode) do { \
fprintf(stderr, "USAGE: %s %s\n", program_name, \
"[-h] [-c|-v] [-p INDENT]\n" \
"   -h       Help: displays this help menu.\n" \
"   -v       Validate: the program reads from standard input and checks whether\n" \
"            it is syntactically correct JSON.  If there is any error, then a message\n" \
"            describing the error is printed to standard error before termination.\n" \
"            No other output is produced.\n" \
"   -c       Canonicalize: once the input has been read and validated, it is\n" \
"            re-emitted to standard output in 'canonical form'.  Unless -p has been\n" \
"            specified, the canonicalized output contains no whitespace (except within\n" \
"            strings that contain whitespace characters).\n" \
"   -p       Pretty-print:  This option is only permissible if -c has also been specified.\n" \
"            In that case, newlines and spaces are used to format the canonical output\n" \
"            in a more human-friendly way.  For the precise requirements on where this\n" \
"            whitespace must appear, see the assignment handout.\n" \
"            The INDENT is an optional nonnegative integer argument that specifies the\n" \
"            number of additional spaces to be output at the beginning of a line for each\n" \
"            for each increase in indentation level.  If no value is specified, then a\n" \
"            default value of 4 is used.\n" \
); \
exit(retcode); \
} while(0)

/*
 * Type used to represent an input character.  It is intended to
 * represent a Unicode code point (4 bytes max), so the C type
 * "char" is not used.  It is signed, so that we can represent
 * the out-of-band value EOF (-1) as a value of this type.
 */
typedef int ARGO_CHAR;

/*
 * Type codes for Argo values.
 */
typedef enum {
    ARGO_NO_TYPE = 0,
    ARGO_BASIC_TYPE = 1,
    ARGO_NUMBER_TYPE = 2,
    ARGO_STRING_TYPE = 3,
    ARGO_OBJECT_TYPE = 4,
    ARGO_ARRAY_TYPE = 5
} ARGO_VALUE_TYPE;

/*
 * Basic Argo values, represented by the (unquoted) tokens
 * "true", "false", or "null" in Argo code.
 */
typedef enum {
    ARGO_NULL, ARGO_TRUE, ARGO_FALSE
} ARGO_BASIC;

/*
 * Structure used to hold a string value.
 * The content field is maintained as an array of char, which is not null-terminated
 * and which might contain '\0' characters.  This data is interpreted as Unicode text,
 * represented as an array of ARGO_CHAR values, each of which represents a single
 * Unicode code point.  The length field gives the length in bytes of the data.
 * The capacity field records the actual size of the data area.  This is included so
 * that the size can be dynamically increased while the string is being read.
 */
typedef struct argo_string {
    size_t capacity;                  // Current total size of space in the content.
    size_t length;                    // Current length of the content.
    ARGO_CHAR *content;              // Unicode code points (not null terminated).
} ARGO_STRING;

/*
 * Structure used to hold a number.
 * The "text_value" field holds a printable/parseable representation of the number
 * as Unicode text, conforming to the Argo standard.
 * The "int_value" field holds the value of the number in integer format, if the
 * number can be exactly represented as such.
 * The "float_value" field holds the value of the number in floating-point format.
 * The "valid_text" field is nonzero if the "text_valid" field contains a valid
 * representation of the value.
 * The "valid_int" field is nonzero if the "int_value" field contains a valid
 * representation of the value.
 * The "valid_float" field is nonzero if the "float_value" field contains a valid
 * representation of the value.
 * 
 * If multiple representations of the value of the number are present, they should
 * agree with each other.
 * It is up to an application to determine which representation is the appropriate
 * one to use, based on the semantics of the data being represented.
 */
typedef struct argo_number {
    struct argo_string string_value;   // Value represented in textual format.
    long int_value;                    // Value represented in integer format.
    double float_value;                // Value represented in floating-point format.
    char valid_string;		       // Nonzero if string representation is valid.
    char valid_int;		       // Nonzero if integer representation is valid.
    char valid_float;		       // Nonzero if floating point representation is valid.
} ARGO_NUMBER;

/*
 * An "object" has a list of members, each of which has a name and a value.
 * To store the members, we use a circular, doubly linked list, with the next and
 * previous pointers stored in the "next" and "prev" fields of the ARGO_VALUE structure
 * and the member name stored in the "name" field of the ARGO_VALUE structure.
 * The "member_list" field of the ARGO_OBJECT structure serves as the sentinel at
 * the head of the list.  This element does not represent one of the members;
 * rather, its "next" field points to the first member and its "prev" field points
 * to the last member.  An empty list of members is represented by the situation in
 * which both the "next" and "prev" fields point back to the sentinel object itself.
 *
 * Note that the collection of members of an object is supposed to be regarded as unordered,
 * which would permit it to be represented using a hash map or similar data structure,
 * which we are not doing here.
 */
typedef struct argo_object {
    struct argo_value *member_list;
} ARGO_OBJECT;

/*
 * An "array" has an ordered sequence of elements, each of which is just a value.
 * Here we represent the elements as a circular, doubly linked list, in the same
 * way as for the members of an object.  The "element_list" field in the ARGO_ARRAY
 * structure serves as the sentinel at the head of the list.
 *
 * Note that elements of an array do not have any name, so the "name" field in each
 * of the elements will be NULL.  Arrays could be represented as actual arrays,
 * but we are not doing that here.
 */
typedef struct argo_array {
    struct argo_value *element_list;
} ARGO_ARRAY;

/*
 * The ARGO_VALUE structure is used to represent all kinds of Argo values.
 * The "type" field tells what type of value it represents.
 * It has "next" and "prev" fields so that it can be linked into "members"
 * or "elements" lists.  It has a "name" field which will hold the name in case
 * it is a member of an object.  The "content" field is the union of the structures
 * that represent the various Argo types.  Depending on the value of the "type" field,
 * one of the "object", "array", or "string", "number", or "basic" variants of this union
 * will be valid.
 */
typedef struct argo_value {
    ARGO_VALUE_TYPE type;
    struct argo_value *next;           // Next value in list of members or elements.
    struct argo_value *prev;           // Previous value in list of members or element.
    struct argo_string name;           // NULL unless value is an object member.
    union {
	struct argo_object object;
	struct argo_array array;
	struct argo_string string;
	struct argo_number number;
	ARGO_BASIC basic;
    } content;
} ARGO_VALUE;

/*
 * The following value is the maximum number of digits that will be printed
 * for a floating point value.
 */
#define ARGO_PRECISION 15

/*
 * Macros that define particular character values mentioned in the Argo standard.
 * You should use these macros where reference to these character values is required,
 * rather than "hard-coding" the values as C character constants.
 */
#define ARGO_COLON ':'
#define ARGO_LBRACE '{'
#define ARGO_RBRACE '}'
#define ARGO_LBRACK '['
#define ARGO_RBRACK ']'
#define ARGO_QUOTE '"'
#define ARGO_BSLASH '\\'
#define ARGO_FSLASH '/'
#define ARGO_COMMA ','
#define ARGO_PERIOD '.'
#define ARGO_PLUS '+'
#define ARGO_MINUS '-'
#define ARGO_DIGIT0 '0'
#define ARGO_B 'b'
#define ARGO_E 'e'
#define ARGO_F 'f'
#define ARGO_N 'n'
#define ARGO_R 'r'
#define ARGO_T 't'
#define ARGO_U 'u'
#define ARGO_BS '\b'
#define ARGO_FF '\f'
#define ARGO_LF '\n'
#define ARGO_CR '\r'
#define ARGO_HT '\t'
#define ARGO_SPACE ' '

/*
 * Macros that define particular classes of characters mentioned in the Argo standard.
 * You should use these macros when it is necessary to test whether a character belongs
 * to a particular class, rather than "hard-coding" expressions involving C character
 * constants.
 */
#define argo_is_whitespace(c) ((c) == ' ' || (c) == '\n' || (c) == '\r' || c == '\t')
#define argo_is_exponent(c) ((c) == 'e' || (c) == 'E')
#define argo_is_digit(c) ((c) >= '0' && (c) <= '9')
#define argo_is_hex(c) (argo_is_digit(c) || ((c) >= 'A' && (c) <= 'F') || ((c) >= 'a' && (c) <= 'f'))
#define argo_is_control(c) ((c) >= 0 && (c) < ' ')

/*
 * Macros that define the tokens used to represent the basic values
 * "true", "false", and "null", defined by the Argo standard.
 * You should use these macros rather than "hard-coding" C string literals
 * into your program.
 */
#define ARGO_TRUE_TOKEN "true"
#define ARGO_FALSE_TOKEN "false"
#define ARGO_NULL_TOKEN "null"

/*
 * Variable that is reset to zero at the beginning of each line and is
 * incremented each time a character is read by function argo_read_char().
 * It is intended to be used for error messages and debugging.  It can be
 * assigned to if it is necessary to reset the value for some reason,
 * such as if reading from multiple sources is done.
 */

/*
 * The following function is used to append a character to a string.
 * An implementation has been provided for you.
 */
int argo_append_char(ARGO_STRING *, ARGO_CHAR);

#endif
