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


SAPCommand rootCmd;             /* the global root command */
static SAPCommand helpCmd;      /* the help command */
static int cmd_cnt = 0;         /* the number of commands */
static Flag helpFlag;           /* the help flag */
const static int IS_PROVIDED = 1;   /* the flag is provided */

/* ++++ functions of Flags ++++ */

void init_flag(Flag *flag, const char *flag_name, const char shorthand, const char *usage, void *dft_val) {
    assert(flag != NULL);
    flag->flag_name = flag_name;
    flag->shorthand = shorthand;
    flag->usage = usage;
    flag->value = dft_val;
    flag->type = single_arg;
}

void set_flag_type(Flag *flag, FlagType type) {
    assert(flag != NULL);
    flag->type = type;
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
    if (add_flag(cmd, flag) == NULL) {
        return NULL;
    }
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

static SAPCommand *find_sap_consider_flags(SAPCommand *cmd, int cmd_cnt, char *cmd_names[], int *out_depth) {
    /* NOTE: the first command in cmd_names may not be $cmd */
    #define not_stack_select ((stack_select == 0)? 1: 0)
    TreeNode *stack[MAX_CMD_COUNT][2];      /* two stack cosplay a queue */
    TreeNode *stack_top = NULL;
    SAPCommand *crt_cmd = NULL;
    int top[2] = {-1, -1};                  /* the top ptr of the two stack */
    int stack_select = 0;                   /* select the available stack*/
    int depth = 0;

    assert(cmd!= NULL);

    /* push the root into stack */
    stack[++top[stack_select]][stack_select] = &(cmd->tree_node);

    while (top[stack_select] >= 0) {
        stack_top = stack[top[stack_select]][stack_select];
        crt_cmd = node2cmd(stack_top);   /* current command */

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
            strcmp(crt_cmd->name, cmd->name) != 0) {
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

    /* ++++ exit of unknown cmds ++++ */
    if (out_depth != NULL) {
        *out_depth = depth - 1;
    }

    if (get_parent_cmd(*crt_cmd)->default_flag != NULL) {
        *out_depth -= 1;
        return get_parent_cmd(*crt_cmd);
    }

    return NULL;
}

static SAPCommand *find_sap(SAPCommand *cmd, int cmd_cnt, char *cmd_names[], int *out_depth) {
    /* NOTE: the first command in cmd_names may not be $cmd */
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
            strcmp(crt_cmd->name, cmd->name) != 0) {
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

    /* ++++ exit of unknown cmds ++++ */
    if (out_depth != NULL) {
        *out_depth = depth - 1;
    }
    return NULL;
}

static int get_option_type(char *arg) {
    if (
        (arg[0] == '-' && arg[1] == '\0') ||
        (arg[0] == '-' && arg[1] == '-' && arg[2] == '\0')
    ) {
        /* if the arg is an error option */
        return -1;
    } else if (arg[0] == '-' && arg[1] == '-') {    /* if the arg is a long option */
        return 1;
    } else if (arg[0] == '-' && arg[1] != '\0') {   /* if the arg is a short option */
        return 2;
    } else {    /* if the arg is a normal arg */
        return 0;
    }
}

static int parse_flags(SAPCommand *cmd, int argc, char *argv[]) {
    /* TODO: identify the help flag */
    /* TODO: parse the default flag(recognize the arg without option in cmdline) */
    assert(cmd != NULL);
    assert(argc > 0);
    assert(argv != NULL);
    int flag_idxs[MAX_ARG_COUNT];
    int flag_num = 0;

    /* find the options */
    for (int i = 1; i < argc; i++) {
        switch (get_option_type(argv[i])) {
        case -1:    /* if the arg is an error option */
            return i;
        case 1:     /* if the arg is a long option */
        case 2:     /* if the arg is a short option */
            flag_idxs[flag_num++] = i;
        }
    }   /* loop the argv */

    for (int i = 0; i < flag_num; i++) {
        int j = 0;
        for (j = 0; j < cmd->flag_cnt; j++) {
            if (
                (argv[flag_idxs[i]][1] == '-' && strcmp(argv[flag_idxs[i]] + 2, cmd->flags[j]->flag_name)== 0) ||
                (argv[flag_idxs[i]][1] != '-' && argv[flag_idxs[i]][1] == cmd->flags[j]->shorthand)
            ) {
                /* current argv: argv[flag_idxs[i]] */
                /* current flag: cmd->flags[j] */
                if (cmd->flags[j]->type == single_arg) {
                    /* if the flag is not multi-arg */
                    if (argv[flag_idxs[i]][1] == '-') {
                        /* if the arg is a long option */
                        cmd->flags[j]->value = argv[flag_idxs[i] + 1];
                    } else {
                        /* if the arg is a short option */
                        cmd->flags[j]->value = argv[flag_idxs[i] + 1];
                    }
                    break;
                } else if (cmd->flags[j]->type == no_arg) {
                    /* if the flag is no-arg */
                    cmd->flags[j]->value = (void *) &IS_PROVIDED;
                } else if (cmd->flags[j]->type == multi_arg) {
                    int arg_cnt = 0;
                    char *arg_stack[argc];
                    /* if the flag is multi-arg */
                    for (int k = flag_idxs[i] + 1; k < argc; k++) {
                        if (get_option_type(argv[k]) == 0) {
                            arg_stack[arg_cnt++] = argv[k];
                        } else {
                            break;
                        }
                    }
                    cmd->flags[j]->value = (char **) malloc(sizeof(char *) * (arg_cnt + 1));
                    for (int i = 0; i < arg_cnt; i++) {
                        ((char **) cmd->flags[j]->value)[i] = arg_stack[i];
                    }
                    ((char **) cmd->flags[j]->value)[arg_cnt] = NULL;
                }
                break;
            }
        }   /* loop the flags */
        if (j == cmd->flag_cnt) {
            return flag_idxs[i];
        }
    }   /* loop the flags in cmd line */

    return 0;
}


static void print_cmd_help(SAPCommand *cmd) {
    SAPCommand *call_stack[MAX_CMD_DEPTH];
    int p_stack = 0;

    assert(cmd != NULL);
    get_cmd_stack(cmd, call_stack);

    printf("Description: %s\n\n", cmd->short_desc);
    if (cmd->long_desc!= NULL) {
        printf("%s\n\n", cmd->long_desc);
    }

    /* TODO: modify the Usage print */
    printf("Usage: %s", cmd->name);
    if (cmd->tree_node.child_cnt != 0) {
        printf(" [command]");
    }
    if (cmd->flag_cnt != 0) {
        printf(" [options]");
    }
    printf("\n\n");

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
                printf("    Default: %s\n", (const char *) cmd->flags[i]->value);
            }
        }
        printf("\n");
    }

    if (get_child_cmd_cnt(cmd) > 0) {

        printf("Use \"");
        while (call_stack[p_stack] != cmd) {
            printf("%s ", call_stack[p_stack]->name);
            p_stack++;
        }
        printf("%s [command] --help\" for more help\n", cmd->name);
    }
}

