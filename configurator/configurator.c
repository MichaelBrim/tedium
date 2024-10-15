/*  Copyright (c) 2018 - Michael J. Brim
 *
 *  Configurator is part of https://github.com/MichaelBrim/tedium
 *
 *  MIT License - See https://github.com/MichaelBrim/tedium/blob/master/LICENSE
 */

#ifdef __cplusplus
# include <cassert>
# include <cctype>
# include <cerrno>
# include <cstddef>
# include <cstdlib>
# include <cstring>
#else
# include <assert.h>
# include <ctype.h>
# include <errno.h>
# include <stddef.h>
# include <stdlib.h>
# include <string.h>
#endif

#include <getopt.h>   // getopt_long()
#include <sys/stat.h> // stat()
#include <unistd.h>

#include <ini.h>
#include <tinyexpr.h>
#include <parser.h>


// CONFIGURATOR USAGE NOTE: update following to actual .h file name/location
#include "configurator.h"

#define PREFIX_CFG_MAX_MSG 1024

#define stringify_indirect(x) #x
#define stringify(x) stringify_indirect(x)


// initialize configuration using all available methods
int prefix_config_init(prefix_cfg_t* cfg,
                       int argc,
                       char** argv)
{
    int rc;
    char* syscfg = NULL;

    if( NULL == cfg )
        return -1;

    memset((void*)cfg, 0, sizeof(prefix_cfg_t));
    
    // set default configuration
    rc = prefix_config_set_defaults(cfg);
    if( rc ) return rc;

    // process system config file (if available)
    syscfg = cfg->prefix_configfile;
    rc = configurator_file_check(NULL, NULL, syscfg, NULL);
    if( 0 == rc ) {
        rc = prefix_config_process_file(cfg, syscfg);
        if( rc ) return rc;
    }
    if( NULL != syscfg )
        free(syscfg);
    cfg->prefix_configfile = NULL;
    
    // process environment (overrides defaults and system config)
    rc = prefix_config_process_environ(cfg);
    if( rc ) return rc;
    
    // process command-line args (overrides all previous)
    rc = prefix_config_process_cli_args(cfg, argc, argv);
    if( rc ) return rc;

    // read config file passed on command-line (does not override cli args)
    if( NULL != cfg->prefix_configfile ) {
        rc = prefix_config_process_file(cfg, cfg->prefix_configfile);
        if( rc ) return rc;
    }

    // validate settings
    rc = prefix_config_validate(cfg);
    if( rc ) return rc;

    return 0;
}

