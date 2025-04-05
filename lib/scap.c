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


SAPCommand rootCmd;         /* the global root command */
static SAPCommand helpCmd;  /* the help command */
static int cmd_cnt = 0;     /* the number of commands */

/* ++++ functions of Flags ++++ */

void init_flag(Flag *flag, const char *flag_name, const char shorthand, const char *usage, const char *dft_val) {
    assert(flag != NULL);
    flag->flag_name = flag_name;
    flag->shorthand = shorthand;
    flag->usage = usage;
    flag->value = dft_val;
}

SAPCommand *add_flag(SAPCommand *cmd, Flag *flag) {
    assert(cmd!= NULL);
    assert(flag!= NULL);
    if (cmd->flag_cnt >= MAX_ARG_COUNT) {
        return NULL;
    }
    cmd->flags[cmd->flag_cnt++] = flag;
    return cmd;
}

SAPCommand *add_default_flag(SAPCommand *cmd, Flag *flag) {
    assert(cmd!= NULL);
    assert(flag!= NULL);
    cmd->default_flag = flag;
    return cmd;
}

/* ---- functions of Flags ----*/

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



/* ++++ functions of SAPCommand ++++ */

SAPCommand* get_parent_cmd(SAPCommand cmd) {
    if (cmd.tree_node.parent == NULL) {
        return NULL;
    }

    return node2cmd(cmd.tree_node.parent);
}

static int get_child_cmd_cnt(SAPCommand *cmd) {
    if (cmd == NULL) {
        return -1;
    }
    return cmd->tree_node.child_cnt;
}

static void get_cmd_stack(SAPCommand *cmd, SAPCommand *call_stack[MAX_SUBCMD_COUNT]) {
    assert(cmd != NULL);
    assert(call_stack != NULL);
    int depth = cmd->tree_node.depth;

    while(depth >= 0) {
        call_stack[depth--] = cmd;
        cmd = get_parent_cmd(*cmd);
        if (cmd == NULL) {
            break;
        }
    }
}

