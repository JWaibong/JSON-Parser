#include <stdlib.h>
#include <stdio.h>

#include "argo.h"
#include "global.h"
#include "debug.h"

#define argo_is_upper_hex(c) ('A' <= c && c <= 'F')
#define PRETTY_PRINT(global_options) (PRETTY_PRINT_OPTION == ((global_options & PRETTY_PRINT_OPTION)))
#define INDENT_SPACES (global_options & 0x7F)

// TODO: ANYTIME CALL FGETC IN THE CODE, CHECK FOR EOF FIRST
// DO NOT ASSIGN TO CHAR FIRST, CHECK EOF FIRST!!!!
// USE GOTO MAYBE? 
// Don’t return pointers to a function’s local variables. I THINK I DID THIS ONCE
int argo_next_value = 0;

int fpeek(FILE* f) {
    int result = fgetc(f);
    ungetc(result, f);
    if(result == EOF){
        return -1;
    }
    return result;
}

int argo_read_object(ARGO_VALUE* members, FILE* f) {
    ARGO_VALUE *sentinel = members;
    int len = 0;
    int obj_char = fgetc(f);
    if(obj_char == EOF) {
        return -1;
    }
    while(argo_is_whitespace(obj_char)) {
        obj_char = fgetc(f); // consume all whitespace characters until reach first non whitepsace
    } 
    if(obj_char == EOF) {
        return -1;
    }
    if(obj_char == ARGO_RBRACE) {
        sentinel->next = sentinel;
        sentinel->prev = sentinel;
        return 0;
    } // checking for empty object
    ungetc(obj_char, f); // push the quote '"' back on the stream
    while(obj_char != ARGO_RBRACE) {
        obj_char = fgetc(f);
        if(obj_char == EOF) {
            return -1;
        }
        if(obj_char == ARGO_QUOTE) {
            sentinel->name.content = NULL;
            sentinel->name.length = 0;
            sentinel->name.capacity = 0;
            if (argo_read_string(&sentinel->name, f)) {
                return -1; //failure to read string
            }
            obj_char = fpeek(f);
            if(obj_char == EOF) {
                return -1;
            }
            if(obj_char == ARGO_COLON || argo_is_whitespace(obj_char)) {
                ungetc(ARGO_SPACE, f);
            }
            continue;
        }
        else if (argo_is_whitespace(obj_char)) {
            // consume all white space following the string in string :value pair
            while(argo_is_whitespace(obj_char)) {
                obj_char = fgetc(f);
            }
            if(obj_char == EOF) {
                return -1;
            }
            if(obj_char == ARGO_COLON) {
                argo_next_value++;
                if(argo_next_value >= NUM_ARGO_VALUES) {
                    return -1;
                }
                members->next = argo_read_value(f);
                if(members->next == NULL) {
                    return -1;
                }
                members->next->name = sentinel->name;
                members->next->name.content = (sentinel->name.content);
                members = members->next;
                len++;
                continue;
            }
            if(obj_char == ARGO_RBRACE) {
                continue;
            }
        }
        else if (obj_char == ARGO_COMMA) {
            int peek = fgetc(f);
            if(peek == EOF || peek == ARGO_RBRACE) {
                if(peek == EOF){
                    fprintf(stderr, "Error: found EOF after comma.%c", '\n');
                }
                else {
                    fprintf(stderr, "Error: found %c after comma.%c", peek,'\n');
                }
                return -1;
            }
            if(argo_is_whitespace(peek)) {
                obj_char = fgetc(f);
                if(obj_char == EOF) {
                    return -1;
                }
                // consume all white space following COMMA ,
                while(argo_is_whitespace(obj_char)) {
                    obj_char = fgetc(f);
                }
                if(obj_char == EOF) {
                    return -1;
                }
                // only continue if first non white space is QUOTE "
                // because only possibility is to continue parsing string :value pairs
                if (obj_char == ARGO_QUOTE) {
                    ungetc(obj_char, f);
                    continue;
                }
                if(obj_char == ARGO_RBRACE) {
                    break;
                }
                return -1;
            }
            return -1;
            // cannot follow COMMA , immediately with RBACE }
            // or any other character
        }
        else {
            return -1;
        }
    }
    if (obj_char == EOF) {
        return -1; //premature EOF
    }
    // members will be pointing at the last element of the list
    members->next = sentinel; // so its next pointer will be sentinel
    sentinel->prev = members; // sentinel->prev points to last element of list
    for(int i=0; i<len; i++) {
        sentinel = sentinel->next;
        members = members->next;
        sentinel->prev = members; //iterate through entire list again, making sure to set the prev pointers for all elements in list
    }
    return 0;
}

