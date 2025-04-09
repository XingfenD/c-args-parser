/**
 * @file ./inc/scap.h
 * @brief a simple command line argument parser in c
 * @author Fendy (xingfen.star@gmail.com)
 * @version 1.0
 * @date 2025-03-30
 * @copyright Copyright (c) 2025
 *
 * this file contains the interfaces of the command line argument parser to be used.
 * you can change the macros in the file scap.h to fit your needs.
 * the macros are used to limit the number of commands, options, and subcommands.
 */

#ifndef SCAP_ARG_PARSER_H
#define SCAP_ARG_PARSER_H


/* ++++ configs ++++ */

#define MAX_SUBCMD_COUNT 5  /* the max number of a command's subcommands */
#define MAX_CMD_DEPTH 5     /* the max depth of command tree */
#define MAX_CMD_COUNT 15    /* the max number of commands */
#define MAX_OPT_COUNT 10    /* the max number of options in a single command or subcommand */

/* ---- configs ---- */



/* ++++ macros functions defination ++++ */

#define to_container(ptr, type, member) ((type*)((char*)(ptr) - offsetof(type, member)))
#define node2cmd(node_ptr) to_container(node_ptr, SAPCommand, tree_node)

/* ---- macros functions defination ---- */



/* ++++ enum defination ++++ */

typedef enum _FlagType {
    single_arg = 0,     /* the flag(option) receives a single argument */
    multi_arg = 1,      /* the flag(option) receives multiple arguments */
    no_arg = 2          /* the flag(option) doesn't receive any argument */
} FlagType;

/* ---- enum defination ---- */



/* ++++ structs defination ++++ */

typedef struct _Flag {
    const char *flag_name;  /* both the flag name and the long option */
    char shorthand;         /* the short option */
    const char *usage;      /* the usage description of the flag */
    void *value;            /* the default value and parsed value of this flag */
    FlagType type;          /* the flag type in (single_arg, multi_arg, no_arg) */
} Flag;

typedef struct _TreeNode {
    int child_cnt;
    int depth;
    struct _TreeNode *parent;
    struct _TreeNode **children;
} TreeNode;

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

/* ---- structs defination ---- */



typedef int (*CmdExec)(SAPCommand *caller);
typedef int (*CmdExecWithArg)(SAPCommand *caller, int argc, char *argv[]);

extern SAPCommand rootCmd;
// extern int cmd_cnt;

/* ++++ functions of Flags ++++ */

/**
 * @brief initialize a flag (Flag) structure.
 *
 * this function is used to initialize a flag structure, setting the flag's name, shorthand, usage description, default value, and type.
 *
 * @param[in] flag A pointer to the flag structure to be initialized.
 * @param[in] flag_name The name of the flag.
 * @param[in] shorthand The shorthand of the flag.
 * @param[in] usage The usage description of the flag.
 * @param[in] dft_val The default value of the flag.
 */
void init_flag(Flag *flag, const char *flag_name, const char shorthand, const char *usage, void *dft_val);

/**
 * @brief add a flag to the specified command.
 *
 * this function is used to add a flag to the specified SAPCommand structure.
 * if the command already contains the maximum number of flags, the function returns NULL.
 * otherwise, it adds the flag to the command's flag list and returns a pointer to the command.
 *
 * @param cmd a pointer to the SAPCommand structure to which the flag will be added.
 * @param flag a pointer to the flag to be added.
 * @return SAPCommand* if the addition is successful, returns a pointer to the SAPCommand structure;
 *                    if the command already contains the maximum number of flags, returns NULL.
 */
SAPCommand *add_flag(SAPCommand *cmd, Flag *flag);

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

/* ++++ functions of Flags ---- */



/* ++++ functions of TreeNode ++++ */
/* ---- functions of TreeNode ---- */



/* ++++ functions of cmd_exec ++++ */

int void_exec(SAPCommand *caller);
int void_self_parse_exec(SAPCommand *caller, int argc, char *argv[]);

/* ---- functions of cmd_exec ---- */



/* ++++ functions of SAPCommand ++++ */

// void get_cmd_stack(SAPCommand *cmd, SAPCommand *call_stack[MAX_SUBCMD_COUNT]);
SAPCommand *get_parent_cmd(SAPCommand cmd);
// void print_cmd_help(SAPCommand *cmd);

/* ---- functions of SAPCommand ---- */



/* ++++ global frame functions will be called by user ++++ */

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
 * @brief initialize a sapcommand structure
 *
 * @param cmd pointer to the sapcommand structure to initialize
 * @param name name of the command
 * @param short_desc short description of the command
 * @param long_desc long description of the command
 * @param exec function pointer for the command execution
 */
void init_sap_command(SAPCommand *cmd, const char *name, const char *short_desc, const char *long_desc, CmdExec exec);

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

/**
 * @brief add a subcommand to a parent command
 *
 * this function adds a child command to a parent command in the command tree.
 * it ensures that both the parent and child commands are valid and attempts to append the child to the parent's tree node.
 * if the operation fails(exceed the MAX_SUBCMD_COUNT), it returns null; otherwise, it returns the parent command.
 *
 * @param parent pointer to the parent SAPCommand structure
 * @param child pointer to the child SAPCommand structure
 * @return pointer to the parent SAPCommand structure if successful, null otherwise
 */
SAPCommand *add_subcmd(SAPCommand *parent, SAPCommand *child);

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

/**
 * @brief free the memory allocated for the root command and its subcommands
 *
 * this function traverses the command tree starting from the root command and frees the memory
 * allocated for multi-argument flags. it uses two stacks to simulate a queue for traversal.
 * finally, it frees the default flag of the help command and the entire command tree.
 *
 */
void free_root_cmd();

/* ---- global frame functions will be called by user ---- */



#endif /* !SCAP_ARG_PARSER_H */
