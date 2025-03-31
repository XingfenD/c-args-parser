/**
 * @file ./lib/scap.c
 * @brief 
 * @author Fendy (xingfen.star@gmail.com)
 * @version 1.0
 * @date 2025-03-30
 * @copyright Copyright (c) 2025
 */

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include <scap.h>


SAPCommand rootCmd;

void init_tree_node(TreeNode *node) {
    if (node == NULL) {
        return;
    }

    node->child_cnt = 0;
    node->parent = NULL;

    /* NOTE: free node.children */
    node->children = (TreeNode **) calloc(sizeof(TreeNode *), MAX_SUBCMD_COUNT);
}

static TreeNode* append_child(TreeNode* parent, TreeNode* child) {
    if (parent == NULL ||
        child == NULL ||
        parent->child_cnt >= MAX_SUBCMD_COUNT) {
        return NULL;
    }

    assert(parent->children != NULL);

    if (parent->child_cnt < MAX_SUBCMD_COUNT) {
        child->parent = parent;
        parent->children[parent->child_cnt++] = child;
    }
    return parent;
}

static void free_node_tree(TreeNode *root) {
    if (root == NULL) {
        return;
    }

    for (int i = 0; i < root->child_cnt; i++) {
        free_node_tree(root->children[i]);
    }

    free(root->children);
}

/* functions of SAPCommand */

// SAPCommand* get_parent_cmd(SAPCommand *cmd) {
//     if (cmd == NULL) {
//         return NULL;
//     }

//     assert(cmd->tree_node!= NULL);

//     if (cmd->tree_node->parent == NULL) {
//         return NULL;
//     }

//     return to_container(cmd->tree_node->parent, SAPCommand, tree_node);
// }

void init_root_cmd(const char *name, const char *short_desc, const char *long_desc, int (*func)(int argc, char **argv)) {
    init_sap_command(&rootCmd, name, short_desc, long_desc, func);
}

void free_root_cmd() {
    free_node_tree(&rootCmd.tree_node);
}

void init_sap_command(SAPCommand *cmd, const char *name, const char *short_desc, const char *long_desc, int (*func)(int argc, char *argv[])) {
    assert(cmd!= NULL);
    cmd->name = name;
    cmd->short_desc = short_desc;
    cmd->long_desc = long_desc;
    cmd->func = func;
    init_tree_node(&cmd->tree_node);
}

SAPCommand* add_subcmd(SAPCommand *parent, SAPCommand *child) {
    assert(parent!= NULL);
    assert(child!= NULL);

    append_child(&parent->tree_node, &child->tree_node);

    return parent;
}

void print_help() {
    if (rootCmd.short_desc != NULL) {
        printf("%s\n", rootCmd.short_desc);
    }
    if (rootCmd.long_desc!= NULL) {
        printf("%s\n", rootCmd.long_desc);
    }

    printf("Usage: %s <command> [options]\n", rootCmd.name);
    printf("Commands:\n");
    printf("  help\t\tDisplay this help message\n");
    for (int i = 0; i < rootCmd.tree_node.child_cnt; i++) {
        SAPCommand *cmd = to_container(rootCmd.tree_node.children[i], SAPCommand, tree_node);
        printf("  %s\t\t%s\n", cmd->name, cmd->short_desc);
    }
}

// void print_cmd_help(SAPCommand *cmd) {
//     char **cmd_name_stack = (char **)malloc(sizeof(char *) * MAX_SUBCMD_COUNT);
// }