/* ---- functions of SAPCommand ---- */



/* ++++ functions of cmd_exec ++++ */

int void_exec(SAPCommand* caller) {
    assert(caller != NULL);

    printf("The command %s haven't been allocate function\n", caller->name);

    return 1;
}

int void_self_parse_exec(SAPCommand* caller, int argc, char *argv[]) {
    assert(caller != NULL);
    assert(argv != NULL);

    printf("The command %s haven't been allocate function\n", caller->name);
    printf("argc: %d\n", argc);
    for (int i = 0; i < argc; i++) {
        printf("argv[%d]: %s\n", i, argv[i]);
    }

    return 1;
}

static int help_exec(SAPCommand* caller, int argc, char *argv[]) {
    /* verify the caller */
    assert(caller != NULL);
    assert(strcmp("help", caller->name) == 0);

    int depth_cmd2get_help = 0;
    SAPCommand *cmd2get_help = find_sap(&rootCmd, argc, argv, &depth_cmd2get_help);

    if (cmd2get_help == NULL) {
        printf("Unknown command: %s. See '%s help'.\n", argv[depth_cmd2get_help], rootCmd.name);
        return -1;
    }

    print_cmd_help(cmd2get_help);

    return 0;
}

static int call_exec(SAPCommand* caller, int argc, char *argv[]) {
    assert(caller!= NULL);

    int ret = parse_flags(caller, argc, argv);

    if (ret != 0) {
        printf("Unknown option: %s\n", argv[ret]);
        return -1;
    }

    if (caller->parse_by_self == 1) {
        assert(caller->exec_self_parse != NULL);
        return caller->exec_self_parse(caller, argc, argv);
    }

    return caller->exec(caller);
}