int argo_read_array(ARGO_VALUE* elements, FILE* f) {
    ARGO_VALUE *sentinel = elements;
    int len = 0;
    int arr_char = fgetc(f);
    if(arr_char == EOF) {
        return -1;
    }
    while(argo_is_whitespace(arr_char)) {
        arr_char = fgetc(f);
    }
    if(arr_char == EOF) {
        return -1;
    }
    if(arr_char == ARGO_RBRACK) {
        sentinel->next = sentinel;
        sentinel->prev = sentinel;
        return 0;
    }
    ungetc(arr_char, f);
    while(arr_char != ARGO_RBRACK && arr_char > 0) {
        argo_next_value++;
        if(argo_next_value > NUM_ARGO_VALUES) {
            return -1;
        }
        ARGO_VALUE *v = argo_read_value(f);
        if(v == NULL) {
            return -1;
        }
        elements->next = v;
        elements = elements->next;
        len++;
        int potential_comma = fgetc(f);
        if(potential_comma == EOF) {
            return -1;
        }
        if(potential_comma == ARGO_COMMA) {
            arr_char = fgetc(f);
            if(arr_char == EOF) {
                return -1; 
            }
            while(argo_is_whitespace(arr_char)) {
                arr_char = fgetc(f);
            }
            // check for white space after last element
            if(arr_char == ARGO_RBRACK || arr_char == EOF) {
                if(arr_char == EOF){
                    fprintf(stderr, "Error: found EOF after comma.%c", '\n');
                }
                else {
                    fprintf(stderr, "Error: found %c after comma.%c", arr_char,'\n');
                }
                return -1;
            }
            ungetc(arr_char, f);
            continue;
        }
        else if (argo_is_whitespace(potential_comma) || potential_comma == ARGO_RBRACK) {
            while(argo_is_whitespace(potential_comma)) {
                potential_comma = fgetc(f);
            }
            if(potential_comma == ARGO_RBRACK) {
                break;
            }
            return -1;
        }
        arr_char = fgetc(f);
        if(arr_char == EOF) {
            return -1;
        }
    }
    if(arr_char <= 0) {
        return -1;
    }
    elements->next = sentinel; // last element points to sentinel
    sentinel->prev = elements; // sentinel points to last element
    for(int i=0; i<len; i++) {
        sentinel = sentinel->next;
        elements = elements->next;
        sentinel->prev = elements;
    }
    return 0;
}

/**
 * @brief  Read JSON input from a specified input stream, parse it,
 * and return a data structure representing the corresponding value.
 * @details  This function reads a sequence of 8-bit bytes from
 * a specified input stream and attempts to parse it as a JSON value,
 * according to the JSON syntax standard.  If the input can be
 * successfully parsed, then a pointer to a data structure representing
 * the corresponding value is returned.  See the assignment handout for
 * information on the JSON syntax standard and how parsing can be
 * accomplished.  As discussed in the assignment handout, the returned
 * pointer must be to one of the elements of the argo_value_storage
 * array that is defined in the const.h header file.
 * In case of an error (these include failure of the input to conform
 * to the JSON standard, premature EOF on the input stream, as well as
 * other I/O errors), a one-line error message is output to standard error
 * and a NULL pointer value is returned.
 *
 * @param f  Input stream from which JSON is to be read.
 * @return  A valid pointer if the operation is completely successful,
 * NULL if there is any error.
 */
