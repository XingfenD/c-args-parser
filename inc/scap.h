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


/* configs */
#define MAX_SUBCMD_COUNT 5
#define MAX_ARG_COUNT 10
#define MAX_COMMAND_COUNT 100
#define MAX_COMMAND_NAME_LENGTH 20
#define MAX_COMMAND_DESCRIPTION_LENGTH 100

#define to_container(ptr, type, member) ((type*)((char*)(ptr) - offsetof(type, member)))

typedef struct _TreeNode {
    int child_cnt;
    struct _TreeNode* parent;
    struct _TreeNode** children;
} TreeNode;

typedef struct _SAPCommand {
    const char *name;
    const char *short_desc;
    const char *long_desc;
    int (*func)(int argc, char *argv[]);
    TreeNode tree_node;
} SAPCommand;

extern SAPCommand rootCmd;

/* functions of TreeNode */
// TreeNode* create_tree_node();
// TreeNode* append_child(TreeNode* parent, TreeNode* child);

/* functions of SAPCommand */
void init_root_cmd(const char *name, const char *short_desc, const char *long_desc, int (*func)(int argc, char **argv));
void free_root_cmd();
void init_sap_command(SAPCommand *cmd, const char *name, const char *short_desc, const char *long_desc, int (*func)(int argc, char *argv[]));
SAPCommand* add_subcmd(SAPCommand *parent, SAPCommand *child);
void print_help();

#endif /* !SCAP_ARG_PARSER_H */