/* ---- functions of cmd_exec ---- */



static void add_helpcmd() {
    init_sap_command(&helpCmd, "help", "Display this help message", NULL, NULL);
    set_cmd_self_parse(&helpCmd, help_exec);
    add_subcmd(&rootCmd, &helpCmd);
}



/* ++++ global frame functions that will be called by user ++++ */

void init_sap_command(SAPCommand *cmd, const char *name, const char *short_desc, const char *long_desc, CmdExec exec) {
    assert(cmd!= NULL);
    assert(++cmd_cnt <= MAX_CMD_COUNT);

    cmd->name = name;
    cmd->short_desc = short_desc;
    cmd->long_desc = long_desc;
    cmd->flag_cnt = 0;
    cmd->parse_by_self = 0;
    cmd->exec_self_parse = NULL;
    cmd->exec = (exec == NULL)? void_exec: exec;
    add_flag(cmd, &helpFlag);   /* add help flag to all sapcommand */

    init_tree_node(&cmd->tree_node);
}

void init_root_cmd(const char *name, const char *short_desc, const char *long_desc, CmdExec exec) {
    init_flag(&helpFlag, "help", 'h', "Display the help message", NULL);
    init_sap_command(&rootCmd, name, short_desc, long_desc, exec);
}

void set_cmd_self_parse(SAPCommand *cmd, CmdExecWithArg self_parse_exec) {
    assert(cmd != NULL);
    cmd->parse_by_self = 1;
    if (self_parse_exec == NULL) {
        cmd->exec_self_parse = void_self_parse_exec;
    } else {
        cmd->exec_self_parse = self_parse_exec;
    }
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

    Flag *cmd2help = (Flag *) malloc(sizeof(Flag));
    init_flag(cmd2help, "cmd", 'c', "Specify the command to get help", NULL);
    add_helpcmd();  /* add the subcommand help to root command */
    add_default_flag(&helpCmd, cmd2help);

    cmd2run = find_sap_consider_flags(&rootCmd, argc, argv, &depth);

    if (cmd2run != NULL) {
        return call_exec(cmd2run, argc - depth, argv + depth);
    } else { /* exit of unknown cmds */
        printf("Unknown command: %s. See '%s help'.\n", argv[depth], rootCmd.name);
        return -1;
    }
}

void free_root_cmd() {
    free(helpCmd.default_flag);

    #define not_stack_select ((stack_select == 0)? 1: 0)
    TreeNode *stack[MAX_CMD_COUNT][2];      /* two stack cosplay a queue */
    int top[2] = {-1, -1};                  /* the top ptr of the two stack */
    int stack_select = 0;                   /* select the available stack*/

    /* push the root into stack */
    stack[++top[stack_select]][stack_select] = &(rootCmd.tree_node);

    while (top[stack_select] >= 0) {
        TreeNode *stack_top = stack[top[stack_select]][stack_select];
        SAPCommand *crt_cmd = node2cmd(stack_top);   /* current command */

        for (int i = 0; i < crt_cmd->flag_cnt; i++) {
            if (crt_cmd->flags[i]->type == multi_arg) {
                free(crt_cmd->flags[i]->value);
            }
        }

        for (int i = 0; i < stack_top->child_cnt; i++) {
            /* push the subcmds into the other stack */
            stack[++top[not_stack_select]][not_stack_select] = stack_top->children[i];
        }

        if (--top[stack_select] < 0) {
            /* pop and judge whether the stack is empty */
            stack_select = not_stack_select;    /* switch the stack */
        }

        /* ---- non-leaf node ---- */
    #undef not_stack_select
    }

    free_node_tree(&rootCmd.tree_node);
}

/* ---- global frame functions that will be called by user ---- */