ARGO_VALUE* argo_read_value(FILE *f) {
// need to implement incrementing argo_value_storage whenever a value is successfully created.
// TODO: MAKE NOTE OF NEXT CHAR IN STREAM BEFORE CALLING ANOTHER READ FUNCTION

// TODO: PASS IN THE ADDRESS OF ARGO_VALUE_STORAGE FOR READING ARRAYS AND OBJECTS

    if(argo_next_value > NUM_ARGO_VALUES) {
        return NULL;
    }
    ARGO_VALUE *v = argo_value_storage + argo_next_value;
    int val_first = fgetc(f);
    if(val_first == EOF) {
        return NULL;
    }
    int c = 0;
    int parse_sucess = 0; 
    while(argo_is_whitespace(val_first)) {
        val_first = fgetc(f);
    }
    if(val_first == EOF) {
        return NULL;
    }
    switch(val_first) {
        case ARGO_LBRACE: // { consumed off stream, OBJECT
            argo_next_value++; // next value is sentinel
            if(argo_next_value >= NUM_ARGO_VALUES) {
                return NULL; // cannot have > 100k values
            } 
            v->content.object.member_list = argo_value_storage + argo_next_value; // sentinel address
            parse_sucess = argo_read_object(v->content.object.member_list, f);
            if (parse_sucess == 0) {
                //argo_next_value++;
                v->type = ARGO_OBJECT_TYPE;
                return v; 
            }
            break;
        case ARGO_LBRACK: // [ ARRAY consumed off stream, OBJECT
            argo_next_value++; // next value is sentinel
            if(argo_next_value > NUM_ARGO_VALUES) {
                return NULL;
            } 
            v->content.array.element_list = argo_value_storage + argo_next_value; // sentinel address
            parse_sucess = argo_read_array(v->content.array.element_list, f);
            if(parse_sucess == 0) {
                v->type = ARGO_ARRAY_TYPE;
                return v; 
            }
            break;
        case ARGO_QUOTE: // " consumed off stream, STRING
            v->type = ARGO_STRING_TYPE;
            parse_sucess = argo_read_string(&v->content.string, f);
            if(parse_sucess == 0) {
                return v; 
            }
            break;
        case ARGO_T: // check for true
            c = fgetc(f);
            if(c != 'r') {
                return NULL;
            }
            c = fgetc(f);
            if(c!= 'u') {
                return NULL;
            }
            c = fgetc(f);
            if(c!= 'e') {
                return NULL;
            }
            v->type = ARGO_BASIC_TYPE;
            v->content.basic = ARGO_TRUE;
            //argo_next_value++;
            return v; 
        case ARGO_F: //check for false
            c = fgetc(f);
            if(c != 'a') {
                return NULL;
            }
            c = fgetc(f);
            if(c!= 'l') {
                return NULL;
            }
            c = fgetc(f);
            if(c != 's') {
                return NULL;
            }
            c = fgetc(f);
            if(c != 'e') {
                return NULL;
            }
            v->type = ARGO_BASIC_TYPE;
            v->content.basic = ARGO_FALSE;
            // argo_next_value++;
            return v;
        case ARGO_N: //check for null
            c = fgetc(f);
            if(c != 'u') {
                return NULL;
            }
            c = fgetc(f);
            if(c!= 'l') {
                return NULL;
            }
            c = fgetc(f);
            if(c != 'l') {
                return NULL;
            }
            v->type = ARGO_BASIC_TYPE;
            v->content.basic = ARGO_NULL;
            // argo_next_value++;
            return v;
        default:
            if(argo_is_digit(val_first) || val_first == ARGO_MINUS) {
                // NUMBER
                ungetc(val_first, f); // push char back onto the stream in order to parse the number correctly
                parse_sucess = argo_read_number(&v->content.number, f);
                if(parse_sucess == 0) {
                    // argo_next_value++;
                    v->type = ARGO_NUMBER_TYPE;
                    return v;
                }
            }
            /*if(argo_is_whitespace(val_first) || val_first < 0) {
                return v;
            }*/
            break;
    }
    return NULL;
}


/**
 * @brief  Read JSON input from a specified input stream, attempt to
 * parse it as a JSON string literal, and return a data structure
 * representing the corresponding string.
 * @details  This function reads a sequence of 8-bit bytes from
 * a specified input stream and attempts to parse it as a JSON string
 * literal, according to the JSON syntax standard.  If the input can be
 * successfully parsed, then a pointer to a data structure representing
 * the corresponding value is returned.
 * In case of an error (these include failure of the input to conform
 * to the JSON standard, premature EOF on the input stream, as well as
 * other I/O errors), a one-line error message is output to standard error
 * and a NULL pointer value is returned.
 *
 * @param f  Input stream from which JSON is to be read.
 * @return  Zero if the operation is completely successful,
 * nonzero if there is any error.
 */
