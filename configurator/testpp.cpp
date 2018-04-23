/*  Copyright (c) 2018 - Michael J. Brim
 *
 *  See https://github.com/MichaelBrim/tedium/blob/master/LICENSE for licensing
 */

#include <cstring>
#include <cstdio>

#include "configurator.h"

int main(int argc, char* argv[])
{
    int rc;
    prefix_cfg_t mycfg;

    if( argc == 1 ) {
        prefix_config_cli_usage(argv[0]);
        return 1;
    }
    
    rc = prefix_config_init(&mycfg, argc, argv);
    if( rc ) {
        fprintf(stderr, "prefix_config_init() failed - rc=%d (%s)\n",
                rc, strerror(rc));
        return 1;
    }

    prefix_config_print(&mycfg, stdout);

    rc = prefix_config_fini(&mycfg);
    if( rc ) {
        fprintf(stderr, "prefix_config_fini() failed - rc=%d (%s)\n",
                rc, strerror(rc));
        return 1;
    }

    return 0;
}