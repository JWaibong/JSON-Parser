#include <stdlib.h>

#include "argo.h"
#include "global.h"
#include "debug.h"

int equals(char *c1, char *c2) {
    while(*c1 != '\0' && *c2 != '\0') {
        if (*c1 != *c2) {
            return 0;
        }
        c1++;
        c2++;
    }
    return 1;
}

int is_unsigned_integer(char* c) {
    int result = 0;
    while(*c != '\0') {
        if (!argo_is_digit(*c)) {
            return -1;
        }
        result = (result * 10) + (*c - '0');
        c++;
    }
    return result;
}
/**
 * @brief Validates command line arguments passed to the program.
 * @details This function will validate all the arguments passed to the
 * program, returning 0 if validation succeeds and -1 if validation fails.
 * Upon successful return, the various options that were specified will be
 * encoded in the global variable 'global_options', where it will be
 * accessible elsewhere in the program.  For details of the required
 * encoding, see the assignment handout.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return 0 if validation succeeds and -1 if validation fails.
 * @modifies global variable "global_options" to contain an encoded representation
 * of the selected program options.
 */

int validargs(int argc, char **argv) {
    int settings = 0;
    int count = 0;
    char* start = *argv; // pointer to first string of argv
    /*
    char* command_start = "bin/argo";
    if( equals(start, command_start) == 0) {
        return -1;
    } //check if first string in args is correct
    */
    count++;

    char* h_flag = "-h";
    char* c_flag = "-c";
    char* v_flag = "-v";
    argv++;
    if(equals(*argv, h_flag) == 1) {
        settings = settings | 0x80000000;
        global_options = settings;
        return 0; // presence of "-h" indicates success no matter if more arguments follow after
    }
    if (equals(*argv, v_flag) == 1) {
        count++;
        if(count == argc) {
            settings = settings | 0x40000000;
            global_options = settings;
            return 0;
        }
        return -1; // presence of "-v" indicates success only if no arguments follow
    }
    if (equals(*argv, c_flag) == 1) {
        settings = settings | 0x20000000;
        count++;
        argv++;
        if (count == argc) {
            global_options = settings;
            return 0;
        }

        char* p_flag = "-p";

        if( equals(*argv, p_flag) == 1) {
            settings = settings | 0x10000000;
            count++;
            if (count == argc) {
                settings = settings | 0x4;
                global_options = settings;
                return 0;
            }
            argv++;
            int p_flag_arg = is_unsigned_integer(*argv);
            if( p_flag_arg != -1 && p_flag_arg < 128) {
                settings = settings | p_flag_arg; // TODO: only applies to lsb 0-7, should check if p_flag_arg is within this range
                count++;  // "-p" flag includes an integer argument
            }
            else {
                return -1; // negative integer or invalid argument or >= 128
            }
        }

        if (count == argc) {
            global_options = settings;
            return 0;
        }
    } // presence of "-c" indicates success if either no arguments follow, "-p" follows, or "-p int"

    return -1;
}