int argo_read_string(ARGO_STRING *s, FILE *f) {
    int c = fgetc(f);
    if (c == EOF) {
        return -1;
    }
    while(c != ARGO_QUOTE) {
        if(c == ARGO_BSLASH) {
            c = fgetc(f);
            if(c == EOF) {
                return -1;
            }
            switch(c) {
                case ARGO_QUOTE:
                    argo_append_char(s, ARGO_QUOTE);
                    break;
                case ARGO_BSLASH:
                    argo_append_char(s, ARGO_BSLASH);
                    break;
                case ARGO_FSLASH:
                    argo_append_char(s, ARGO_FSLASH);
                    break;
                case ARGO_B: // \b
                    argo_append_char(s, ARGO_BS);
                    break;
                case ARGO_F: // \f
                    argo_append_char(s, ARGO_FF);
                    break;
                case ARGO_N: // \n
                    argo_append_char(s, ARGO_LF);
                    break;
                case ARGO_R: // \r
                    argo_append_char(s, ARGO_CR);
                    break;
                case ARGO_T: // \t
                    argo_append_char(s, ARGO_HT);
                    break;
                case ARGO_U: ; // \u
                    int first = fgetc(f);
                    if(first == EOF) {
                        return -1;
                    }
                    int second = fgetc(f);
                    if(second == EOF) {
                        return -1;
                    }
                    int third = fgetc(f);
                    if(third == EOF) {
                        return -1;
                    }
                    int fourth = fgetc(f);
                    if(fourth == EOF) {
                        return -1;
                    }

                    if(argo_is_hex(first) && argo_is_hex(second) && argo_is_hex(third) && argo_is_hex(fourth)) {
                        int first_hex = 0;
                        int second_hex = 0;
                        int third_hex = 0;
                        int fourth_hex = 0;
                        if(argo_is_upper_hex(first)) {
                            first = first + 32;
                        }
                        if(argo_is_upper_hex(second)) {
                            second = second + 32;
                        }
                        if(argo_is_upper_hex(third)) {
                            third = third + 32;
                        }
                        if(argo_is_upper_hex(fourth)) {
                            fourth = fourth + 32;
                        }

                        if(argo_is_digit(first)) {
                            first_hex = 16 * 16 * 16 * (first - 48);
                        }
                        else { // lower hex digit
                            first_hex = 16 * 16 * 16 * (first - 87);
                        }

                        if(argo_is_digit(second)) {
                            second_hex = 16 * 16 * (second - 48);
                        }
                        else { // lower hex digit
                            second_hex = 16 * 16 * (second - 87);
                        }
                        if(argo_is_digit(third)) {
                            third_hex = 16 * (third - 48);
                        }
                        else { // lower hex digit
                            third_hex = 16 * (third - 87);
                        }
                        if(argo_is_digit(fourth)) {
                            fourth_hex = (fourth - 48);
                        }
                        else { // lower hex digit
                            fourth_hex = (fourth - 87);
                        }
                        argo_append_char(s, first_hex + second_hex + third_hex + fourth_hex);
                        if(fpeek(f) == -1) {
                            return -1;
                        }
                        c = fgetc(f);
                        continue;
                    }
                return -1; // there are not 4 valid hex characters following \u
                default:
                    return -1; // invalid escape char following '\' 
            } // end of switch statement
            if(fpeek(f) == ARGO_QUOTE) {
                c = fgetc(f);
                break;
            }
        } // end of if c== '\' statement
        else if(argo_is_control(c)) {
            return -1;
        }
        else { 
            argo_append_char(s, c);
            c = fgetc(f);
            if(c == EOF) {
                return -1;
            }
            continue;
        }
        c = fgetc(f);
        if(c == EOF) {
            return -1;
        }
    } // end of while statement
    if (c == ARGO_QUOTE) {
        return 0;
    }
    // either the while loop stopped because EOF was read or ARGO_QUOTE was read
    // exit successfully only if ARGO_QUOTE
    return -1;
}



/**
 * @brief  Read JSON input from a specified input stream, attempt to
 * parse it as a JSON number, and return a data structure representing
 * the corresponding number.
 * @details  This function reads a sequence of 8-bit bytes from
 * a specified input stream and attempts to parse it as a JSON numeric
 * literal, according to the JSON syntax standard.  If the input can be
 * successfully parsed, then a pointer to a data structure representing
 * the corresponding value is returned.  The returned value must contain
 * (1) a string consisting of the actual sequence of characters read from
 * the input stream; (2) a floating point representation of the corresponding
 * value; and (3) an integer representation of the corresponding value,
 * in case the input literal did not contain any fraction or exponent parts.
 * In case of an error (these include failure of the input to conform
 * to the JSON standard, premature EOF on the input stream, as well as
 * other I/O errors), a one-line error message is output to standard error
 * and a NULL pointer value is returned.
 *
 * @param f  Input stream from which JSON is to be read.
 * @return  Zero if the operation is completely successful,
 * nonzero if there is any error.
 */
