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

#define MAX_CMD_COUNT 20    /* TODO: add the realization to handle max count  */
// #define MAX_ARG_COUNT 10

/* ---- configs ---- */

#define to_container(ptr, type, member) ((type*)((char*)(ptr) - offsetof(type, member)))

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
    int (*func)(struct _SAPCommand* caller, int argc, char *argv[]);
    TreeNode tree_node;
} SAPCommand;

typedef int (*CmdExec)(SAPCommand* caller, int argc, char *argv[]);

extern SAPCommand rootCmd;
// extern int cmd_cnt;

/* ++++ functions of TreeNode ++++ */

/* ---- functions of TreeNode ---- */

/* ++++ functions of cmd_exec ++++ */

int void_exec(SAPCommand* caller, int argc, char *argv[]);
int help_exec(SAPCommand* caller, int argc, char *argv[]);

/* ---- functions of cmd_exec ---- */

/* ++++ functions of SAPCommand ++++ */

void init_root_cmd(const char *name, const char *short_desc, const char *long_desc, CmdExec func);
void free_root_cmd();
void init_sap_command(SAPCommand *cmd, const char *name, const char *short_desc, const char *long_desc, CmdExec func);
SAPCommand* add_subcmd(SAPCommand *parent, SAPCommand *child);
SAPCommand* get_parent_cmd(SAPCommand cmd);

int call_subcmd(int argc, char *argv[]);

/* ---- functions of SAPCommand ---- */

#endif /* !SCAP_ARG_PARSER_H */
