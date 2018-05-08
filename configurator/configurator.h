/*  Copyright (c) 2018 - Michael J. Brim
 *
 *  Configurator is part of https://github.com/MichaelBrim/tedium
 *
 *  MIT License - See https://github.com/MichaelBrim/tedium/blob/master/LICENSE
 */

#ifndef _PREFIX_CONFIGURATOR_H_
#define _PREFIX_CONFIGURATOR_H_

/* Configurator unifies config files, environment variables, and command-line
   arguments into a set of simple preprocessor definitions that capture the
   necessary info.

   See README.md for instructions on usage.
*/ 

// need bool, NULL, FILE*
#ifdef __cplusplus
# include <cstddef>
# include <cstdio>
#else
# include <stdbool.h>
# include <stddef.h>
# include <stdio.h>
#endif

/* PREFIX_CONFIGS is the list of configuration settings, and should contain
   one macro definition per setting */
#define PREFIX_CONFIGS \
    PREFIX_CFG_CLI(prefix, configfile, STRING, /etc/prefix.conf, "path to configuration file", configurator_file_check, 'c', "specify full path to config file") \
    PREFIX_CFG_CLI(prefix, debug, BOOL, off, "enable debug output", NULL, 'd', "on|off") \
    PREFIX_CFG_CLI(log, verbosity, INT, 0, "log verbosity level", NULL, 'v', "specify logging verbosity level") \
    PREFIX_CFG_CLI(log, file, STRING, prefix.log, "log file name", NULL, 'l', "specify log file name") \
    PREFIX_CFG_CLI(log, dir, STRING, /tmp, "log file directory", configurator_directory_check, 'L', "specify full path to directory for placing log file") \
    


#ifdef __cplusplus
extern "C" {
#endif

    /* prefix_cfg_t struct */
    typedef struct {
#define PREFIX_CFG(sec, key, typ, dv, desc, vfn) \
        char* sec##_##key;

#define PREFIX_CFG_CLI(sec, key, typ, dv, desc, vfn, opt, use) \
        char* sec##_##key;

#define PREFIX_CFG_MULTI(sec, key, typ, dv, desc, vfn, me) \
        char* sec##_##key[me]; \
        unsigned n_##sec##_##key;

#define PREFIX_CFG_MULTI_CLI(sec, key, typ, dv, desc, vfn, me, opt, use) \
        char* sec##_##key[me]; \
        unsigned n_##sec##_##key;
        
        PREFIX_CONFIGS

#undef PREFIX_CFG
#undef PREFIX_CFG_CLI
#undef PREFIX_CFG_MULTI
#undef PREFIX_CFG_MULTI_CLI
    } prefix_cfg_t;

    /* initialization and cleanup */

    int prefix_config_init(prefix_cfg_t* cfg,
                           int argc,
                           char** argv);

    int prefix_config_fini(prefix_cfg_t* cfg);
                                   

    /* print configuration to specified file (or stderr if fp==NULL) */

    void prefix_config_print(prefix_cfg_t* cfg,
                             FILE* fp);

    /* print configuration in .INI format to specified file (or stderr) */

    void prefix_config_print_ini(prefix_cfg_t* cfg,
                                 FILE* inifp);

    /* used internally, but may be useful externally */

    int prefix_config_set_defaults(prefix_cfg_t* cfg);

    void prefix_config_cli_usage(char* arg0);
    void prefix_config_cli_usage_error(char* arg0,
                                       char* err_msg);

    int prefix_config_process_cli_args(prefix_cfg_t* cfg,
                                       int argc,
                                       char** argv);

    int prefix_config_process_environ(prefix_cfg_t* cfg);

    int prefix_config_process_file(prefix_cfg_t* cfg,
                                   const char* file);

    int prefix_config_validate(prefix_cfg_t* cfg);

    /* validate function prototype
       -  Returns: 0 for valid input, non-zero otherwise.
       -  out_val: set this output parameter to specify an alternate value */
    typedef int (*configurator_validate_fn)(const char* section,
                                            const char* key,
                                            const char* val,
                                            char** out_val);

    /* predefined validation functions */
    int configurator_bool_val(const char* val,
                              bool* b);
    int configurator_bool_check(const char* section,
                                const char* key,
                                const char* val,
                                char** oval);

    int configurator_float_val(const char* val,
                               double* d);
    int configurator_float_check(const char* section,
                                 const char* key,
                                 const char* val,
                                 char** oval);

    int configurator_int_val(const char* val,
                             long* l);
    int configurator_int_check(const char* section,
                               const char* key,
                               const char* val,
                               char** oval);

    int configurator_file_check(const char* section,
                                const char* key,
                                const char* val,
                                char** oval);

    int configurator_directory_check(const char* section,
                                     const char* key,
                                     const char* val,
                                     char** oval);


#ifdef __cplusplus
} /* extern C */
#endif

#endif /* PREFIX_CONFIGURATOR_H */