int argo_read_number(ARGO_NUMBER *n, FILE *f) {
    ARGO_STRING *s = &(n->string_value);
    int c = fgetc(f); 
    // FROM CALLING THIS FUNCTION, THIS VALUE IS GUARANTEED TO BE A DIGIT OR '-' THE FIRST TIME
    if(c==EOF) { // CHECK ANYWAYS BECAUSE WHY NOT
        return -1;
    }
    int negative = 1; // set to 1 if positive, -1 if negative
    int valid_float = 0; // set to 0 if int, set to 1 if float
    long long e_val = 0; // value of exponent if any
    int e_sign = 1; //  sign of exponent. -1 if negative
    long long i_val = 0; // value of unsigned integer part if any
    double d_val = 0; // value of part after decimal point
    double f_val = 0;  // value of actual float
    int d_count = 10;
    if(c == ARGO_MINUS) {
        argo_append_char(s, ARGO_MINUS);
        negative = -1;
        c = fgetc(f); // consume a digit from the stream
        if(!(argo_is_digit(c))) {
            return -1;
        }
    }
    if(c == ARGO_DIGIT0) {
        argo_append_char(s, ARGO_DIGIT0);
        c = fgetc(f); //next char. It will be '.' or 'e' or 'E' or EOF or , or whitespace
        if(argo_is_digit(c)) {
            return -1; // leading 0s not allowed in int or double
        }
    }
    if(argo_is_digit(c)) {
        while(argo_is_digit(c)) {
            argo_append_char(s, c);
            long long digit = (i_val * 10) + (c-48);
            if(digit < i_val) {
                return -1;
            }
            i_val = digit; 
            c=fgetc(f);
        }
    }
    if(c == ARGO_PERIOD) {
            argo_append_char(s, ARGO_PERIOD);       
            c = fgetc(f); // look at digit
            if(!(argo_is_digit(c)) || c==EOF ) {
                return -1;
            }
            while((argo_is_digit(c))) {
                argo_append_char(s,c);
                d_val = d_val + ((double)(c-48))/d_count;
                d_count *= 10;
                c = fgetc(f);
            }
            
            // after exiting while, c can be 'e' 'E' EOF
            f_val = (d_val + i_val);
            valid_float = 1;
            // EOF CHECKING IS WEIRD HERE BECAUSE IT CAN JUST BE 1 NUMBER AND EOF
        }
    if(argo_is_exponent(c)) {
            valid_float = 1;
            f_val = (d_val + i_val);
            argo_append_char(s, ARGO_E);
            c = fgetc(f); // either decimal, '+' or '-'
            if(c==EOF ) {
                return -1; // cannot EOF or be non digit after e or E
            }
            if(c == ARGO_PLUS) {
                c = fgetc(f);
                if(c==EOF || c==ARGO_MINUS) {
                    return -1;
                }
            }
            if(c == ARGO_MINUS) {
                c = fgetc(f);
                if(c==EOF) {
                    return -1;
                }
                e_sign = -1;
                argo_append_char(s, ARGO_MINUS);
            }
            if(argo_is_digit(c)) { // get rid of all leading 0 
                while(c==ARGO_DIGIT0) {
                    c = fgetc(f);
                    argo_append_char(s, ARGO_DIGIT0);
                }
                if(c==EOF || !(argo_is_digit(c))) { // end 
                    ungetc(c,f);
                    n->valid_float = 1; 
                    n->valid_string = 1;
                    n->float_value = f_val;
                    return 0;
                }
                if(argo_is_digit(c)) {
                    while(argo_is_digit(c)) {
                        argo_append_char(s, c);
                        if((e_val*10) + (c-48) < e_val) {
                            return -1;
                        }
                        e_val = (e_val*10) + (c-48);
                        c = fgetc(f);
                    }
                }
                if(e_sign == 1) {
                    for(int i=0; i<e_val; i++) {
                        f_val *= 10.0;
                    }
                }
                if (e_sign == -1) {
                    for(int i=0; i<e_val; i++) {
                        f_val /= 10.0;
                    }
                }
            }
        }
        if(argo_is_whitespace(c) || c==EOF || c==ARGO_COMMA || c == ARGO_RBRACE || c == ARGO_RBRACK) {
            ungetc(c, f);
        }
        if(valid_float == 0) {
            n->valid_string = 1;
            n->valid_int = 1;
            n->int_value = i_val * negative;
            return 0;
        }
        if(valid_float) {
            n->valid_string = 1;
            n->valid_float = 1;
            n->float_value = f_val * negative;
            return 0;
        }
    return -1;
}


