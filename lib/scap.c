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
#include <string.h>

#include <scap.h>


SAPCommand rootCmd; /* the global root command */
static SAPCommand helpCmd;
// int cmd_cnt = 0; /* the number of commands */

/* ++++ functions of TreeNode ++++ */
static void init_tree_node(TreeNode *node) {
    if (node == NULL) {
        return;
    }

    node->child_cnt = 0;
    node->depth = 0;
    node->parent = NULL;

    /* NOTE: free node.children */
    node->children = (TreeNode **) calloc(sizeof(TreeNode *), MAX_SUBCMD_COUNT);
}

static int get_max_depth(TreeNode *node) {
    int max_depth = 0;
    if (node == NULL) { /* this branch won't be excuted */
        return max_depth;
    }

    /* get the max depth of all children */
    for (int i = 0; i < node->child_cnt; i++) {
        int child_depth = get_max_depth(node->children[i]);
        if (child_depth > max_depth) {
            max_depth = child_depth;
        }
    }
    return max_depth + 1;
}

// static void recursive_adjust_depth(int depth2add, TreeNode *node) {
//     if (node == NULL) {
//         return;
//     }

//     node->depth += depth2add;
//     for (int i = 0; i < node->child_cnt; i++) {
//         recursive_adjust_depth(depth2add, node->children[i]);
//     }
// }

static void non_recursive_adjust_depth(int depth2add, TreeNode *node) {
    if (node == NULL) return;

    /* use stack instead of recursion */
    TreeNode *stack[MAX_CMD_DEPTH];  /* the maximum depth of the tree is MAX_CMD_DEPTH */
    int top = -1;

    stack[++top] = node;  /* push the root node */
    while (top >= 0) {
        TreeNode *current = stack[top--];
        current->depth += depth2add + 1;

        for (int i = 0; i < current->child_cnt; i++) {
            stack[++top] = current->children[i];    /* push the children nodes */
        }
    }
}


static int _adjust_depth(int depth2add, TreeNode *subtree_root) {
    if (subtree_root == NULL) {
        return 0;
    }
    if (get_max_depth(subtree_root) + depth2add > MAX_CMD_DEPTH) {
        return -1;
    }

    /* adjust all the depth or not */
    non_recursive_adjust_depth(depth2add, subtree_root);
    return 1;
}

static int adjust_depth(TreeNode *parent, TreeNode *subtree_root) {
    return _adjust_depth(parent->depth, subtree_root);
}

