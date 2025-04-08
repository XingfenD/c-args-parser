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
    single_arg = 0,
    multi_arg = 1,
    no_arg = 2
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

SAPCommand *add_flag(SAPCommand *cmd, Flag *flag);
SAPCommand *add_default_flag(SAPCommand *cmd, Flag *flag);
void set_flag_type(Flag *flag, FlagType type);
Flag *get_flag(SAPCommand *cmd, const char *flag_name);
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

void init_root_cmd(const char *name, const char *short_desc, const char *long_desc, CmdExec exec);
void set_cmd_self_parse(SAPCommand *cmd, CmdExecWithArg self_parse_exec);
void init_sap_command(SAPCommand *cmd, const char *name, const char *short_desc, const char *long_desc, CmdExec exec);
SAPCommand *add_subcmd(SAPCommand *parent, SAPCommand *child);
int do_parse_subcmd(int argc, char *argv[]);
void free_root_cmd();

/* ---- global frame functions will be called by user ---- */



#endif /* !SCAP_ARG_PARSER_H */