int argo_write_basic(ARGO_BASIC *, FILE *);
int argo_write_object(ARGO_OBJECT *, FILE *);
int argo_write_array(ARGO_ARRAY *, FILE *);

/**
 * @brief  Write canonical JSON representing a specified value to
 * a specified output stream.
 * @details  Write canonical JSON representing a specified value
 * to specified output stream.  See the assignment document for a
 * detailed discussion of the data structure and what is meant by
 * canonical JSON.
 *
 * @param v  Data structure representing a value.
 * @param f  Output stream to which JSON is to be written.
 * @return  Zero if the operation is completely successful,
 * nonzero if there is any error.
 */

int argo_write_value(ARGO_VALUE *v, FILE *f) {

    switch(v->type) {
        case 0: // ARGO_NO_TYPE
                // base case for recursion
            break;
        case 1: ; // ARGO_BASIC_TYPE
            argo_write_basic(&(v->content.basic),f);
            break;
        case 2: // ARGO_NUMBER_TYPE
            argo_write_number(&(v->content.number), f);
            if((PRETTY_PRINT(global_options)) && indent_level == 0) {
                fputc(ARGO_LF, f);
                for(int i=0; i<indent_level*INDENT_SPACES; i++) {
                    fputc(ARGO_SPACE, f);
                }
            }
            break;
        case 3: // ARGO_STRING_TYPE
            argo_write_string(&(v->content.string), f);
            if((PRETTY_PRINT(global_options)) && indent_level == 0) {
                fputc(ARGO_LF, f);
                for(int i=0; i<indent_level*INDENT_SPACES; i++) {
                    fputc(ARGO_SPACE, f);
                }
            }
            break;
        case 4: // ARGO_OBJECT_TYPE
            argo_write_object(&(v->content.object), f);
            break;
        case 5: // ARGO_ARRAY_TYPE
            argo_write_array(&(v->content.array), f);
            break;
        default:
            break;
    }
    if ((PRETTY_PRINT(global_options)) && indent_level == 0) {
        fputc(ARGO_LF, f);
        for(int i=0; i<indent_level*INDENT_SPACES; i++) {
            fputc(ARGO_SPACE, f);
        }
    }
    if(ferror(f)) {
        return -1;
    }
    return 0;
}

int argo_write_basic(ARGO_BASIC *b, FILE *f) {
    if(*b == ARGO_TRUE) {
                // write true
        fputs(ARGO_TRUE_TOKEN, f);
    }
    else if (*b == ARGO_FALSE) {
        fputs(ARGO_FALSE_TOKEN, f);
                // write false
    }
    else {
                // write null
        fputs(ARGO_NULL_TOKEN,f);
    }

    return 0;
}

int argo_write_array(ARGO_ARRAY *a, FILE *f) {
            fputc(ARGO_LBRACK, f);
            if(PRETTY_PRINT(global_options)) {
                fputc(ARGO_LF, f);
                indent_level++;
                for(int i=0; i<indent_level*INDENT_SPACES; i++) {
                        fputc(ARGO_SPACE, f);
                }
            }
            ARGO_VALUE *arr_head = a->element_list;
            ARGO_VALUE *original_arr_head = arr_head;

            argo_write_value(arr_head, f);
            arr_head = arr_head->next;
            while(arr_head->next != original_arr_head) {
                argo_write_value(arr_head, f);
                fputc(ARGO_COMMA, f);
                if(PRETTY_PRINT(global_options)) {
                    fputc(ARGO_LF, f);
                    for(int i=0; i<indent_level*INDENT_SPACES; i++) {
                        fputc(ARGO_SPACE, f);
                    }
                }
                arr_head = arr_head->next;
            }
            argo_write_value(arr_head, f);
            if(PRETTY_PRINT(global_options)) {
                indent_level--;
                fputc(ARGO_LF, f);
                for(int i=0; i<indent_level*INDENT_SPACES; i++) {
                    fputc(ARGO_SPACE, f);
                }
            }
            fputc(ARGO_RBRACK, f);
    return 0;
}