static TreeNode* append_child(TreeNode *parent, TreeNode *child) {
    if (parent == NULL ||
        child == NULL ||
        parent->child_cnt >= MAX_SUBCMD_COUNT
    ) {
        return NULL;
    }
    assert(parent->children != NULL);
    if (adjust_depth(parent, child) != 1) {
        return NULL;
    }
    child->parent = parent;
    parent->children[parent->child_cnt++] = child;

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

/* ---- functions of TreeNode ---- */



/* ++++ functions of cmd_exec ++++ */

int void_exec(SAPCommand* caller, int argc, char *argv[]) {
    /* TODO: print the message that the command haven't been allocate function */
    assert(caller != NULL);

    printf("The command %s haven't been allocate function\n", caller->name);
    return 0;
}

int help_exec(SAPCommand* caller, int argc, char *argv[]) {
    /* TODO: parse the argv to provide detailed help for subfunctions */
    assert(caller != NULL);
    assert(strcmp("help", caller->name) == 0);

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

    return 0;
}

int call_exec(SAPCommand* caller, int argc, char *argv[]) {
    assert(caller!= NULL);
    printf("%s\n", argv[0]);
    return caller->func(caller, argc, argv);
}

/* ---- functions of cmd_exec ---- */



/* ++++ functions of SAPCommand ++++ */

// void get_call_stack(SAPCommand *cmd) {
//     /* finish this function after realize depth feature*/
//     assert(cmd!= NULL);
//     char *stack[MAX_SUBCMD_COUNT];
// }

static void add_help_cmd() {
    init_sap_command(&helpCmd, "help", "Display this help message", NULL, help_exec);
    add_subcmd(&rootCmd, &helpCmd);
}

SAPCommand* get_parent_cmd(SAPCommand cmd) {
    if (cmd.tree_node.parent == NULL) {
        return NULL;
    }

    return to_container(cmd.tree_node.parent, SAPCommand, tree_node);
}

void init_root_cmd(const char *name, const char *short_desc, const char *long_desc, CmdExec func) {
    init_sap_command(&rootCmd, name, short_desc, long_desc, func);
}

void free_root_cmd() {
    free_node_tree(&rootCmd.tree_node);
}

void init_sap_command(SAPCommand *cmd, const char *name, const char *short_desc, const char *long_desc, CmdExec func) {
    assert(cmd!= NULL);
    cmd->name = name;
    cmd->short_desc = short_desc;
    cmd->long_desc = long_desc;
    cmd->func = (func == NULL)? void_exec: func;
    init_tree_node(&cmd->tree_node);
}

SAPCommand* add_subcmd(SAPCommand *parent, SAPCommand *child) {
    assert(parent!= NULL);
    assert(child!= NULL);

    if (append_child(&parent->tree_node, &child->tree_node) == NULL) {
        return NULL;
    }

    return parent;
}

int call_subcmd(int argc, char *argv[]) {

    add_help_cmd(); /* add the subcommand help to root command */

    #define not_stack_select ((stack_select == 0)? 1: 0)
    TreeNode *stack[MAX_CMD_COUNT][2];  /* two stack cosplay a queue */
    int top[2] = {-1, -1};                  /* the top ptr of the two stack */
    int stack_select = 0;                   /* select the available stack*/
    int depth = 0;

    assert(&rootCmd.tree_node != NULL);

    /* push the root into stack */
    stack[++top[stack_select]][stack_select] = &(rootCmd.tree_node);

    while (top[stack_select] >= 0) {
        TreeNode *stack_top = stack[top[stack_select]][stack_select];
        SAPCommand *crt_cmd = to_container(stack_top, SAPCommand, tree_node);   /* current command */

        if (depth >= argc || argv[depth][0] == '-') {
            /* if the depth is out of range or the argv[depth] is an option */
            /**
             * that is:
             * 1. the command have subcmd but is not provided
             * 2. the command have subcmd but an option is provided instead of subcmd
             */
            SAPCommand *parent_cmd = get_parent_cmd(*crt_cmd);
            /* exec the parent cmd, consequently depth decrease */
            return call_exec(parent_cmd, argc - (depth - 1), argv + (depth - 1));
        }

        if (strcmp(crt_cmd->name, argv[depth]) != 0 &&
            !(strcmp(crt_cmd->name, rootCmd.name) == 0)) {
            /* if current cmd isn't our target cmd or root cmd */
            if (--top[stack_select] < 0) {
                /* pop and judge whether the stack is empty */
                stack_select = not_stack_select; /* switch the stack */
                depth++;
            }   /* top[stack_select] == -1 */
            continue;
        }   /* if current cmd isn't our target cmd */

        /* ++++ if current cmd is our target cmd ++++ */

        if (stack_top->child_cnt == 0) {
            /* leaf node */
            return call_exec(crt_cmd, argc - depth, argv + depth);
        }   /* leaf node */

        /* ++++ non-leaf node ++++ */

        for (int i = 0; i < stack_top->child_cnt; i++) {
            /* push the subcmds into the other stack */
            stack[++top[not_stack_select]][not_stack_select] = stack_top->children[i];
        }

        top[stack_select] = -1;
        stack_select = not_stack_select;
        depth++;

        /* ---- non-leaf node ---- */
        /* ---- if current cmd is our target cmd ---- */
    #undef not_stack_select
    }

    /* exit of unknown cmd */
    printf("Unknown command: %s\n", argv[depth - 1]);
    return -1;
}

/* ---- functions of SAPCommand ---- */