static SAPCommand* find_cmd_by_stack(SAPCommand *cmd, int cmd_cnt, char *cmd_names[], int *out_depth) {
    /* NOTE: the first command in cmd_names is $cmd */
    #define not_stack_select ((stack_select == 0)? 1: 0)
    TreeNode *stack[MAX_CMD_COUNT][2];      /* two stack cosplay a queue */
    int top[2] = {-1, -1};                  /* the top ptr of the two stack */
    int stack_select = 0;                   /* select the available stack*/
    int depth = 0;

    assert(cmd!= NULL);

    /* push the root into stack */
    stack[++top[stack_select]][stack_select] = &(cmd->tree_node);

    while (top[stack_select] >= 0) {
        TreeNode *stack_top = stack[top[stack_select]][stack_select];
        SAPCommand *crt_cmd = node2cmd(stack_top);   /* current command */

        if (depth >= cmd_cnt || cmd_names[depth][0] == '-') {
            /* if the depth is out of range or the argv[depth] is an option */
            /**
             * that is:
             * 1. the command have subcmd but is not provided
             * 2. the command have subcmd but an option is provided instead of subcmd
             */
            SAPCommand *parent_cmd = get_parent_cmd(*crt_cmd);
            /* exec the parent cmd, consequently depth decrease */
            if (out_depth != NULL) {
                *out_depth = depth - 1;
            }
            return parent_cmd;
        }

        if (strcmp(crt_cmd->name, cmd_names[depth]) != 0 &&
            !(strcmp(crt_cmd->name, cmd->name) == 0)) {
            /* if current cmd isn't our target cmd or root cmd */
            if (--top[stack_select] < 0) {
                /* pop and judge whether the stack is empty */
                stack_select = not_stack_select;    /* switch the stack */
                depth++;
            }   /* top[stack_select] == -1 */
            continue;
        }   /* if current cmd isn't our target cmd */

        /* ++++ if current cmd is our target cmd ++++ */

        if (stack_top->child_cnt == 0) {
            /* exit of leaf nodes */
            if (out_depth != NULL) {
                *out_depth = depth;
            }
            return crt_cmd;
        }   /* exit of leaf nodes */

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

    /* exit of unknown cmds */
    if (out_depth!= NULL) {
        *out_depth = depth - 1;
    }
    return NULL;
}

static void print_cmd_help(SAPCommand *cmd) {
    SAPCommand *call_stack[MAX_CMD_DEPTH];
    int p_stack = 0;

    assert(cmd != NULL);

    printf("Description: %s\n\n", cmd->short_desc);
    if (cmd->long_desc!= NULL) {
        printf("%s\n\n", cmd->long_desc);
    }

    /* TODO: modify the Usage print */
    printf("Usage: %s [options]\n\n", cmd->name);

    /* print the available subcmds */
    if (cmd->tree_node.child_cnt != 0) {
        printf("Available Commands:\n");
        for (int i = 0; i < cmd->tree_node.child_cnt; i++) {
            SAPCommand *sub_cmd = node2cmd(cmd->tree_node.children[i]);
            printf("  %s\t%s\n", sub_cmd->name, sub_cmd->short_desc);
        }
        printf("\n");
    }

    /* print the available flags */
    if (cmd->flag_cnt != 0) {
        printf("Flags:\n");
        for (int i = 0; i < cmd->flag_cnt; i++) {
            printf("  ");
            if (cmd->flags[i]->shorthand != '\0') {
                printf("-%c, ", cmd->flags[i]->shorthand);
            }
            printf("--%s\t%s\n", cmd->flags[i]->flag_name, cmd->flags[i]->usage);
            if (cmd->flags[i]->value != NULL) {
                printf("    Default: %s\n", cmd->flags[i]->value);
            }
        }
        printf("\n");
    }

    if (get_child_cmd_cnt(cmd) > 0) {

        printf("Use \"");
        get_cmd_stack(cmd, call_stack);
        while (call_stack[p_stack] != cmd) {
            printf("%s ", call_stack[p_stack]->name);
            p_stack++;
        }
        printf("%s [command] --help\" for more help\n", cmd->name);
    }
}

/* ---- functions of SAPCommand ---- */



/* ++++ functions of cmd_exec ++++ */

int void_exec(SAPCommand* caller, int argc, char *argv[]) {
    assert(caller != NULL);

    printf("The command %s haven't been allocate function\n", caller->name);
    return 1;
}

static int help_exec(SAPCommand* caller, int argc, char *argv[]) {
    /* TODO: parse the argv to provide detailed help for subfunctions */
    /* verify the caller */
    assert(caller != NULL);
    assert(strcmp("help", caller->name) == 0);
    SAPCommand *cmd2get_help = find_cmd_by_stack(&rootCmd, argc, argv, NULL);
    if (cmd2get_help == NULL) {
        printf("Unknown command: %s. See '%s help'.\n", argv[0], rootCmd.name);
        return -1;
    }
    // printf("cmd2get_help in help_exec: %s\n", cmd2get_help->name);
    print_cmd_help(cmd2get_help);

    // if (rootCmd.short_desc != NULL) {
    //     printf("%s\n", rootCmd.short_desc);
    // }
    // if (rootCmd.long_desc!= NULL) {
    //     printf("%s\n", rootCmd.long_desc);
    // }

    return 0;
}

static int call_exec(SAPCommand* caller, int argc, char *argv[]) {
    assert(caller!= NULL);
    // printf("call_exec: %s\n", argv[0]);
    return caller->func(caller, argc, argv);
}

/* ---- functions of cmd_exec ---- */



static void add_helpcmd() {
    init_sap_command(&helpCmd, "help", "Display this help message", NULL, help_exec);
    add_subcmd(&rootCmd, &helpCmd);
}



/* ++++ global frame functions that will be called by user ++++ */

void init_sap_command(SAPCommand *cmd, const char *name, const char *short_desc, const char *long_desc, CmdExec func) {
    assert(cmd!= NULL);
    assert(++cmd_cnt <= MAX_CMD_COUNT);

    cmd->name = name;
    cmd->short_desc = short_desc;
    cmd->long_desc = long_desc;
    cmd->flag_cnt = 0;
    cmd->func = (func == NULL)? void_exec: func;

    init_tree_node(&cmd->tree_node);
}

void init_root_cmd(const char *name, const char *short_desc, const char *long_desc, CmdExec func) {
    init_sap_command(&rootCmd, name, short_desc, long_desc, func);
}

SAPCommand* add_subcmd(SAPCommand *parent, SAPCommand *child) {
    assert(parent!= NULL);
    assert(child!= NULL);

    if (append_child(&parent->tree_node, &child->tree_node) == NULL) {
        return NULL;
    }

    return parent;
}

int do_parse_subcmd(int argc, char *argv[]) {
    SAPCommand *cmd2run = NULL;
    int depth = 0;


    add_helpcmd(); /* add the subcommand help to root command */

    cmd2run = find_cmd_by_stack(&rootCmd, argc, argv, &depth);
    // printf("cmd2run in do_parse_subcmd: %s\n", cmd2run->name);
    if (cmd2run != NULL) {
        return call_exec(cmd2run, argc - depth, argv + depth);
    } else { /* exit of unknown cmds */
        printf("Unknown command: %s. See '%s help'.\n", argv[depth], rootCmd.name);
        return -1;
    }
}

void free_root_cmd() {
    free_node_tree(&rootCmd.tree_node);
}

/* ---- global frame functions that will be called by user ---- */