int argo_write_object(ARGO_OBJECT *o, FILE *f) {
            ARGO_VALUE *object_head = o->member_list;
            ARGO_VALUE *original_object_head = object_head;
            fputc(ARGO_LBRACE, f);
            if(PRETTY_PRINT(global_options)) {
                fputc(ARGO_LF, f);
                indent_level++;
                for(int i=0; i<indent_level*INDENT_SPACES; i++) {
                        fputc(ARGO_SPACE, f);
                }
            }
            argo_write_value(object_head, f);
            object_head = object_head->next;
            while(object_head->next != original_object_head) {
                argo_write_string(&(object_head->name),f);
                fputc(ARGO_COLON, f);
                if(PRETTY_PRINT(global_options)) {
                    fputc(ARGO_SPACE, f);
                }
                argo_write_value(object_head, f);
                fputc(ARGO_COMMA, f);
                if(PRETTY_PRINT(global_options)) {
                    fputc(ARGO_LF, f);
                    for(int i=0; i<indent_level*INDENT_SPACES; i++) {
                        fputc(ARGO_SPACE, f);
                    }
                }
                object_head = object_head->next;
             }
            argo_write_string(&(object_head->name), f);
            fputc(ARGO_COLON, f);
            if(PRETTY_PRINT(global_options)) {
                fputc(ARGO_SPACE, f);
            }
            argo_write_value(object_head, f);
            if(PRETTY_PRINT(global_options)) {
                indent_level--;
                fputc(ARGO_LF, f);
                for(int i=0; i<indent_level*INDENT_SPACES; i++) {
                    fputc(ARGO_SPACE, f);
                }
            }
             // write }
            fputc(ARGO_RBRACE, f);
            return 0;
}

int argo_write_u_escape(ARGO_CHAR val, FILE* f) {
    fputc(ARGO_BSLASH, f);
    fputc(ARGO_U, f);

    // hard code the 4 HEX digits
    char first = (val % 16) + 87;
    val = val / 16;

    char second =  (val % 16) + 87;

    val = val / 16;

    char third = (val % 16) + 87;

    val = val /16 ;

    char fourth = (val % 16) + 87;

    if (fourth >= 'W'  && !(argo_is_hex(fourth))) {
        fputc(fourth-39, f );
    }
    else {
        fputc(fourth, f);
    }

    if (third >= 'W'  && !(argo_is_hex(third))) {
        fputc(third-39, f);
    }
    else {
        fputc(third, f);
    }

    if (second >= 'W'  && !(argo_is_hex(second))) {
        fputc(second-39, f);
    }
    else {
        fputc(second, f);
    }

    if (first >= 'W' && !(argo_is_hex(first))) {
        fputc(first-39, f);
    }
    else {
        fputc(first, f);
    }

    return 0;
}

/**
 * @brief  Write canonical JSON representing a specified string
 * to a specified output stream.
 * @details  Write canonical JSON representing a specified string
 * to specified output stream.  See the assignment document for a
 * detailed discussion of the data structure and what is meant by
 * canonical JSON.  The argument string may contain any sequence of
 * Unicode code points and the output is a JSON string literal,
 * represented using only 8-bit bytes.  Therefore, any Unicode code
 * with a value greater than or equal to U+00FF cannot appear directly
 * in the output and must be represented by an escape sequence.
 * There are other requirements on the use of escape sequences;
 * see the assignment handout for details.
 *
 * @param v  Data structure representing a string (a sequence of
 * Unicode code points).
 * @param f  Output stream to which JSON is to be written.
 * @return  Zero if the operation is completely successful,
 * nonzero if there is any error.
 */
int argo_write_string(ARGO_STRING *s, FILE *f) {
    // write \"

    fputc(ARGO_QUOTE, f);
    ARGO_CHAR *c = s->content; //equivalent to int *
    // Unicode code point, 4 bytes max
    // EOF == -1
//    \ followed by
//    "   34
//  '\' 92
//  '/' 47
//    'b'
//    'f'
//    'n'
//    'r'
//    't'
//    'uHHHH'
    for(int i=0; i<s->length; i++, c++) {
        ARGO_CHAR current = *c;
        switch(current) {
            case ARGO_BS:
                fputc(ARGO_BSLASH,f);
                fputc(ARGO_B,f);
                break;
            case ARGO_FF:
                fputc(ARGO_BSLASH,f);
                fputc(ARGO_F,f);
                break;
            case ARGO_LF:
                fputc(ARGO_BSLASH,f);
                fputc(ARGO_N,f);
                break;
            case ARGO_CR:
                fputc(ARGO_BSLASH,f);
                fputc(ARGO_R,f);
                break;
            case ARGO_HT:
                fputc(ARGO_BSLASH,f);
                fputc(ARGO_T,f);
                break;
            case ARGO_QUOTE:
                fputc(ARGO_BSLASH,f);
                fputc(ARGO_QUOTE,f);
                break;
            case ARGO_BSLASH:
                fputc(ARGO_BSLASH, f);
                fputc(ARGO_BSLASH, f);
                break;
            case ARGO_FSLASH:
                fputc(ARGO_BSLASH, f);
                fputc(ARGO_FSLASH, f);
                break;
            default:
                if(current >= 32 && current <= 255) {
                    fputc(current, f);
                }
                else if (current > 255) { // outside multillingual plane
                    argo_write_u_escape(current, f);
                }
                else { // current < 32, its a control character
                    argo_write_u_escape(current,f);
                }
                break;
            }
        }
    // write \"
    fputc(ARGO_QUOTE, f);
    return 0;
}

