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
    long l;
    double d;
    prefix_cfg_t mycfg;

    if( argc == 1 ) {
        prefix_config_cli_usage(argv[0]);
        return 1;
    }
    
    printf("TEST: initializing config\n");
    rc = prefix_config_init(&mycfg, argc, argv);
    if( rc ) {
        fprintf(stderr, "prefix_config_init() failed - rc=%d (%s)\n",
                rc, strerror(rc));
        return 1;
    }

    printf("TEST: printing human format to stdout\n");
    printf("========\n");
    prefix_config_print(&mycfg, stdout);
    printf("========\n\n");

    printf("TEST: printing .ini format to stderr\n");
    prefix_config_print_ini(&mycfg, stderr);
    printf("\n\n");

    if( 0 == configurator_int_val(mycfg.test_maxint, &l) )
        printf("TEST SUCCESS: test_maxint = %ld\n", l);
    else
        printf("TEST FAILURE: test_maxint (cfg=%s)\n", mycfg.test_maxint);

    if( 0 == configurator_int_val(mycfg.test_intexpr, &l) )
        printf("TEST SUCCESS: test_intexpr = %ld\n", l);
    else
        printf("TEST FAILURE: test_intexpr (cfg=%s)\n", mycfg.test_intexpr);

    if( 0 == configurator_float_val(mycfg.test_pi, &d) )
        printf("TEST SUCCESS: test_pi = %.6le\n", d);
    else
        printf("TEST FAILURE: test_pi (cfg=%s)\n", mycfg.test_pi);

    if( 0 == configurator_float_val(mycfg.test_exponent, &d) )
        printf("TEST SUCCESS: test_exponent = %.6le\n", d);
    else
        printf("TEST FAILURE: test_exponent (cfg=%s)\n", mycfg.test_exponent);

    if( 0 == configurator_float_val(mycfg.test_floatexpr, &d) )
        printf("TEST SUCCESS: test_floatexpr = %.6le\n", d);
    else
        printf("TEST FAILURE: test_floatexpr (cfg=%s)\n", mycfg.test_floatexpr);

    printf("TEST: finalizing config\n");
    rc = prefix_config_fini(&mycfg);
    if( rc ) {
        fprintf(stderr, "prefix_config_fini() failed - rc=%d (%s)\n",
                rc, strerror(rc));
        return 1;
    }

    return 0;
}
