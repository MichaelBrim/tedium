/*  Copyright (c) 2018 - Michael J. Brim
 *
 *  See https://github.com/MichaelBrim/tedium/blob/master/LICENSE for licensing
 */

// USAGE NOTE: update following include to actual .h file name/location
#include "prefix_enumerator.h"

/* c-strings for enum names */

#define ENUMITEM(name,desc) \
const char* PREFIX_ ## name ## _NAME_STR = #name ;
PREFIX_ENUMERATOR
#undef ENUMITEM

const char* prefix_enum_str( prefix_e e )
{
    switch( e ) {
    case PREFIX_ENUM_INVALID : return "PREFIX_ENUM_INVALID";
#define ENUMITEM(name,desc) \
    case PREFIX_ENUM_ ## name : return PREFIX_ENUM_ ## name ## _NAME_STR ;
PREFIX_ENUMERATOR
#undef ENUMITEM
    default:
        break;
    }
    return NULL;
}

/* c-strings for enum descriptions */

#define ENUMITEM(name,desc) \
const char* PREFIX_ ## name ## _DESC_STR = #desc ;
PREFIX_ENUMERATOR
#undef ENUMITEM

const char* prefix_enum_description( prefix_e e )
{
    switch( e ) {
    case PREFIX_ENUM_INVALID : return "invalid prefix_e value";
#define ENUMITEM(name,desc) \
    case PREFIX_ENUM_ ## name : return PREFIX_ENUM_ ## name ## _DESC_STR ;
PREFIX_ENUMERATOR
#undef ENUMITEM
    default:
        break;
    }
    return NULL;
}

/* validity check */

int check_valid_prefix_enum( prefix_e e )
{
    return ((e > PREFIX_ENUM_INVALID) &&
            (e < PREFIX_ENUM_MAX) &&
            (NULL != prefix_enum_str(e)));
}

