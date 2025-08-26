# detailed introduction to scap interface

## `SAPCommand` Structure

The prototype:

```c
typedef struct SAPCommand_ {
    const char *name;           /* the name of the command */
    const char *short_desc;     /* the short description of the command */
    const char *long_desc;      /* the long description of the command */
    int flag_cnt;               /* the number of flags(options) in the command */
    int parse_by_self;          /* whether the cmd_exec parses argc&argv itself (default 0) */
    int (*exec_self_parse)(struct SAPCommand_ *caller, int argc, char *argv[]); /* be called when parse_by_self is to set 1 */
    int (*exec)(struct SAPCommand_ *caller);    /* be called when parse_by_self is to set 0 */
    Flag *default_flag;         /* the default flag, unassigned arguments will be assigned default_flag's argument */
    Flag *flags[MAX_OPT_COUNT]; /* the flags of this SAPCommand */
    TreeNode tree_node;         /* the tree node of this command, used to manage the command tree */
} SAPCommand;
```

​	All the fields in SAPCommand should be set by given functions, setting(writing) the fields by yourself is not permitted (All the fields are private).

​	The below fields are permitted to read:

- name
- flag_cnt
- default_flag
- flags

​	About field `parse_by_self` and `exec_self_parse`, take a look at [`set_cmd_self_parse`](#`set_cmd_self_parse` Function) section.

## `init_sap_cmd` and `init_root_cmd` Function

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

## `set_cmd_self_parse` Function

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

## `add_subcmd` Function

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

## `do_parse_subcmd` Function

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

## `free_root_cmd` Function

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

## `Flag` Structure

The prototype

```c
typedef struct {
    const char *flag_name;  /* both the flag name and the long option */
    char shorthand;         /* the short option */
    const char *usage;      /* the usage description of the flag */
    void *value;            /* the default value and parsed value of this flag */
    FlagType type;          /* the flag type in (single_arg, multi_arg, no_arg) */
} Flag;
```

​	All the fields in Flag should be set by given functions, setting(writing) the fields by yourself is not permitted (All the fields are private).

​	The below fields are permitted to read:

- flag_name
- shorthand
- value
- type

​	The current lib will not check the duplicate shorthand, please make sure the same shorthand is not used when programing.

​	The two fields: value and type should be used together. When value == NULL, it refers this flag(option) are not provided in the command-line arguments and don't have a default value.

1. type == single_arg: value is a (char *), pointing to a string.
2. type == multi_arg: value is a (char **), pointing to a string array, **which ends with NULL**.
3. type == no_arg: calue is a (int *), when this option is provided, it will be not NULL.

## `init_flag` Function

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

## `add_flag` Function

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

## `add_default_flag` Function

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

## `add_persist_flag` Function

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

## `set_flag_type` Function

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
typedef enum {
    single_arg = 0,     /* the flag(option) receives a single argument */
    multi_arg = 1,      /* the flag(option) receives multiple arguments */
    no_arg = 2          /* the flag(option) doesn't receive any argument */
} FlagType;
```

## `get_flag`&`get_flag_by_shorthand` Function

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
