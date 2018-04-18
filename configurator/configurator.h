#ifndef _PREFIX_CONFIGURATOR_H_
#define _PREFIX_CONFIGURATOR_H_

/* Configurator unifies config files, environment variables, and command-line
   arguments into a set of simple preprocessor definitions that capture the
   necessary info.

   Priority: (higher numbers override lower numbers)
     0. default values (hard-coded)
     1. config file settings
     2. environment variable settings
     3. command-line settings

   Files:
  
     .ini format config files are assumed, with the following format:
 
       [section1]
       key = val1
       key2 = another-value ; this is an inline comment

       # this is a full-line comment
       ; so is this

       [section2]
       key = val2

   Configuration Setting Macros:
    1. PREFIX_CFG( section, key, type, default-value, description, validate-fn )
    2. PREFIX_CFG_CLI( <PREFIX_CFG args>, cli-option-char, usage ) 
    3. PREFIX_CFG_MULTI( section, key, type, description, validate-fn, max-entries )
    4. PREFIX_CFG_MULTI_CLI( <PREFIX_CFG_MULTI args>, cli-option-char, usage )

     <section>, <key>, <description>, and <usage> should be constant C-strings

     Supported values for <type> are:   BOOL  | FLOAT  |   INT   | STRING
          with corresponding C types: uint8_t | double | int64_t |  char*

     <validate-fn> is an optional function pointer to validate config settings
     using STRING vals (see configurator_validate_fn typedef)
 
       int (validate_fn*)(const char* section,
                          const char* key,
                          const char* value,
                          char**      out_value)

       Return Value: 0 for valid input, non-zero otherwise.
        
       May optionally specify an alternate value by setting <out_value>


     The _MULTI forms allow configuration options to be defined multiple times, up
     to <max-entries>.

     The _CLI forms are used to define configuration options that are permitted to
     passed as command-line arguments. 


   Command-Line:

     POSIX getopt_long() is used for argument processing.

     --section-key[=val]  (long form)

     --key[=val]          (long form, for "section" == "prefix")

     -o [val]             (short form, with CLI option char 'o')

     NOTE: <val> is optional for BOOL settings only, 
           when not given, equivalent to "on"


   Environment variables:

     PREFIX_SECTION_KEY=val       (non _MULTI macro forms)

     PREFIX_SECTION_KEY_<#>=val   (_MULTI macro forms)   
*/

// need NULL and FILE*
#ifdef __cplusplus
# include <cstddef>
# include <cstdio>
#else
# include <stddef.h>
# include <stdio.h>
#endif

// need fixed bit-size types [u]int{8,16,32,64}_t
#if defined(__cplusplus) && __cplusplus >= 201103L
// C++11
#include <cstdint>
#include <cinttypes>
#else
// C99
#include <stdint.h>
#include <inttypes.h>
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

    int configurator_bool_check(const char* section,
                                const char* key,
                                const char* val,
                                char** oval);

    int configurator_float_check(const char* section,
                                 const char* key,
                                 const char* val,
                                 char** oval);

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
