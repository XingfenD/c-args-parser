/**
 * @file ./inc/scap.h
 * @brief 
 * @author Fendy (xingfen.star@gmail.com)
 * @version 1.0
 * @date 2025-03-30
 * @copyright Copyright (c) 2025
 */

#ifndef SCAP_ARG_PARSER_H
#define SCAP_ARG_PARSER_H


/* ++++ configs ++++ */

#define MAX_SUBCMD_COUNT 5
#define MAX_CMD_DEPTH 5
#define MAX_CMD_COUNT 15
#define MAX_ARG_COUNT 10

/* ---- configs ---- */



/* ++++ macros defination ++++ */

#define to_container(ptr, type, member) ((type*)((char*)(ptr) - offsetof(type, member)))
#define node2cmd(node_ptr) to_container(node_ptr, SAPCommand, tree_node)

/* ---- macros defination ---- */



/* ++++ structs defination ++++ */

typedef struct _Flag {
    const char *flag_name;
    char shorthand;
    const char *usage;
    const char *value;
} Flag;

typedef struct _TreeNode {
    int child_cnt;
    int depth;
    struct _TreeNode* parent;
    struct _TreeNode** children;
} TreeNode;

typedef struct _SAPCommand {
    const char *name;
    const char *short_desc;
    const char *long_desc;
    int flag_cnt;
    int (*func)(struct _SAPCommand* caller, int argc, char *argv[]);
    Flag *default_flag;
    Flag *flags[MAX_ARG_COUNT];
    TreeNode tree_node;
} SAPCommand;

/* ---- structs defination ---- */



typedef int (*CmdExec)(SAPCommand* caller, int argc, char *argv[]);

extern SAPCommand rootCmd;
// extern int cmd_cnt;

/* ++++ functions of Flags ++++ */

void init_flag(Flag *flag, const char *flag_name, const char shorthand, const char *usage, const char *dft_val);
SAPCommand *add_flag(SAPCommand* cmd, Flag *flag);
SAPCommand *add_default_flag(SAPCommand* cmd, Flag *flag);
void get_flag_value(SAPCommand* caller, const char *flag_name, char *value);
void get_flag_value_by_shorthand(SAPCommand* caller, char shorthand, char *value);

/* ++++ functions of Flags ---- */



/* ++++ functions of TreeNode ++++ */
/* ---- functions of TreeNode ---- */



/* ++++ functions of cmd_exec ++++ */

int void_exec(SAPCommand* caller, int argc, char *argv[]);
// int help_exec(SAPCommand* caller, int argc, char *argv[]);

/* ---- functions of cmd_exec ---- */



/* ++++ functions of SAPCommand ++++ */

// void get_cmd_stack(SAPCommand *cmd, SAPCommand *call_stack[MAX_SUBCMD_COUNT]);
SAPCommand* get_parent_cmd(SAPCommand cmd);
// void print_cmd_help(SAPCommand *cmd);

/* ---- functions of SAPCommand ---- */



/* ++++ global frame functions will be called by user ++++ */

void init_root_cmd(const char *name, const char *short_desc, const char *long_desc, CmdExec func);
void init_sap_command(SAPCommand *cmd, const char *name, const char *short_desc, const char *long_desc, CmdExec func);
SAPCommand* add_subcmd(SAPCommand *parent, SAPCommand *child);
int do_parse_subcmd(int argc, char *argv[]);
int do_parse_flags(SAPCommand *cmd, int argc, char *argv[]);
void free_root_cmd();

/* ---- global frame functions will be called by user ---- */



#endif /* !SCAP_ARG_PARSER_H */
