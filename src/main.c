#include <stdio.h>
#include <stdlib.h>

#include "argo.h"
#include "global.h"
#include "debug.h"

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif


int main(int argc, char **argv)
{
   int parse_input_success = validargs(argc, argv);
    if(parse_input_success == -1) {
        USAGE(*argv, EXIT_FAILURE);
    }
    if(global_options == HELP_OPTION) {
        USAGE(*argv, EXIT_SUCCESS);
    }
    if(parse_input_success == 0 && ((global_options & VALIDATE_OPTION) == VALIDATE_OPTION)) {
        ARGO_VALUE *val = argo_read_value(stdin);
        if(val != NULL) {
            return EXIT_SUCCESS;
        }
        fprintf(stderr, "Validation failed. Failed to parse json file.%c", '\n');
        return EXIT_FAILURE;
    }
    if(parse_input_success == 0 && ((global_options & CANONICALIZE_OPTION) == CANONICALIZE_OPTION)) {

        //USAGE(*argv, EXIT_SUCCESS); // replace later
        ARGO_VALUE *val = argo_read_value(stdin);
        if(val == NULL) {
            fprintf(stderr, "Failed to parse json file.%c",'\n');
            return EXIT_FAILURE;
        }
        if(argo_write_value(val, stdout)) {
            fprintf(stderr, "Failed to write json to output stream. %c", '\n');
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }

    // TO BE IMPLEMENTED
    return EXIT_FAILURE;



}
// bin/argo -c < rsrc/numbers.json > numbers.out

/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */
