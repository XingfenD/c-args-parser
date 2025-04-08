# SCAP/SCPPAP - Simple C/C++ Agument Parser

​	SCAP/SCPPAP is an command-line argument parsing framework in C and C++ (C++ version is waiting for implementation).

## Project Status

### scap.h&scap.c - basic finished

​	Former test in actual project is needed.

Todo:

- [ ] Parse multiple no_arg flags in a single option. Eg. -rf
- [ ] Permit that the flag don't have an shorthand.


### Scppap.hpp&scppap.cpp - unfinished

## Introduction to SCAP

### 1. Overview

​	`scap.h` and `scap.c` realize a simple command-line argument parser. This tool helps developers build command-line applications with support for subcommands, options, and nested structures. It is designed to handle long options (`--option`), short options (`-o`), default values, multi-argument options, and hierarchical subcommands.

### 2. Key Features

- Subcommand Support: Builds a command tree with multi-level subcommands.
- Option Parsing: Supports long options, short options, no-argument options, and multi-argument options.
- Help Information: Automatically generates help messages with a default `help` command.
- Error Handling: Provides detailed error messages for unknown options, too many/too few arguments, etc.

### 3. Interfaces

#### Structures

- `Flag` Structure: Represents command-line options, including name, shorthand, usage, value, and type.
- `SAPCommand` Structure: Encapsulates command information, including name, description, options, and execution functions.

#### Initialization and Configuration

- `init_root_cmd`: Initializes the root command.
- `init_sap_command`: Initializes subcommands.
- `add_subcmd`: Adds subcommands to a parent command.
- `add_flag` and `add_default_flag`: Adds options to a command.
- `set_flag_type`: Sets the type of an option (single argument, multi-argument, or no argument).

#### Other Utils functions

- `get_flag`: Just as the function name goes.
- `get_flag_by_shorthand`: Just as the function name goes.
- `get_cmd_stack`(Commented defaultly): get the commands call stack(all ancestor commands).
- `get_parent_cmd`: get the parent `SAPCommand` of the one provided.
- `print_cmd_help`(Commented defaultly): print the help information of the `SAPCommand` provided.

#### Command Execution Templates

```c
/* int cmd_exec(SAPCommand *caller) */
typedef int (*CmdExec)(SAPCommand *caller);
int void_exec(SAPCommand *caller);

/* int cmd_exec_with_arg(SAPCommand *caller, int argc, char *argv[]) */
typedef int (*CmdExecWithArg)(SAPCommand *caller, int argc, char *argv[]);
int void_self_parse_exec(SAPCommand *caller, int argc, char *argv[]);
```

​	This framework will call the related `CmdExec` or `CmdExecWithArg` bound with the recognized subcommand.

### Detailed Introduction

#### `SAPCommand` Structure

```c
typedef struct _SAPCommand {
    const char *name;           /* the name of the command */
    const char *short_desc;     /* the short description of the command */
    const char *long_desc;      /* the long description of the command */
    int flag_cnt;               /* the number of flags(options) in the command */
    int parse_by_self;          /* whether the cmd_exec parses argc&argv itself (default 0) */
    int (*exec_self_parse)(struct _SAPCommand *caller, int argc, char *argv[]); /* be called when parse_by_self is to set 1 */
    int (*exec)(struct _SAPCommand *caller);    /* be called when parse_by_self is to set 0 */
    Flag *default_flag;         /* the default flag, unassigned arguments will be assigned default_flag's argument */
    Flag *flags[MAX_OPT_COUNT]; /* the flags of this SAPCommand */
    TreeNode tree_node;         /* the tree node of this command, used to manage the command tree */
} SAPCommand;
```

​	All the fields in SAPCommand will be set by given functions, setting(writing) the fields by yourself is not permitted.

​	The below fields are permitted to read:

- name
- flag_cnt
- default_flag
- flags

#### `Flag` Structure

```c
typedef struct _Flag {
    const char *flag_name;  /* both the flag name and the long option */
    char shorthand;         /* the short option */
    const char *usage;      /* the usage description of the flag */
    void *value;            /* the default value and parsed value of this flag */
    FlagType type;          /* the flag type in (single_arg, multi_arg, no_arg) */
} Flag;
```

​	All the fields in Flag will be set by given functions, setting(writing) the fields by yourself is not permitted.

​	The below fields are permitted to read:

- flag_name
- shorthand
- value
- type

​	The current lib will not check the duplicate shorthand, please make sure the same shorthand is not used when programing.

​	The two fields: value and type should be used together. When value == NULL, it refers this flag(option) are not provided in the command-line arguments and don't have a default value.

1. type == single_arg: value is a (char *), pointing to a string.
2. type == multi_arg: value is a (char **), pointing to a string array, which ends with NULL.
3. type == no_arg: calue is a (int *), when this option is provided, it will be not NULL.

#### `init_root_cmd` Function

The prototype:

```c
/**
 * @brief initialize a flag (Flag) structure.
 *
 * this function is used to initialize a flag structure,
 * setting the flag's name, shorthand, usage description, default value, and type.
 *
 * @param[in] flag A pointer to the flag structure to be initialized.
 * @param[in] flag_name The name of the flag.
 * @param[in] shorthand The shorthand of the flag.
 * @param[in] usage The usage description of the flag.
 * @param[in] dft_val The default value of the flag.
 */
void init_flag(Flag *flag, const char *flag_name, const char shorthand, const char *usage, void *dft_val);
```

​	`init_flag` is used to initialize a flag structure. the `flag` shouldn't be NULL. Whatever, `*flag` shouldn't be a local variable, that is `*flag` should have a action scope wider than the main function.

### Apply SCAP To Your Project

​	Just download the scap.h scap.c or scppap.hpp scppap.cpp, include the head file compile the source file like your own library. Eg:

```c
#include <stdio.h>
#include "scap.h"

// Define a simple execution function
int my_exec(SAPCommand* caller) {
    printf("Executing command: %s\n", caller->name);
    return 0;
}

int main(int argc, char *argv[]) {
    // Initialize the root command
    SAPCommand root;
    init_root_cmd("myapp", "My Application", "A simple command-line application", my_exec);

    // Add an option
    Flag versionFlag;
    init_flag(&versionFlag, "version", 'v', "Show version", NULL);
    set_flag_type(&versionFlag, no_arg);
    add_flag(&root, &versionFlag);

    // Add a subcommand
    SAPCommand subcmd;
    init_sap_command(&subcmd, "sub", "Subcommand", "A subcommand example", my_exec);
    add_subcmd(&root, &subcmd);

    // Parse command-line arguments and execute
    return do_parse_subcmd(argc, argv);
}
```

