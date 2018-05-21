/*  Copyright (c) 2018 - Michael J. Brim
 *
 *  See https://github.com/MichaelBrim/tedium/blob/master/LICENSE for licensing
 */

#include <cstring>
#include <cstdio>

#include "enumerator.h"

int main(int argc, char* argv[])
{
    int checkval;
    prefix_e testval;
    const char* estr;

    for( checkval = (int)PREFIX_ENUM_INVALID+1; checkval < (int)PREFIX_ENUM_MAX; checkval++ ) {
        printf("checking value %d - ", checkval);
        if( check_valid_prefix_enum((prefix_e)checkval) ) {
            estr = prefix_enum_str((prefix_e)checkval);
            printf("VALID\n\tname = %s\n\tdescription = %s\n",
                    estr, prefix_enum_description((prefix_e)checkval));
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