/**
 * @brief  Write canonical JSON representing a specified number
 * to a specified output stream.
 * @details  Write canonical JSON representing a specified number
 * to specified output stream.  See the assignment document for a
 * detailed discussion of the data structure and what is meant by
 * canonical JSON.  The argument number may contain representations
 * of the number as any or all of: string conforming to the
 * specification for a JSON number (but not necessarily canonical),
 * integer value, or floating point value.  This function should
 * be able to work properly regardless of which subset of these
 * representations is present.
 *
 * @param v  Data structure representing a number.
 * @param f  Output stream to which JSON is to be written.
 * @return  Zero if the operation is completely successful,
 * nonzero if there is any error.
 */
int argo_write_number(ARGO_NUMBER *n, FILE *f) {
    ARGO_CHAR *int_digits = argo_digits;
    if(n->valid_int) {
        long long int_val = n->int_value;
        if(int_val < 0) {
            fputc(ARGO_MINUS,f);
            int_val *= -1;
        }
        if(int_val == 0) {
            fputc(ARGO_DIGIT0, f);
            return 0;
        }

        for(int i=0; i<ARGO_MAX_DIGITS; i++, int_digits++) {
            *int_digits = (int_val % 10) + 48;
            int_val = int_val / 10;
        }
        int_digits--;
        while(*int_digits == ARGO_DIGIT0) {
            int_digits--;
        }
        while(int_digits >= argo_digits) {
            fputc(*int_digits, f);
            int_digits--;
        }
    }
    else if(n->valid_float) {
        double d_val = n->float_value;

        if(d_val == 0) {
            fputs("0.0",f);
            return 0;
        }
        int e_val = 0;
        if(d_val < 0) {
            fputc(ARGO_MINUS, f);
            d_val *= -1;
        }
        // guaranteed d_val > 0 at this point
        double d_val_copy = d_val; 
        if(d_val_copy > 1) {
            while(d_val_copy >= 1) {
                d_val_copy = d_val_copy / 10;
                e_val++;
            }
        }
        else if (d_val_copy < 1) {
            while(d_val_copy <= 1) {
                d_val_copy = d_val_copy * 10;
                e_val--;
            }
            while(d_val_copy >= 1) {
                d_val_copy = d_val_copy / 10;
                e_val++;
            }
        }
        else {
            fputs("0.1e1",f);
            return 0;
        }
        
        if(e_val == -1) {
            d_val_copy = d_val_copy * 10;
            e_val++;
        }
        fputc(ARGO_DIGIT0, f);
        fputc(ARGO_PERIOD, f);
        
        for(int i=0; i<ARGO_PRECISION; i++) {
            d_val_copy = d_val_copy * 10;
            fputc(((int)(d_val_copy)) + 48, f);
            d_val_copy = d_val_copy - (int)d_val_copy; 
        }
        if(e_val == 0) {
            return 0;
        }
        fputc(ARGO_E, f);
        if (e_val < 0) {
            fputc(ARGO_MINUS,f);
            e_val *= -1;
        }

        for(int i=0; i<ARGO_MAX_DIGITS; i++, int_digits++) {
            *int_digits = (e_val % 10) + 48;
            e_val = e_val / 10;
        }
        int_digits--;
        while(*int_digits == ARGO_DIGIT0) {
            int_digits--;
        }
        while(int_digits >= argo_digits) {
            fputc(*int_digits, f);
            int_digits--;
        }
        return 0;

    }
    else if (n->valid_string) {
        ARGO_CHAR *c = n->string_value.content;
        size_t len = n->string_value.length;
        if(len > 0) {
            for(int i=0; i<len; i++,c++) {
                fputc(*c,f);
            }
        }
        else {
            fputs("null",f);
        }
    }
    else {
        return -1;
    }
    return 0;
}
