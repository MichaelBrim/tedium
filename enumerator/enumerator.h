/* USAGE INSTRUCTIONS
 *  1) copy enumerator.[ch] to new files
 *  2) in both new files, globally replace
 *    - '@@PREFIX@@' with desired prefix in uppercase
 *    - '@@prefix@@' with desired prefix in lowercase
 *  3) integrate with your project and enjoy
 */

#ifndef _@@PREFIX@@_ENUMERATOR_H_
#define _@@PREFIX@@_ENUMERATOR_H_

/* list expanded many times, each with a different ENUMITEM() definition
   macro args: (item name, item short description) */
#define @@PREFIX@@_ENUMERATOR \
 ENUMITEM(BAD_PARAM,"function parameter has invalid value") \
 ENUMITEM(INTERNAL_FAILURE,"internal failure") \
 ENUMITEM(NYI,"function has not been implemented")

#ifdef __cplusplus
extern "C" {
#endif

// enumeration is fun
typedef enum {
  @@PREFIX@@_ENUM_INVALID = 0,
#define ENUMITEM(name,desc) \
  @@PREFIX@@_ENUM_ ## name ,
@@PREFIX@@_ENUMERATOR
#undef ENUMITEM
  @@PREFIX@@_ENUM_MAX
} @@prefix@@_e;

const char* @@prefix@@_enum_str( @@prefix@@_e e );
const char* @@prefix@@_enum_description( @@prefix@@_e e );

int check_valid_@@prefix@@_enum( @@prefix@@_e );

#ifdef __cplusplus
} /* extern C */
#endif

#endif /* @@PREFIX@@_ENUMERATOR_H */
