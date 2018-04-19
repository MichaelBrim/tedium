/*  Copyright (c) 2018 - Michael J. Brim
 *
 *  See https://github.com/MichaelBrim/tedium/blob/master/LICENSE for licensing
 */

#ifndef _PREFIX_ENUMERATOR_H_
#define _PREFIX_ENUMERATOR_H_

/* list expanded many times, each with a different ENUMITEM() definition
   macro args: (item name, item short description) */
#define PREFIX_ENUMERATOR \
 ENUMITEM(BAD_PARAM,"function parameter has invalid value") \
 ENUMITEM(INTERNAL_FAILURE,"internal failure") \
 ENUMITEM(NYI,"function has not been implemented")

#ifdef __cplusplus
extern "C" {
#endif

// enumeration is fun
typedef enum {
  PREFIX_ENUM_INVALID = 0,
#define ENUMITEM(name,desc) \
  PREFIX_ENUM_ ## name ,
PREFIX_ENUMERATOR
#undef ENUMITEM
  PREFIX_ENUM_MAX
} prefix_e;

const char* prefix_enum_str( prefix_e e );
const char* prefix_enum_description( prefix_e e );

int check_valid_prefix_enum( prefix_e );

#ifdef __cplusplus
} /* extern C */
#endif

#endif /* PREFIX_ENUMERATOR_H */
