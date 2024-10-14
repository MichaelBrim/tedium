# configurator
A unified, preprocessor-based system for providing configuration information 
to an application via configuration files, command-line arguments, and 
environment variables.

When handling configuration options specified via multiple methods,
the order of precedence is: (higher numbers have greater priority)
 1. macro default values
 2. system configuration file
 3. environment variables
 4. command-line arguments, including configuration files passed via CLI

## Dependencies
 * C99 or C++
 * getopt_long()
 * getenv()
 * inih .INI config file parser - <https://github.com/benhoyt/inih>
 * tinyexpr C expression evaluator - <https://github.com/codeplea/tinyexpr>
 * nanojsonc json parser - <https://github.com/open-source-patterns/nanojsonc>

## Getting Started
 1. download inih `ini.[ch]` from GitHub
 2. download tinyexpr `tinyexpr.[ch]` from GitHub
 3. copy `configurator.[ch]` to new files
 4. in both new files, globally replace
   - `PREFIX` with desired prefix in uppercase  (e.g., `sed -e 's/PREFIX/MYPROJECT/g'`)
   - `prefix` with desired prefix in lowercase  (e.g., `sed -e 's/prefix/myproject/g'`)
 5. integrate with your project and enjoy
   - update configuration options (i.e., `PREFIX_CONFIGS`) in `configurator.h`
   - declare a `prefix_cfg_t` variable to hold your configuration
   - call `prefix_config_init()` at beginning of main
   - use `configurator_xxx_val()` to get boolean, integer, or floating point values
   - call `prefix_config_fini()` at end of main
## CMake
CMake can be used to download and install the necessary libraries as well

## Setup

The following macros are used to define configuration options:
  * `PREFIX_CFG( section, key, type, default-value, description)`
  * `PREFIX_CFG_CLI( section, key, type, default-value, description, option-char, usage )` 
  * `PREFIX_CFG_MULTI( section, key, type, description, max-entries )`
  * `PREFIX_CFG_CLI_MULTI( section, key, type, description, max-entries, option-char, usage )`
   
The `_CLI` forms indicate that the config option can be passed via command-line switch.
The `_MULTI` forms indicate that the config option may be given multiple values (`max-entries` times).

In the macros, `type` is one of: `BOOL  |  FLOAT  |  INT  |  STRING`
  - `BOOL` values: `0|1`, `y|n`, `Y|N`, `yes|no`, `true|false`, `on|off` 
  - `FLOAT` values: scalars convertible to C double, or compatible tinyexpr expression
  - `INT` values: scalars convertible to C long, or compatible tinyexpr expression

## Usage
### configurator.h:
```c++
...
#define PREFIX_CONFIGS \
    PREFIX_CFG_CLI(prefix, my_config_option, INT, NULL, "My INT Config Option", int, 'i', "Add an Integer to the configuration") \
...
```

### main.cpp:
```c++
#include <configurator.h>

int main(int argc, char* argv[])
{
    int rc;
    long l;    
    prefix_cfg_t my_config;

    if( argc == 1 ) {
        prefix_config_cli_usage(argv[0]);
        return 1;
    }       

    if( prefix_config_init(&mycfg, argc, argv)) {
        fprintf(stderr, "prefix_config_init() failed - rc=%d (%s)\n", rc, strerror(rc));
        return 1;
    }

    prefix_config_print(&my_config, stdout);
    if( 0 == configurator_int_val(my_config.prefix_my_config_option, &l))
        printf("My Config Option = %ld\n", l);
    prefix_config_fini(&my_config);
    return 0;
}

```

## Configuration files
Configuration files have .ini section-key-value format:
```
  # whole-line comment
  [section]
  key = val ; inline comment
  another_key = "a value with spaces"
```
They may also have .json section-key-value format:
```json
{
  "section": {
    "key": "val",
    "another_key": "a value with spaces"
  }
}
```

### Command Line Interface (CLI)
 * ` --section-key [val]`  (long form)
 * ` -o [val]`             (short form, with CLI `option-char`)
 * NOTE: `val` is optional for `BOOL` values only, when `val` not supplied, equivalent to `on`

### Environment Variables
 * `PREFIX_KEY=val`             (when "section" == "prefix")
 * `PREFIX_SECTION_KEY=val` 
 * `PREFIX_SECTION_KEY_<#>=val` (for `_MULTI` forms, where `<#>` in [1,`max-entries`])
