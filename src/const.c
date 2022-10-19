/*
 * DO NOT MODIFY THE CONTENTS OF THIS FILE.
 * IT WILL BE REPLACED DURING GRADING
 */

#include <stdlib.h>

#include "argo.h"
#include "global.h"
#include "debug.h"

/**
 * @brief  Append a specified character to a specified string.
 * @details This function takes a pointer to an ARGO_STRING structure,
 * which is assumed previously to have been initialized (an ARGO_STRING
 * is initialized to empty by zeroing its fields) and an ARGO_CHAR
 * value, representing an arbitrary Unicode code point, and it appends
 * the character to the string, taking care of reallocating the string
 * content into a new memory area if its length has grown to exceed
 * the area already allocated for it.
 *
 * @return  Zero if the operation completes without error, nonzero if
 * there was a failure to allocate additional memory required to
 * accomodate the string.
 */
int argo_append_char(ARGO_STRING *s, ARGO_CHAR c) {
    if(s->capacity == 0) {
	s->capacity = 10;
	s->content = malloc(s->capacity * sizeof(ARGO_CHAR));
	if(!s->content) {
	    fprintf(stderr, "[%d] Failed to allocate space for string text",
		    argo_lines_read);
	    return 1;
	}
    }
    if(s->length == s->capacity) {
	s->capacity *= 2;
	s->content = realloc(s->content, s->capacity * sizeof(ARGO_CHAR));
	if(!s->content) {
	    fprintf(stderr, "[%d] Failed to allocate space for string text",
		    argo_lines_read);
	    return 1;
	}
    }
    s->content[s->length++] = c;
    return 0;
}
