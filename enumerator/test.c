/*  Copyright (c) 2018 - Michael J. Brim
 *
 *  See https://github.com/MichaelBrim/tedium/blob/master/LICENSE for licensing
 */

#include <string.h>
#include <stdio.h>

#include "enumerator.h"

int main(int argc, char* argv[])
{
    prefix_e checkval;
    prefix_e testval;
    const char* estr;

    for( checkval = PREFIX_ENUM_INVALID+1; checkval < PREFIX_ENUM_MAX; checkval++ ) {
        printf("checking value %d - ", checkval);
        if( check_valid_prefix_enum(checkval) ) {
            estr = prefix_enum_str(checkval);
            printf("VALID\n\tname = %s\n\tdescription = %s\n",
                    estr, prefix_enum_description(checkval));
            testval = prefix_enum_from_str(estr);
            if( checkval != testval ) {
                printf("\tERROR on reverse lookup from string '%s'\n", estr);
            } 
        }
        else {
            printf("NOT DEFINED\n");
        }
    }

    return 0;
}
