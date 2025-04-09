# SCAP/SCPPAP - Simple C/C++ Agument Parser

​	SCAP/SCPPAP is an command-line argument parsing framework in C and C++ (C++ version is waiting for implementation).

## Project Status

### scap.h&scap.c - basic finished

​	Former test in actual project is needed.

Todo:

- [ ] Parse multiple no_arg flags in a single option. Eg. -rf
- [x] Permit that the flag don't have an shorthand.
- [x] Check the run result of default no_arg flags.
- [ ] Merge the `add_flag` function family together, and add a mode argument.
- [ ] Add dependency check for flags


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

The prototype:

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

​	About field `parse_by_self` and `exec_self_parse`, take a look at [`set_cmd_self_parse`](#`set_cmd_self_parse` Function) section.

#### `init_sap_cmd` and `init_root_cmd` Function

The prototype:

```c
/**
 * @brief initialize the root sapcommand
 *
 * @param name name of the root command
 * @param short_desc short description of the root command
 * @param long_desc long description of the root command
 * @param exec function pointer for the root command execution
 *
 * this function will initialize the help flag and then call init_sap_command to initialize the root command
 * consequently, call this function before calling any init_sap_command functions
 */
void init_root_cmd(const char *name, const char *short_desc, const char *long_desc, CmdExec exec);

/**
 * @brief initialize the root sapcommand
 *
 * @param name name of the root command
 * @param short_desc short description of the root command
 * @param long_desc long description of the root command
 * @param exec function pointer for the root command execution
 
 */
void init_root_cmd(const char *name, const char *short_desc, const char *long_desc, CmdExec exec);
```

​	The field `long_desc` can be NULL, while the others can't. If `exec` is provided to be NULL, then will be set to void_exec. `init_root_cmd` will call the `init_sap_cmd` to initialize the pre-defined variable `helpFlag` and `rootCmd`.

#### `set_cmd_self_parse` Function

The prototype:

```c
/**
 * @brief set the self-parse execution function for a command
 *
 * this function sets whether a command should parse command-line arguments by itself and specifies the corresponding self-parse execution function.
 * if the provided self-parse execution function is null, it uses the default void self-parse execution function.
 *
 * @param cmd pointer to the SAPCommand structure representing the command to be set
 * @param self_parse_exec pointer to the self-parse execution function which takes a SAPCommand pointer and command-line arguments
 */
void set_cmd_self_parse(SAPCommand *cmd, CmdExecWithArg self_parse_exec);
```

​	Set a `SAPCommand` to parse command-line arguments by itself and specify the corresponding self-parse execution function. If a command is set to parse the arguments it self, the SCAP framework will **not check the input-validty** for this single command. Also SCAP framework will **not print help information** when `--help` or `-h` is given.

#### `add_subcmd` Function

The prototype:

```c
/**
 * @brief add a subcommand to a parent command
 *
 * this function adds a child command to a parent command in the command tree.
 * it ensures that both the parent and child commands are valid and attempts to append the child to the parent's tree node.
 * if the operation fails(exceed the max_subcmd_cnt), it returns null; otherwise, it returns the parent command.
 *
 * @param parent pointer to the parent SAPCommand structure
 * @param child pointer to the child SAPCommand structure
 * @return pointer to the parent SAPCommand structure if successful, null otherwise
 */
SAPCommand *add_subcmd(SAPCommand *parent, SAPCommand *child);
```

​	Just as the documents goes.

#### `do_parse_subcmd` Function

The prototype

```
/**
 * @brief parse and execute subcommands based on command-line arguments
 *
 * this function parses the command-line arguments to find the appropriate subcommand to execute.
 * it adds a help flag to the help command and then searches for the target subcommand.
 * if the subcommand is found, it executes the subcommand; otherwise, it prints an error message.
 *
 * @param argc the number of command-line arguments
 * @param argv the array of command-line arguments
 * @return the return value of the executed subcommand or -1 if the command is unknown
 */
 int do_parse_subcmd(int argc, char *argv[]);
```

​	The core function of SCAP, init help command and some flag, parse the command-line arguments to get the subcmd to run, and then call the cmd_exec to run the command execution.

#### `free_root_cmd` Function

The prototype

```c
/**
 * @brief free the memory allocated for the root command and its subcommands
 *
 * this function traverses the command tree starting from the root command and frees the memory
 * allocated for multi-argument flags. it uses two stacks to simulate a queue for traversal.
 * finally, it frees the default flag of the help command and the entire command tree.
 *
 */
void free_root_cmd();
```

​	free the memory used by the framework, this function should be called before the program exit.

#### `Flag` Structure

The prototype

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

#### `init_flag` Function

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

​	`init_flag` is used to initialize a flag structure. the `flag` shouldn't be NULL. What's more, `*flag` shouldn't be a local variable, that is `*flag` should have an action scope wider than the main function.

#### `add_flag` Function

The prototype:

```c
/**
 * @brief Add a flag to the specified command.
 *
 * This function is used to add a flag to the specified SAPCommand structure.
 * If the command already contains the maximum number of flags, the function returns NULL.
 * Otherwise, it adds the flag to the command's flag list and returns a pointer to the command.
 *
 * @param cmd A pointer to the SAPCommand structure to which the flag will be added.
 * @param flag A pointer to the flag to be added.
 * @return SAPCommand* If the addition is successful, returns a pointer to the SAPCommand structure;
 *                    if the command already contains the maximum number of flags, returns NULL.
 */
SAPCommand *add_flag(SAPCommand *cmd, Flag *flag);
```
​	This function is used to add a flag to the specified SAPCommand structure. If the command already contains the maximum number of flags, the function returns NULL. Otherwise, it adds the flag to the command's flag list and returns a pointer to the command. The arguments cmd and flag can't be NULL, and have a wide action scope.

#### `add_default_flag` Function

The prototype:

```c
/**
 * @brief add a default flag to the specified command.
 *
 * this function is used to add a default flag to the specified SAPCommand structure.
 * if the command already contains the maximum number of flags, the function returns NULL.
 * otherwise, it adds the flag to the command's flag list and set the field $default_flag, then a pointer to the command.
 *
 * @param cmd a pointer to the SAPCommand structure to which the flag will be added.
 * @param flag a pointer to the flag to be added.
 * @return SAPCommand* if the addition is successful, returns a pointer to the SAPCommand structure;
 *                    if the command already contains the maximum number of flags, returns NULL.
 */
SAPCommand *add_default_flag(SAPCommand *cmd, Flag *flag);
```

​	This function is used to add a default flag to the specified command. A**default flag** is the flag that receives the arguments which are not allocated to other commands. For example a command `ex` has two flags: `-m` and `-d`, `-d` is the default `single_arg` flag, and `-m` is `single_arg`, too. In command `ex dddd -m mmmm`, argument `dddd` will be allocated to `-d` flag.

​	If two default flags are added to the same `SAPCommand`, the latter one will over-write the former one.

​	If a no_arg flag is added as a default flag, it will be regarded to be provided defaultly, although it seems to be meaningless.

#### `add_persist_flag` Function

The prototype:

```c
/**
 * @brief add a persist flag to the specified command and its all progeny
 *
 * @param cmd the root command of the subcommand tree
 * @param flag the persist flag to be added
 * @return int the number of failed addtions, 0 means all succeed
 *
 * @note this command should be called after all subcommands are added to the root command(of the subcommand tree)
 */
int add_persist_flag(SAPCommand *cmd, Flag *flag);
```

​	This function is used to add the flag to all the descendants of the given `cmd`. This command should be called after all subcommands are added to the root command(of the subcommand tree).

#### `set_flag_type` Function

The prototype:

```c
/**
 * @brief set the type of a flag.
 *
 * this function is used to set the type of a specified flag to the given type.
 * if the flag already has a value and the new type is a multi-argument type,
 * it will print a warning message and set the flag's value to NULL.
 *
 * @param flag A pointer to the flag whose type is to be set.
 * @param type The type to set the flag to.
 */
void set_flag_type(Flag *flag, FlagType type);
```

​	This function is used to set the type of a specified flag to the given type. if the flag already has a value and the new type is a multi-argument type, it will print a warning message and set the flag's value to NULL.

​	FlagType is an enum, whose definition is :

```c
typedef enum _FlagType {
    single_arg = 0,     /* the flag(option) receives a single argument */
    multi_arg = 1,      /* the flag(option) receives multiple arguments */
    no_arg = 2          /* the flag(option) doesn't receive any argument */
} FlagType;
```

#### `get_flag`&`get_flag_by_shorthand` Function

The prototype:

```c
/**
 * @brief retrieve a flag by its name from a given command.
 *
 * @param cmd pointer to the SAPCommand structure.
 * @param flag_name the name of the flag to search for.
 * @return Flag* pointer to the matching flag, or NULL if not found.
 */
Flag *get_flag(SAPCommand *cmd, const char *flag_name);

/**
 * @brief retrieve a flag by its shorthand character from a given command.
 *
 * @param cmd pointer to the SAPCommand structure.
 * @param shorthand the shorthand character of the flag to search for.
 * @return Flag* pointer to the matching flag, or NULL if not found.
 */
Flag *get_flag_by_shorthand(SAPCommand *cmd, char shorthand);
```

​	Just as the documents goes.

### Apply SCAP To Your Project

​	Just download the scap.h scap.c or scppap.hpp scppap.cpp, include the head file compile the source file like your own library. The example program is given in [EXAMPLE.c](EXAMPLE.c)

```c
/**
 * @file ./docs/EXAMPLE.c
 * @brief the example of using scap framework
 * @author Fendy (xingfen.star@gmail.com)
 * @version 1.0
 * @date 2025-05-09
 * @copyright Copyright (c) 2025
 */

#include <stdio.h>
#include <scap.h>

int exec_without_args(SAPCommand *caller) {
    printf("Current Command Name: %s\n", caller->name);
    if (caller->default_flag != NULL) {
        printf("The default_flag name: %s\n", caller->default_flag->flag_name);
    }
    for (int i = 0; i < caller->flag_cnt; i++) {
        if (caller->flags[i]->type == single_arg) {
            printf("%s flag[%d]: %s, single_arg, value: %s\n", caller->name, i, caller->flags[i]->flag_name, (const char *) caller->flags[i]->value);
        } else if (caller->flags[i]->type == multi_arg) {
            printf("%s flag[%d]: %s, multi_arg, address: %p, value:\n", caller->name, i, caller->flags[i]->flag_name, caller->flags[i]->value);
            if (caller->flags[i]->value == NULL) {
                continue;
            }
            for (int j = 0; ; j++) {
                if (((char **) caller->flags[i]->value)[j] == NULL) {
                    printf("\n");
                    break;
                }
                printf("%s ", ((char **) caller->flags[i]->value)[j]);
            }
        } else if (caller->flags[i]->type == no_arg) {
            printf("%s flag[%d]: %s, no_arg, isGiven: %d\n", caller->name, i, caller->flags[i]->flag_name, caller->flags[i]->value != NULL);
        }
    }
    return 0;
}

int exec_with_args(SAPCommand *caller, int argc, char *argv[]) {
    printf("Current Command Name: %s\n", caller->name);
    printf("argc: %d\n", argc);
    for (int i = 0; i < argc; i++) {
        printf("argv[%d]: %s\n", i, argv[i]);
    }
    return 0;
}

int main(int argc, char *argv[]) {

    init_root_cmd("example", "This is short description for example", "This is long description for example", exec_without_args);

    Flag root_s;
    init_flag(&root_s, "root_s", 's', "This is long description for root_s", "default_value for root_s");
    add_flag(&rootCmd, &root_s);

    Flag root_m;
    init_flag(&root_m, "root_m", 'm', "This is long description for root_m", NULL);
    set_flag_type(&root_m, multi_arg);
    add_default_flag(&rootCmd, &root_m);

    Flag root_n;
    init_flag(&root_n, "root_n", 'n', "This is long description for root_n", NULL);
    set_flag_type(&root_n, no_arg);
    add_flag(&rootCmd, &root_n);

    SAPCommand sub1;
    init_sap_command(&sub1, "sub1", "This is short description for sub1", "This is long description for sub1", NULL);
    set_cmd_self_parse(&sub1, exec_with_args);
    add_subcmd(&rootCmd, &sub1);

    SAPCommand sub2;
    init_sap_command(&sub2, "sub2", "This is short description for sub2", "This is long description for sub2", NULL);
    add_subcmd(&rootCmd, &sub2);

    Flag persist_flag;
    init_flag(&persist_flag, "persist_flag", 'p', "This is long description for persist_flag", "default_value for persist_flag");
    add_persist_flag(&sub1, &persist_flag);

    int ret = do_parse_subcmd(argc, argv);

    free_root_cmd();

    return ret;
}

```

​	Some examples for input and output is given below:

```cmd
$ ./example_c -s arg_s -n -m arg1 arg2 arg3
Current Command Name: example
The default_flag name: root_n
example flag[0]: help, no_arg, isGiven: 0
example flag[1]: root_s, single_arg, value: arg_s
example flag[2]: root_m, multi_arg, address: 0x12a605da0, value:
arg1 arg2 arg3
example flag[3]: root_n, no_arg, isGiven: 1
example flag[4]: persist_flag, single_arg, value: default_value for persist_flag

$ ./example_c sub1 arg1 arg2 arg3
Current Command Name: sub1
argc: 4
argv[0]: sub1
argv[1]: arg1
argv[2]: arg2
argv[3]: arg3

$ ./example_c --help
Description: This is short description for example

This is long description for example

Usage: example [command] [options]

Available Commands:
  sub1  This is short description for sub1
  sub2  This is short description for sub2
  help  Display this help message

Flags:
  -h, --help    Display the help message
  -s, --root_s  This is long description for root_s
  -m, --root_m  This is long description for root_m
  -n, --root_n  This is long description for root_n     - default flag
  -p, --persist_flag    This is long description for persist_flag

Use "example [command] --help" for more help
```