// cleanup allocated state
int prefix_config_fini(prefix_cfg_t* cfg)
{
    unsigned u;

    if( NULL == cfg )
        return -1;
    
#define PREFIX_CFG(sec, key, typ, dv, desc, vfn)        \
    if( NULL != cfg->sec##_##key ) {                    \
        free(cfg->sec##_##key);                         \
        cfg->sec##_##key = NULL;                        \
    }

#define PREFIX_CFG_CLI(sec, key, typ, dv, desc, vfn, opt, use)  \
    if( NULL != cfg->sec##_##key ) {                            \
        free(cfg->sec##_##key);                                 \
        cfg->sec##_##key = NULL;                                \
    }

#define PREFIX_CFG_MULTI(sec, key, typ, desc, vfn, me)      \
    for( u=0; u < me; u++ ) {                                   \
        if( NULL != cfg->sec##_##key[u] ) {                     \
            free(cfg->sec##_##key[u]);                          \
            cfg->sec##_##key[u] = NULL;                         \
        }                                                       \
    }                                                           \
    cfg->n_##sec##_##key = 0;

#define PREFIX_CFG_MULTI_CLI(sec, key, typ, desc, vfn, me, opt, use) \
    for( u=0; u < me; u++ ) {                                           \
        if( NULL != cfg->sec##_##key[u] ) {                             \
            free(cfg->sec##_##key[u]);                                  \
            cfg->sec##_##key[u] = NULL;                                 \
        }                                                               \
    }                                                                   \
    cfg->n_##sec##_##key = 0;

    PREFIX_CONFIGS;
#undef PREFIX_CFG
#undef PREFIX_CFG_CLI
#undef PREFIX_CFG_MULTI
#undef PREFIX_CFG_MULTI_CLI

    return 0;
}

// print configuration to specified file (or stderr)
void prefix_config_print(prefix_cfg_t* cfg,
                         FILE* fp)
{
    unsigned u;
    char msg[PREFIX_CFG_MAX_MSG];

    if( NULL == fp )
        fp = stderr;

#define PREFIX_CFG(sec, key, typ, dv, desc, vfn)                \
    if( NULL != cfg->sec##_##key ) {                            \
        snprintf(msg, sizeof(msg), "PREFIX CONFIG: %s.%s = %s", \
                 #sec, #key, cfg->sec##_##key);                 \
        fprintf(fp, "%s\n", msg);                               \
    }

#define PREFIX_CFG_CLI(sec, key, typ, dv, desc, vfn, opt, use)  \
    if( NULL != cfg->sec##_##key ) {                            \
        snprintf(msg, sizeof(msg), "PREFIX CONFIG: %s.%s = %s", \
                 #sec, #key, cfg->sec##_##key);                 \
        fprintf(fp, "%s\n", msg);                               \
    }

#define PREFIX_CFG_MULTI(sec, key, typ, desc, vfn, me)              \
    for( u=0; u < me; u++ ) {                                           \
        if( NULL != cfg->sec##_##key[u] ) {                             \
            snprintf(msg, sizeof(msg), "PREFIX CONFIG: %s.%s[%u] = %s", \
                     #sec, #key, u+1, cfg->sec##_##key[u]);             \
            fprintf(fp, "%s\n", msg);                                   \
        }                                                               \
    }

#define PREFIX_CFG_MULTI_CLI(sec, key, typ, desc, vfn, me, opt, use) \
    for( u=0; u < me; u++ ) {                                           \
        if( NULL != cfg->sec##_##key[u] ) {                             \
            snprintf(msg, sizeof(msg), "PREFIX CONFIG: %s.%s[%u] = %s", \
                     #sec, #key, u+1, cfg->sec##_##key[u]);             \
            fprintf(fp, "%s\n", msg);                                   \
        }                                                               \
    }
    
    PREFIX_CONFIGS;
#undef PREFIX_CFG
#undef PREFIX_CFG_CLI
#undef PREFIX_CFG_MULTI
#undef PREFIX_CFG_MULTI_CLI

    fflush(fp);
}

// print configuration in .INI format to specified file (or stderr)
void prefix_config_print_ini(prefix_cfg_t* cfg,
                             FILE* inifp)
{
    unsigned u;
    const char* curr_sec = NULL;
    const char* last_sec = NULL;

    if( NULL == inifp )
        inifp = stderr;

#define PREFIX_CFG(sec, key, typ, dv, desc, vfn)                        \
    if( NULL != cfg->sec##_##key ) {                                    \
        curr_sec = #sec;                                                \
        if( (NULL == last_sec) || (0 != strcmp(curr_sec, last_sec)) )   \
            fprintf(inifp, "\n[%s]\n", curr_sec);                       \
        fprintf(inifp, "%s = %s\n", #key, cfg->sec##_##key);            \
        last_sec = curr_sec;                                            \
    }

#define PREFIX_CFG_CLI(sec, key, typ, dv, desc, vfn, opt, use)          \
    if( NULL != cfg->sec##_##key ) {                                    \
        curr_sec = #sec;                                                \
        if( (NULL == last_sec) || (0 != strcmp(curr_sec, last_sec)) )   \
            fprintf(inifp, "\n[%s]\n", curr_sec);                       \
        fprintf(inifp, "%s = %s\n", #key, cfg->sec##_##key);            \
        last_sec = curr_sec;                                            \
    }

#define PREFIX_CFG_MULTI(sec, key, typ, desc, vfn, me)                  \
    for( u=0; u < me; u++ ) {                                           \
        if( NULL != cfg->sec##_##key[u] ) {                             \
            curr_sec = #sec;                                            \
            if( (NULL == last_sec) || (0 != strcmp(curr_sec, last_sec)) ) \
                fprintf(inifp, "\n[%s]\n", curr_sec);                   \
            fprintf(inifp, "%s = %s ; (instance %u)\n",                 \
                    #key, cfg->sec##_##key[u], u+1);                    \
            last_sec = curr_sec;                                        \
        }                                                               \
    }

#define PREFIX_CFG_MULTI_CLI(sec, key, typ, desc, vfn, me, opt, use)    \
    for( u=0; u < me; u++ ) {                                           \
        if( NULL != cfg->sec##_##key[u] ) {                             \
            curr_sec = #sec;                                            \
            if( (NULL == last_sec) || (0 != strcmp(curr_sec, last_sec)) ) \
                fprintf(inifp, "\n[%s]\n", curr_sec);                   \
            fprintf(inifp, "%s = %s ; (instance %u)\n",                 \
                    #key, cfg->sec##_##key[u], u+1);                    \
            last_sec = curr_sec;                                        \
        }                                                               \
    }

    PREFIX_CONFIGS;
#undef PREFIX_CFG
#undef PREFIX_CFG_CLI
#undef PREFIX_CFG_MULTI
#undef PREFIX_CFG_MULTI_CLI

    fflush(inifp);
}

// set default values given in PREFIX_CONFIGS
int prefix_config_set_defaults(prefix_cfg_t* cfg)
{
    int rc;
    char* val;

    if( NULL == cfg )
        return -1;
    
#define PREFIX_CFG(sec, key, typ, dv, desc, vfn)        \
    val = stringify(dv);                                \
    if( 0 != strcmp(val, "NULLSTRING") ) {              \
        cfg->sec##_##key = strdup(val);                 \
    }

#define PREFIX_CFG_CLI(sec, key, typ, dv, desc, vfn, opt, use)  \
    val = stringify(dv);                                        \
    if( 0 != strcmp(val, "NULLSTRING") ) {                      \
        cfg->sec##_##key = strdup(val);                         \
    }

#define PREFIX_CFG_MULTI(sec, key, typ, desc, vfn, me)              \
    cfg->n_##sec##_##key = 0;                                           \
    memset((void*)cfg->sec##_##key, 0, sizeof(cfg->sec##_##key));

#define PREFIX_CFG_MULTI_CLI(sec, key, typ, desc, vfn, me, opt, use) \
    cfg->n_##sec##_##key = 0;                                           \
    memset((void*)cfg->sec##_##key, 0, sizeof(cfg->sec##_##key));

    PREFIX_CONFIGS;
#undef PREFIX_CFG
#undef PREFIX_CFG_CLI
#undef PREFIX_CFG_MULTI
#undef PREFIX_CFG_MULTI_CLI
        
    return 0;
}


// utility routine to print CLI usage (and optional usage error message)
void prefix_config_cli_usage(char* arg0)
{    
    fprintf(stderr, "USAGE: %s [options]\n", arg0);

#define PREFIX_CFG(sec, key, typ, dv, desc, vfn) 

#define PREFIX_CFG_CLI(sec, key, typ, dv, desc, vfn, opt, use)         \
    fprintf(stderr, "    -%c,--%s-%s <%s>\t%s (default value: %s)\n",    \
            opt, #sec, #key, #typ, use, stringify(dv));

#define PREFIX_CFG_MULTI(sec, key, typ, desc, vfn, me) 

#define PREFIX_CFG_MULTI_CLI(sec, key, typ, desc, vfn, me, opt, use) \
    fprintf(stderr, "    -%c,--%s-%s <%s>\t%s (multiple values supported - max %u entries)\n", \
            opt, #sec, #key, #typ, use, me);

    PREFIX_CONFIGS;
#undef PREFIX_CFG
#undef PREFIX_CFG_CLI
#undef PREFIX_CFG_MULTI
#undef PREFIX_CFG_MULTI_CLI

    fflush(stderr);
}

// print usage error message
void prefix_config_cli_usage_error(char* arg0,
                                   char* err_msg)
{
    if( NULL != err_msg )
        fprintf(stderr, "USAGE ERROR: %s : %s\n\n", arg0, err_msg);
    
    prefix_config_cli_usage(arg0);
}


static struct option cli_options[] = {
#define PREFIX_CFG(sec, key, typ, dv, desc, vfn)
#define PREFIX_CFG_CLI(sec, key, typ, dv, desc, vfn, opt, use)  \
    { #sec "-" #key, required_argument, NULL, opt },
#define PREFIX_CFG_MULTI(sec, key, typ, desc, vfn, me) 
#define PREFIX_CFG_MULTI_CLI(sec, key, typ, desc, vfn, me, opt, use) \
    { #sec "-" #key, required_argument, NULL, opt },
    PREFIX_CONFIGS
#undef PREFIX_CFG
#undef PREFIX_CFG_CLI
#undef PREFIX_CFG_MULTI
#undef PREFIX_CFG_MULTI_CLI
    { NULL, 0, NULL, 0 }
};

// update config struct based on command line args
int prefix_config_process_cli_args(prefix_cfg_t* cfg,
                                   int argc,
                                   char** argv)
{
    int rc, c;
    int usage_err = 0;
    int ondx = 0;
    int sndx = 0;
    char errmsg[PREFIX_CFG_MAX_MSG];
    char short_opts[256];
    extern char* optarg;
    extern int optind, optopt;

    if( NULL == cfg )
        return -1;

    // setup short_opts and cli_options
    memset((void*)short_opts, 0, sizeof(short_opts));
    short_opts[sndx++] = ':'; // report missing args

#define PREFIX_CFG(sec, key, typ, dv, desc, vfn) 

#define PREFIX_CFG_CLI(sec, key, typ, dv, desc, vfn, opt, use)  \
    short_opts[sndx++] = opt;                                   \
    if( 0 == strcmp(#typ, "BOOL") ) {                           \
        short_opts[sndx++] = ':';                               \
        short_opts[sndx++] = ':';                               \
        cli_options[ondx++].has_arg = optional_argument;        \
    }                                                           \
    else {                                                      \
        short_opts[sndx++] = ':';                               \
        cli_options[ondx++].has_arg = required_argument;        \
    }
#define PREFIX_CFG_MULTI(sec, key, typ, desc, vfn, me) 

#define PREFIX_CFG_MULTI_CLI(sec, key, typ, desc, vfn, me, opt, use)    \
    short_opts[sndx++] = opt;                                           \
    if( 0 == strcmp(#typ, "BOOL") ) {                                   \
        short_opts[sndx++] = ':';                                       \
        short_opts[sndx++] = ':';                                       \
        cli_options[ondx++].has_arg = optional_argument;                \
    }                                                                   \
    else {                                                              \
        short_opts[sndx++] = ':';                                       \
        cli_options[ondx++].has_arg = required_argument;                \
    }

    PREFIX_CONFIGS;
#undef PREFIX_CFG
#undef PREFIX_CFG_CLI
#undef PREFIX_CFG_MULTI
#undef PREFIX_CFG_MULTI_CLI

    //fprintf(stderr, "PREFIX CONFIG DEBUG: short-opts '%s'\n", short_opts);

    // process argv
    while( -1 != (c = getopt_long(argc, argv, short_opts, cli_options, NULL)) ) {
        switch( c ) {

#define PREFIX_CFG(sec, key, typ, dv, desc, vfn) 

#define PREFIX_CFG_CLI(sec, key, typ, dv, desc, vfn, opt, use)  \
            case opt: {                                         \
                if( optarg )                                    \
                    cfg->sec##_##key = strdup(optarg);          \
                else if( 0 == strcmp(#typ, "BOOL") )            \
                    cfg->sec##_##key = strdup("on");            \
                break;                                          \
            }

#define PREFIX_CFG_MULTI(sec, key, typ, desc, vfn, me) 

#define PREFIX_CFG_MULTI_CLI(sec, key, typ, desc, vfn, me, opt, use)    \
            case opt: {                                                 \
                cfg->sec##_##key[cfg->n_##sec##_##key++] = strdup(optarg); \
                break;                                                  \
            }

            PREFIX_CONFIGS;
#undef PREFIX_CFG
#undef PREFIX_CFG_CLI
#undef PREFIX_CFG_MULTI
#undef PREFIX_CFG_MULTI_CLI

        case ':':
            usage_err = 1;
            snprintf(errmsg, sizeof(errmsg), 
                     "CLI option -%c requires operand", optopt);
            break;
        case '?':
            usage_err = 1;
            snprintf(errmsg, sizeof(errmsg), 
                     "unknown CLI option -%c", optopt);
            break;
        default:
            // should not reach here
            fprintf(stderr, "PREFIX CONFIG DEBUG: unhandled option '%s'\n", optarg);
            break;
        }
        if( usage_err ) break;
    }

    if( ! usage_err ) 
        rc = 0;
    else {
        rc = -1;
        prefix_config_cli_usage_error(argv[0], errmsg);
    }
        
    return rc;
}

// helper to check environment variable
char* getenv_helper(const char* section,
                    const char* key,
                    unsigned mentry)
{
    static char envname[256];
    unsigned u;
    size_t len;
    size_t ndx = 0;

    memset((void*)envname, 0, sizeof(envname));

    
    ndx += sprintf(envname, "PREFIX_");

    if( 0 != strcmp(section, "prefix") ) {
        len = strlen(section);
        for( u=0; u < len; u++ )
            envname[ndx+u] = toupper(section[u]);
        ndx += len;
        envname[ndx++] = '_';
    }

    len = strlen(key);
    for( u=0; u < len; u++ )
        envname[ndx+u] = toupper(key[u]);
    ndx += len;

    if( mentry )
        ndx += sprintf(envname+ndx, "_%u", mentry);
    
    //fprintf(stderr, "PREFIX CONFIG DEBUG: checking env var %s\n", envname);
    return getenv(envname);
}
                    

// update config struct based on environment variables
int prefix_config_process_environ(prefix_cfg_t* cfg)
{
    char* envval;

    if( NULL == cfg )
        return -1;

    
#define PREFIX_CFG(sec, key, typ, dv, desc, vfn)        \
    envval = getenv_helper(#sec, #key, 0);              \
    if( NULL != envval ) {                              \
        cfg->sec##_##key = strdup(envval);              \
    }

#define PREFIX_CFG_CLI(sec, key, typ, dv, desc, vfn, opt, use)  \
    envval = getenv_helper(#sec, #key, 0);                      \
    if( NULL != envval ) {                                      \
        cfg->sec##_##key = strdup(envval);                      \
    }

#define PREFIX_CFG_MULTI(sec, key, typ, desc, vfn, me)  \
    for( u=0; u < me; u++ ) {                           \
        envval = getenv_helper(#sec, #key, u+1);        \
        if( NULL != envval ) {                          \
            cfg->sec##_##key[u] = strdup(envval);       \
            cfg->n_##sec##_##key++;                     \
        }                                               \
    }

#define PREFIX_CFG_MULTI_CLI(sec, key, typ, desc, vfn, me, opt, use)    \
    for( u=0; u < me; u++ ) {                                           \
        envval = getenv_helper(#sec, #key, u+1);                        \
        if( NULL != envval ) {                                          \
            cfg->sec##_##key[u] = strdup(envval);                       \
            cfg->n_##sec##_##key++;                                     \
        }                                                               \
    }

    PREFIX_CONFIGS;
#undef PREFIX_CFG
#undef PREFIX_CFG_CLI
#undef PREFIX_CFG_MULTI
#undef PREFIX_CFG_MULTI_CLI

    return 0;
}

// inih callback handler
int inih_config_handler(void* user,
                        const char* section,
                        const char* kee,
                        const char* val)
{
    char* curval;
    char* defval;
    prefix_cfg_t* cfg = (prefix_cfg_t*) user;
    assert( NULL != cfg );

    // if not already set by CLI args, set cfg cfgs
    if( 0 ) ;

#define PREFIX_CFG(sec, key, typ, dv, desc, vfn)                        \
    else if( (0 == strcmp(section, #sec)) && (0 == strcmp(kee, #key)) ) { \
        curval = cfg->sec##_##key;                                      \
        defval = stringify(dv);                                         \
        if( (NULL == curval) || (0 == strcmp(defval, curval)) )         \
            cfg->sec##_##key = strdup(val);                             \
    }

#define PREFIX_CFG_CLI(sec, key, typ, dv, desc, vfn, opt, use)          \
    else if( (0 == strcmp(section, #sec)) && (0 == strcmp(kee, #key)) ) { \
        curval = cfg->sec##_##key;                                      \
        defval = stringify(dv);                                         \
        if( (NULL == curval) || (0 == strcmp(defval, curval)) )         \
            cfg->sec##_##key = strdup(val);                             \
    }

#define PREFIX_CFG_MULTI(sec, key, typ, desc, vfn, me)                  \
    else if( (0 == strcmp(section, #sec)) && (0 == strcmp(kee, #key)) ) { \
        cfg->sec##_##key[cfg->n_##sec##_##key++] = strdup(val);         \
    }

#define PREFIX_CFG_MULTI_CLI(sec, key, typ, desc, vfn, me, opt, use)    \
    else if( (0 == strcmp(section, #sec)) && (0 == strcmp(kee, #key)) ) { \
        cfg->sec##_##key[cfg->n_##sec##_##key++] = strdup(val);         \
    }

    PREFIX_CONFIGS;
#undef PREFIX_CFG
#undef PREFIX_CFG_CLI
#undef PREFIX_CFG_MULTI
#undef PREFIX_CFG_MULTI_CLI

    return 1;
}

// json callback handler
void json_config_handler(enum NanoJSONCError error, const char *const key, const char *const value,
                        const char *const section, void *object)
{
    if (error != NO_ERROR)
    {
        return;
    }
    char* simple_section = section ? strdup(section) : NULL;

    if(simple_section)
    {
        simple_section[strlen(simple_section)-1] = '\0';
        inih_config_handler(object, simple_section + sizeof(char), key, value);

    }
    else
    {
        inih_config_handler(object, NULL, key, value);
    }

    free(simple_section);
}

int json_parse(const char* filename, NanoJSONCCallback json_config_handler, prefix_cfg_t* cfg)
{
    int error = -1;
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        return -1; // File could not be opened
    }

    // Get file size
    struct stat sb;
    if(stat(filename, &sb) != 0)
    {
        goto cleanup;
    }

    // Allocate buffer for file content
    char* source = malloc(sizeof(char) * (sb.st_size + 1));
    if (!source) {
        goto cleanup; // Memory allocation failed
    }

    // Read the entire file into memory
    size_t newLen = fread(source, sizeof(char), sb.st_size, fp);
    if (ferror(fp)) {
        goto cleanup; // Error reading file
    }

    source[newLen+1] = '\0'; /* Just to be safe. */
    error = 0;
    nanojsonc_parse_object(source, NULL, cfg, json_config_handler);

cleanup:
    if (source)
        free(source);
    fclose(fp);
    return error;
}

int ini_error_handler(const int errcode, const char* file_path) {
    int rc;
    char errmsg[PREFIX_CFG_MAX_MSG];
    switch( errcode ) {
    case 0:
        rc = 0;
        break;
    case -1:
        snprintf(errmsg, sizeof(errmsg),
                 "failed to open config file %s",
                 file_path);
        fprintf(stderr, "PREFIX CONFIG ERROR: %s\n", errmsg);
        rc = ENOENT;
        break;
    case -2:
        snprintf(errmsg, sizeof(errmsg),
                 "failed to parse config file %s",
                 file_path);
        fprintf(stderr, "PREFIX CONFIG ERROR: %s\n", errmsg);
        rc = ENOMEM;
        break;
    default:
        /* > 0  indicates parse error at line */
        if( errcode > 0 )
            snprintf(errmsg, sizeof(errmsg),
                     "parse error at line %d of config file %s",
                     errcode, file_path);
        else
            snprintf(errmsg, sizeof(errmsg),
                     "failed to parse config file %s",
                     file_path);
        rc = EINVAL;
        fprintf(stderr, "PREFIX CONFIG ERROR: %s\n", errmsg);
        break;
    }
    return rc;
}

int json_error_handler(const int errcode, const char* file_path)
{
    int rc;
    char errmsg[PREFIX_CFG_MAX_MSG];
    switch( errcode ) {
    case 0:
            rc = 0;
        break;
    case -1:
        snprintf(errmsg, sizeof(errmsg),
                     "failed to open config file %s",
                     file_path);
        fprintf(stderr, "PREFIX CONFIG ERROR: %s\n", errmsg);
        rc = ENOENT;
        break;
    case -2:
        snprintf(errmsg, sizeof(errmsg),
                     "failed to parse config file %s",
                     file_path);
        fprintf(stderr, "PREFIX CONFIG ERROR: %s\n", errmsg);
        rc = ENOMEM;
        break;
    default:
        /* > 0  indicates parse error at line */
        if( errcode > 0 )
            snprintf(errmsg, sizeof(errmsg),
                     "parse error at line %d of config file %s",
                     errcode, file_path);
        else
            snprintf(errmsg, sizeof(errmsg),
                     "failed to parse config file %s",
                     file_path);
        rc = EINVAL;
        fprintf(stderr, "PREFIX CONFIG ERROR: %s\n", errmsg);
        break;
    }
    return rc;
}

//int yaml_error_handler()
//{
//  return 0;
//}

// update config struct based on config file, using inih
int prefix_config_process_file(prefix_cfg_t* cfg,
                               const char* file)
{
    int rc;

    if( NULL == cfg )
        return rc;
    
    if( NULL == file )
        return EINVAL;

    // Determine the filetype based on extension
    char* ext = strrchr(file, '.');
    if (!ext) {
        // no extension, so assume ini
        ext = "ini";
    }
    else
    {
        ext++;
    }

    if (strcmp(ext, "cfg") == 0 || strcmp(ext, "ini") == 0) {
        const int ini_error = ini_parse(file, inih_config_handler, cfg);
        rc = ini_error_handler(ini_error, file);
    }
    else if (strcmp(ext, "json") == 0) {
        const int json_error = json_parse(file, json_config_handler, cfg);
        rc = json_error_handler(json_error, file);
    }
//    else if (strcmp(ext, "yaml") == 0 || strcmp(ext, "yml") == 0) {
//        rc = yaml_error_handler();
//    }

    return rc;
}


/* predefined validation functions */

// utility routine to validate a single value given function
int validate_value(const char* section,
                   const char* key,
                   const char* val,
                   const char* typ,
                   configurator_validate_fn vfn,
                   char** new_val)
{
    if( NULL != vfn ) {
        return vfn(section, key, val, new_val);
    }
    else if( 0 == strcmp(typ, "BOOL") ) {
        return configurator_bool_check(section, key, val, NULL);
    }
    else if( 0 == strcmp(typ, "INT") ) {
        return configurator_int_check(section, key, val, new_val);
    }
    else if( 0 == strcmp(typ, "FLOAT") ) {
        return configurator_float_check(section, key, val, new_val);
    }
    return 0;
}


// validate configuration
int prefix_config_validate(prefix_cfg_t* cfg)
{
    unsigned u;
    int rc = 0;
    int vrc;
    char* new_val = NULL;

    if( NULL == cfg )
        return EINVAL;

#define PREFIX_CFG(sec, key, typ, dv, desc, vfn)                        \
    vrc = validate_value(#sec, #key, cfg->sec##_##key, #typ, vfn, &new_val); \
    if( vrc ) {                                                         \
        rc = vrc;                                                       \
        fprintf(stderr, "PREFIX CONFIG ERROR: value '%s' for %s.%s is INVALID %s\n", \
                cfg->sec##_##key, #sec, #key, #typ);                    \
    } else if( NULL != new_val ) {                                      \
        if( NULL != cfg->sec##_##key ) free(cfg->sec##_##key);          \
        cfg->sec##_##key = new_val;                                     \
        new_val = NULL;                                                 \
    }

#define PREFIX_CFG_CLI(sec, key, typ, dv, desc, vfn, opt, use)          \
    vrc = validate_value(#sec, #key, cfg->sec##_##key, #typ, vfn, &new_val); \
    if( vrc ) {                                                         \
        rc = vrc;                                                       \
        fprintf(stderr, "PREFIX CONFIG ERROR: value '%s' for %s.%s is INVALID %s\n", \
                cfg->sec##_##key, #sec, #key, #typ);                    \
    } else if( NULL != new_val ) {                                      \
        if( NULL != cfg->sec##_##key ) free(cfg->sec##_##key);          \
        cfg->sec##_##key = new_val;                                     \
        new_val = NULL;                                                 \
    }

#define PREFIX_CFG_MULTI(sec, key, typ, desc, vfn, me)                  \
    for( u=0; u < me; u++ ) {                                           \
        vrc = validate_value(#sec, #key, cfg->sec##_##key[u], #typ, vfn, &new_val); \
        if( vrc ) {                                                     \
            rc = vrc;                                                   \
            fprintf(stderr, "PREFIX CONFIG ERROR: value[%u] '%s' for %s.%s is INVALID %s\n", \
                    u+1, cfg->sec##_##key[u], #sec, #key, #typ);        \
        } else if( NULL != new_val ) {                                  \
            if( NULL != cfg->sec##_##key[u] ) free(cfg->sec##_##key[u]); \
            cfg->sec##_##key[u] = new_val;                              \
            new_val = NULL;                                             \
        }                                                               \
    }

#define PREFIX_CFG_MULTI_CLI(sec, key, typ, desc, vfn, me, opt, use)    \
    for( u=0; u < me; u++ ) {                                           \
        vrc = validate_value(#sec, #key, cfg->sec##_##key[u], #typ, vfn, &new_val); \
        if( vrc ) {                                                     \
            rc = vrc;                                                   \
            fprintf(stderr, "PREFIX CONFIG ERROR: value[%u] '%s' for %s.%s is INVALID %s\n", \
                    u+1, cfg->sec##_##key[u], #sec, #key, #typ);        \
        } else if( NULL != new_val ) {                                  \
            if( NULL != cfg->sec##_##key[u] ) free(cfg->sec##_##key[u]); \
            cfg->sec##_##key[u] = new_val;                              \
            new_val = NULL;                                             \
        }                                                               \
    }

    PREFIX_CONFIGS;
#undef PREFIX_CFG
#undef PREFIX_CFG_CLI
#undef PREFIX_CFG_MULTI
#undef PREFIX_CFG_MULTI_CLI

    return rc;
}

int contains_expression(const char* val)
{
    static char expr_chars[8] = {'(', ')', '+', '-', '*', '/', '%', '^'};
    size_t s;
    char c;
    char* match;
    for( s=0; s < sizeof(expr_chars); s++ ) {
        c = expr_chars[s];
        match = strchr(val, c);
        if( NULL != match ) {
            if( (('+' == c) || ('-' == c)) && (match > val) ) {
                // check for exponent notation
                c = *(--match);
                if( ('e' == c) || ('E' == c) )
                   return 0; // tinyexpr can't handle exponent notation
            }
            return 1;
        }
    }
    return 0;
}

int configurator_bool_val(const char* val,
                          bool* b)
{
    if( (NULL == val) || (NULL == b) )
        return EINVAL;

    if( 1 == strlen(val) ) {
        switch( val[0] ) {
        case '0':
        case 'f':
        case 'n':
        case 'F':
        case 'N':
            *b = false;
            return 0;
        case '1':
        case 't':
        case 'y':
        case 'T':
        case 'Y':
            *b = true;
            return 0;
        default:
            return 1;
        }
    }
    else if( (0 == strcmp(val, "no"))
             || (0 == strcmp(val, "off"))
             || (0 == strcmp(val, "false")) ) {
        *b = false;
        return 0;
    }
    else if( (0 == strcmp(val, "yes"))
             || (0 == strcmp(val, "on"))
             || (0 == strcmp(val, "true")) ) {
        *b = true;
        return 0;
    }
    return EINVAL;
}

int configurator_bool_check(const char* s,
                            const char* k,
                            const char* val,
                            char** o)
{
    bool b;

    if( NULL == val ) // unset is OK
        return 0;

    return configurator_bool_val(val, &b);
}

int configurator_float_val(const char* val,
                           double* d)
{
    int err;
    double check, teval;
    char* end = NULL;

    if( (NULL == val) || (NULL == d) )
        return EINVAL;

    if( contains_expression(val) ) {
        err = 0;
        teval = te_interp(val, &err);
        if( 0 == err )
            check = teval;
        else
            return EINVAL;
    }
    else {
        errno = 0;
        check = strtod(val, &end);
        err = errno;
        if( (ERANGE == err) || (end == val) )
            return EINVAL;
        else if( *end != 0 ) {
            switch( *end ) {
            case 'f':
            case 'l':
            case 'F':
            case 'L':
                break;
            default:
                return EINVAL;
            }
        }
    }

    *d = check;
    return 0;
}

int configurator_float_check(const char* s,
                             const char* k,
                             const char* val,
                             char** o)
{
    int rc;
    size_t len;
    double d;
    char* newval = NULL;

    if( NULL == val ) // unset is OK
        return 0;

    rc = configurator_float_val(val, &d);
    if( (NULL != o) && (0 == rc) && contains_expression(val) ) {
        // update config setting to evaluated value
        len = strlen(val) + 1; // evaluated value should be shorter
        newval = (char*) calloc(len, sizeof(char));
        if( NULL != newval ) {
            snprintf(newval, len, "%.6le", d);
            *o = newval;
        }
    }
    return rc;
}

int configurator_int_val(const char* val,
                         long* l)
{
    long check;
    double teval;
    int err;
    char* end = NULL;

    if( (NULL == val) || (NULL == l) )
        return EINVAL;

    if( contains_expression(val) ) {
        err = 0;
        teval = te_interp(val, &err);
        if( 0 == err )
            check = (long)teval;
        else
            return EINVAL;
    }
    else {
        errno = 0;
        check = strtol(val, &end, 0);
        err = errno;
        if( (ERANGE == err) || (end == val) )
            return EINVAL;
        else if( *end != 0 ) {
            switch( *end ) {
            case 'l':
            case 'u':
            case 'L':
            case 'U':
                break;
            default:
                return EINVAL;
            }
        }
    }

    *l = check;
    return 0;
}

int configurator_int_check(const char* s,
                           const char* k,
                           const char* val,
                           char** o)
{
    int rc;
    size_t len;
    long l;
    char* newval = NULL;

    if( NULL == val ) // unset is OK
        return 0;

    rc = configurator_int_val(val, &l);
    if( (NULL != o) && (0 == rc) && contains_expression(val) ) {
        // update config setting to evaluated value
        len = strlen(val) + 1; // evaluated value should be shorter
        newval = (char*) calloc(len, sizeof(char));
        if( NULL != newval ) {
            snprintf(newval, len, "%ld", l);
            *o = newval;
        }
    }
    return rc;
}
    
int configurator_file_check(const char* s,
                            const char* k,
                            const char* val,
                            char** o)
{
    int rc;
    struct stat st;

    if( NULL == val )
        return 0;

    rc = stat(val, &st);
    if( 0 == rc ) {
        if( st.st_mode & S_IFREG )
            return 0;
        else
            return ENOENT;
    }
    return errno; // invalid
}

int configurator_directory_check(const char* s,
                                 const char* k,
                                 const char* val,
                                 char** o)
{
    int rc;
    struct stat st;

    if( NULL == val )
        return 0;

    rc = stat(val, &st);
    if( 0 == rc ) {
        if( st.st_mode & S_IFDIR )
            return 0;
        else
            return ENOTDIR;
    }
    return errno; // invalid
}
