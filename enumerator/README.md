# enumerator
A simple preprocessor-based method for quickly defining enumerated types with
associated C-string names and descriptions, including range checking.

## Getting Started
 1. copy enumerator.[ch] to new files
 2. in both new files, globally replace
   - `@@PREFIX@@` with desired prefix in uppercase
   - `@@prefix@@` with desired prefix in lowercase
 3. integrate with your project and enjoy
   - the typename for the enumerated type is `@@prefixe@@_e`
   - add new `ENUMITEM()` definitions to `@@PREFIX@@_ENUMERATOR` in enumerator.h

## Usage
The `ENUMITEM( name, description )` macro defines a new enumerated value with the 
given name and description. The enum constant is named `@@PREFIX@@_ENUM_<NAME>`.

See enumerator.h for example `ENUMITEM()` definitions.

## API
 * `const char* @@prefix@@_enum_str( @@prefix@@_e e )`
   - returns a C-string name for the given enum.
 * `const char* @@prefix@@_enum_description( @@prefix@@_e e )`
   - returns a C-string description for the given enum.
 * `int check_valid_@@prefix@@_enum( @@prefix@@_e )`
   - checks if given enum value is valid
