/* USAGE INSTRUCTIONS
 *  1) copy enumerator.[ch] to new files
 *  2) in both new files, globally replace
 *    - '@@PREFIX@@' with desired prefix in uppercase
 *    - '@@prefix@@' with desired prefix in lowercase
 *  3) integrate with your project and enjoy
 */

// USAGE NOTE: update following include to actual .h file name/location
#include "@@prefix@@_enumerator.h"

/* c-strings for enum names */

#define ENUMITEM(name,desc) \
const char* @@PREFIX@@_ ## name ## _NAME_STR = #name ;
@@PREFIX@@_ENUMERATOR
#undef ENUMITEM

const char* @@prefix@@_enum_str( @@prefix@@_e e )
{
    switch( e ) {
    case @@PREFIX@@_ENUM_INVALID : return "@@PREFIX@@_ENUM_INVALID";
#define ENUMITEM(name,desc) \
    case @@PREFIX@@_ENUM_ ## name : return @@PREFIX@@_ENUM_ ## name ## _NAME_STR ;
@@PREFIX@@_ENUMERATOR
#undef ENUMITEM
    default:
        break;
    }
    return NULL;
}

/* c-strings for enum descriptions */

#define ENUMITEM(name,desc) \
const char* @@PREFIX@@_ ## name ## _DESC_STR = #desc ;
@@PREFIX@@_ENUMERATOR
#undef ENUMITEM

const char* @@prefix@@_enum_description( @@prefix@@_e e )
{
    switch( e ) {
    case @@PREFIX@@_ENUM_INVALID : return "invalid @@prefix@@_e value";
#define ENUMITEM(name,desc) \
    case @@PREFIX@@_ENUM_ ## name : return @@PREFIX@@_ENUM_ ## name ## _DESC_STR ;
@@PREFIX@@_ENUMERATOR
#undef ENUMITEM
    default:
        break;
    }
    return NULL;
}

/* validity check */

int check_valid_@@prefix@@_enum( @@prefix@@_e e )
{
    return ((e > @@PREFIX@@_ENUM_INVALID) &&
            (e < @@PREFIX@@_ENUM_MAX) &&
            (NULL != @@prefix@@_enum_str(e)));
}

