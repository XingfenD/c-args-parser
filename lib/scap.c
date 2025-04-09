/**
 * @file ./lib/scap.c
 * @brief a simple command line argument parser in c
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
static const int IS_PROVIDED = 1;   /* the flag is provided */
static enum {
    normal = 0,
    unknown_arg,
    too_many_args,
    too_few_args,
    illegal_equal
} parse_err = normal;

/* ++++ functions of Flags ++++ */

void init_flag(Flag *flag, const char *flag_name, const char shorthand, const char *usage, void *dft_val) {
    assert(flag != NULL);
    flag->flag_name = flag_name;
    flag->shorthand = shorthand;
    flag->usage = usage;
    flag->value = dft_val;
    flag->type = single_arg;
}

SAPCommand *add_flag(SAPCommand *cmd, Flag *flag) {
    /* ensure that the incoming command pointer and flag pointer are not NULL */
    assert(cmd!= NULL);
    assert(flag!= NULL);
    /* check if the number of flags in the command has reached the maximum value */
    if (cmd->flag_cnt >= MAX_OPT_COUNT) {
        /* if the maximum value is reached, return NULL */
        return NULL;
    }
    /* add the flag to the command's flag list and increment the flag counter */
    cmd->flags[cmd->flag_cnt++] = flag;
    /* return the command pointer */
    return cmd;
}

SAPCommand *add_default_flag(SAPCommand *cmd, Flag *flag) {
    /* call the add_flag function and check the return value */
    if (add_flag(cmd, flag) == NULL) {
        /* if the add_flag function returns NULL, return NULL */
        return NULL;
    }
    /* set the default flag of the command and return the command pointer */
    cmd->default_flag = flag;
    return cmd;
}

int add_persist_flag(SAPCommand *cmd, Flag *flag) {
    assert(cmd != NULL);
    assert(flag != NULL);

    #define not_stack_select ((stack_select == 0)? 1: 0)
    TreeNode *stack[MAX_CMD_COUNT][2];      /* two stack cosplay a queue */
    int top[2] = {-1, -1};                  /* the top ptr of the two stack */
    int stack_select = 0;                   /* select the available stack*/
    int fail_cnt = 0;

    /* push the root into stack */
    stack[++top[stack_select]][stack_select] = &(rootCmd.tree_node);

    while (top[stack_select] >= 0) {
        TreeNode *stack_top = stack[top[stack_select]][stack_select];
        SAPCommand *crt_cmd = node2cmd(stack_top);   /* current command */
        if (add_flag(crt_cmd, flag) == NULL) {       /* add the flag to the current command */
            fail_cnt++;
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
    }
    #undef not_stack_select

    return fail_cnt;
}

void set_flag_type(Flag *flag, FlagType type) {
    /* ensure that the incoming flag pointer is not NULL */
    assert(flag != NULL);
    /* if the flag already has a value and the new type is multi-argument */
    if (flag->value != NULL && type == multi_arg) {
        /* print a warning message */
        printf("Warning: the flag %s is already set, but its type is changed to multi_arg\n", flag->flag_name);
        /* set the flag's value to NULL */
        flag->value = NULL;
    }
    /* NOTE: test the no_arg when value is set */
    /* set the flag's type */
    flag->type = type;
}

Flag *get_flag(SAPCommand *cmd, const char *flag_name) {
    assert(cmd != NULL);       /* ensure the command is not NULL. */
    assert(flag_name != NULL); /* ensure the flag name is not NULL. */

    /* iterate through all flags in the command */
    for (int i = 0; i < cmd->flag_cnt; i++) {
        /* compare the flag name with the current flag */
        if (strcmp(cmd->flags[i]->flag_name, flag_name) == 0) {
            return cmd->flags[i]; // Return the matching flag.
        }
    }
    return NULL; /* return NULL if no matching flag is found */
}

Flag *get_flag_by_shorthand(SAPCommand *cmd, char shorthand) {
    assert(cmd != NULL);       /* ensure the command is not NULL */
    assert(shorthand != '\0'); /* ensure the shorthand character is valid */

    /* iterate through all flags in the command */
    for (int i = 0; i < cmd->flag_cnt; i++) {
        /* compare the shorthand character with the current flag */
        if (cmd->flags[i]->shorthand == shorthand) {
            return cmd->flags[i]; /* return the matching flag */
        }
    }
    return NULL; /* return NULL if no matching flag is found */
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

static TreeNode *append_child(TreeNode *parent, TreeNode *child) {
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

SAPCommand *get_parent_cmd(SAPCommand cmd) {
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

typedef enum _ArgType {
    error_option = -1,
    normal_arg = 0,
    short_option = 1,
    long_option = 2,
    long_option_with_equal = 3,
} ArgType;

static ArgType get_option_type(char *arg) {
    if (
        (arg[0] == '-' && arg[1] == '\0') ||
        (arg[0] == '-' && arg[1] != '-' && strlen(arg + 1) > 1) ||
        (arg[0] == '-' && arg[1] == '-' && arg[2] == '=') ||
        (arg[0] == '-' && arg[1] == '-' && arg[2] == '\0')
    ) {
        /* if the arg is an error option */
        return error_option;
    } else if (arg[0] == '-' && arg[1] == '-') {    /* if the arg is a long option */
        if (strchr(arg, '=') != NULL) {
            /* if the arg is a long option with '=' */
            return long_option_with_equal;
        }
        return long_option;
    } else if (arg[0] == '-' && arg[1] != '\0') {   /* if the arg is a short option */
        return short_option;
    } else {    /* if the arg is a normal arg */
        return normal_arg;
    }
}

/**
 * @brief parse the flags (options) in the command line arguments.
 *
 * this function iterates through the command line arguments, identifies the flags,
 * and sets the values for the corresponding flags based on their types (no argument,
 * single argument, or multiple arguments). If an unknown flag or an error option is
 * encountered, it returns the index of that option in the argv array.
 *
 * @param cmd a pointer to the SAPCommand structure representing the command whose
 *            flags are to be parsed.
 * @param argc the number of command line arguments.
 * @param argv the array of command line arguments.
 * @return int returns 0 if the parsing is successful. If an error option is found,
 *             it returns the index of that option in the argv array.
 */
static int parse_flags(SAPCommand *cmd, int argc, char *argv[]) {
    /* Ensure that the input parameters are not null and argc is greater than 0 */
    assert(cmd != NULL);
    assert(argc > 0);
    assert(argv != NULL);

    int p_argv = 1;
    int unused_arg[argc];
    int unused_cnt = 0;
    int i = 0;

    while (p_argv < argc) {
        #define CRT_ARGV argv[p_argv]
        switch (get_option_type(CRT_ARGV))
        {
        case error_option:
            parse_err = unknown_arg;
            return p_argv;
        case short_option:
        case long_option:
            #define CRT_FLAG cmd->flags[i]
            #define CRT_FLAGNAME CRT_FLAG->flag_name
            for (i = 0; i < cmd->flag_cnt; i++) {
                if (
                    (CRT_ARGV[1] == '-' && strcmp(CRT_FLAGNAME, CRT_ARGV + 2) == 0) ||
                    (CRT_ARGV[1] != '-' && CRT_ARGV[1] == CRT_FLAG->shorthand)
                ) {
                    if (CRT_FLAG->type == single_arg) {
                        if (p_argv + 1 >= argc || get_option_type(argv[p_argv + 1]) != normal_arg) {
                            parse_err = too_few_args;
                            return p_argv;
                        }
                        CRT_FLAG->value = argv[++p_argv];
                        break;
                    } else if (CRT_FLAG->type == multi_arg) {
                        int arg_cnt = 0;
                        char *arg_stack[argc];
                        while (++p_argv < argc) {
                            if (get_option_type(argv[p_argv]) == normal_arg) {
                                arg_stack[arg_cnt++] = argv[p_argv];
                            } else {    /* if the next arg is an option */
                                /* stop collecting when an option is encountered */
                                break;
                            }
                        }

                        /* copy the collected arguments to the flag's value */
                        if (arg_cnt == 0) {
                            parse_err = too_few_args;
                            return p_argv - 1;
                        }
                        CRT_FLAG->value = (char **) malloc(sizeof(char *) * (arg_cnt + 1));
                        for (int j = 0; j < arg_cnt; j++) {
                            ((char **) CRT_FLAG->value)[j] = arg_stack[j];
                        }
                        /* add NULL to the end of the argument list as a terminator */
                        ((char **) CRT_FLAG->value)[arg_cnt] = NULL;
                        break;
                    } else if (CRT_FLAG->type == no_arg) {
                        /* if the flag is no-arg */
                        /* set its value to the address of IS_PROVIDED */
                        CRT_FLAG->value = (void *) &IS_PROVIDED;
                        break;

                    }
                }
            }
            #undef CRT_FLAGNAME
            #undef CRT_FLAG

            if (i == cmd->flag_cnt) {   /* unknown flag */
                parse_err = unknown_arg;
                return p_argv;
            }

            break;
        case long_option_with_equal:
            ;
            char *p_equal_ch = strchr(CRT_ARGV, '=');
            assert(p_equal_ch != NULL);

            char *flag2parse = CRT_ARGV + 2;
            int flag2parse_len = p_equal_ch - flag2parse; /* extract the flag name */

            #define CRT_FLAG cmd->flags[i]
            #define CRT_FLAGNAME CRT_FLAG->flag_name

            /* iterate through all the flags of the current command */
            for (i = 0; i < cmd->flag_cnt; i++) {
                /* check if the flag name matches */
                if (
                    strncmp(flag2parse, CRT_FLAGNAME, flag2parse_len) == 0 &&
                    strlen(CRT_FLAGNAME) == (size_t) (flag2parse_len)
                ) {
                    if (CRT_FLAG->type == single_arg) {
                        /*  if it matches, set the value after the equal sign as the flag's value */
                        CRT_FLAG->value = p_equal_ch + 1;
                        break;  /* break the inner loop once a matching flag is found */
                    } else if (CRT_FLAG->type == no_arg) {
                        parse_err = illegal_equal;
                        return p_argv;
                    } else if (CRT_FLAG->type == multi_arg) {
                        parse_err = illegal_equal;
                        return p_argv;
                    }
                }
            }

            if (i == cmd->flag_cnt) {   /* unknown flag */
                parse_err = unknown_arg;
                return p_argv;
            }

            #undef CRR_FLAG
            #undef CRT_FLAGNAME
            break;
        default:
            /* if the arg is a normal arg */
            unused_arg[unused_cnt++] = p_argv;
            break;
        }

        #undef CRT_ARGV

        p_argv++;
    }

    /* if the unused args are more than 0 */
    if (unused_cnt > 0) {
        if (cmd->default_flag == NULL || cmd->default_flag->type == no_arg) {
            /* if the default flag not set */
            parse_err = too_many_args;
            return unused_arg[0];
        } else if (unused_cnt > 1 && cmd->default_flag->type == single_arg) {
            /* if the default flag is single arg */
            parse_err = too_many_args;
            return unused_arg[1];
        } else if (unused_cnt > 0 && cmd->default_flag->type == multi_arg) {
            /* if the default flag is multi arg */
            char **arg_stack = (char **) malloc(sizeof(char *) * (unused_cnt + 1));
            for (int i = 0; i < unused_cnt; i++) {
                arg_stack[i] = argv[unused_arg[i]];
            }
            arg_stack[unused_cnt] = NULL;
            cmd->default_flag->value = arg_stack;
        } else if (unused_cnt == 1 && cmd->default_flag->type == single_arg) {
            /* if the default flag is single arg */
            cmd->default_flag->value = argv[unused_arg[0]];
        }
    }

    if (cmd->default_flag != NULL && cmd->default_flag->type == no_arg) {
        /* if the default flag is no arg and the value is NULL */
        cmd->default_flag->value = (void *) &IS_PROVIDED;
    }
    // if (cmd->default_flag != NULL && cmd->default_flag->type == multi_arg && cmd->default_flag->value == NULL) {
    //     /* if the default flag is multi arg and the value is NULL */
    //     parse_err = too_few_args;
    //     return 1;
    // }

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
            printf("--%s\t%s", cmd->flags[i]->flag_name, cmd->flags[i]->usage);
            if (cmd->default_flag == cmd->flags[i]) {
                printf("\t- default flag");
            }
            printf("\n");
            /* BUG: when options are provided, the default value will be changed */
            // if (cmd->flags[i]->value != NULL) {
            //     printf("    Default: %s\n", (const char *) cmd->flags[i]->value);
            // }
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

int void_exec(SAPCommand *caller) {
    assert(caller != NULL);

    printf("The command %s haven't been allocate function\n", caller->name);

    return 1;
}

int void_self_parse_exec(SAPCommand *caller, int argc, char *argv[]) {
    assert(caller != NULL);
    assert(argv != NULL);

    printf("The command %s haven't been allocate function\n", caller->name);
    printf("argc: %d\n", argc);
    for (int i = 0; i < argc; i++) {
        printf("argv[%d]: %s\n", i, argv[i]);
    }

    return 1;
}

static int help_exec(SAPCommand *caller, int argc, char *argv[]) {
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

static int call_exec(SAPCommand *caller, int argc, char *argv[]) {
    assert(caller!= NULL);

    if (caller->parse_by_self == 1) {
        assert(caller->exec_self_parse != NULL);
        return caller->exec_self_parse(caller, argc, argv);
    }

    int ret = parse_flags(caller, argc, argv);

    if (ret != 0 && parse_err != normal) {
        switch (parse_err) {
        case unknown_arg:
            printf("Argument unrecognized: %s\n", argv[ret]);
            break;
        case too_many_args:
            printf("Too many arguments: %s\n", argv[ret]);
            break;
        case too_few_args:
            printf("Too few arguments: %s\n", argv[ret]);
            break;
        case illegal_equal:
            printf("Illegal option: %s\n", argv[ret]);
            break;
        default:
            printf("Unknown error occurs on: %s\n", argv[ret]);
            break;
        }

        return -1;
    }

    if (helpFlag.value != NULL) {
        /* if the help flag is provided */
        print_cmd_help(caller);
        return 0;
    }

    return caller->exec(caller);
}

/* ---- functions of cmd_exec ---- */



/* ++++ functions for initialization ++++ */

static void add_helpcmd() {
    init_sap_command(&helpCmd, "help", "Display this help message", NULL, NULL);
    set_cmd_self_parse(&helpCmd, help_exec);
    set_flag_type(&helpFlag, no_arg);
    add_subcmd(&rootCmd, &helpCmd);
}

static void check_shorthand() {
    #define not_stack_select ((stack_select == 0)? 1: 0)
    TreeNode *stack[MAX_CMD_COUNT][2];      /* two stack cosplay a queue */
    int top[2] = {-1, -1};                  /* the top ptr of the two stack */
    int stack_select = 0;                   /* select the available stack*/

    /* push the root into stack */
    stack[++top[stack_select]][stack_select] = &(rootCmd.tree_node);

    while (top[stack_select] >= 0) {
        TreeNode *stack_top = stack[top[stack_select]][stack_select];
        SAPCommand *crt_cmd = node2cmd(stack_top);   /* current command */
        SAPCommand *call_stack[MAX_CMD_DEPTH];
        get_cmd_stack(crt_cmd, call_stack);
        Flag *ch_occupied[26] = {0};
        for (int i = 0; i < crt_cmd->flag_cnt; i++) {
            if (crt_cmd->flags[i]->shorthand!= '\0') {
                int char_idx = crt_cmd->flags[i]->shorthand - 'a';
                if (ch_occupied[char_idx] != NULL) {
                    int j = 0;
                    printf("In command: ");
                    while (call_stack[j] != crt_cmd) {
                        printf("%s ", call_stack[j]->name);
                        j++;
                    }
                    printf("%s\n", crt_cmd->name);
                    printf("Warning: shorthand '%c' is already occupied by %s\n", crt_cmd->flags[i]->shorthand, ch_occupied[char_idx]->flag_name);
                } else {
                    ch_occupied[char_idx] = crt_cmd->flags[i];
                }
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
    }
    #undef not_stack_select
}

/* ---- functions for initialization ----*/



/* ++++ global frame functions that will be called by user ++++ */

void init_root_cmd(const char *name, const char *short_desc, const char *long_desc, CmdExec exec) {
    /* init the help flag */
    init_flag(&helpFlag, "help", 'h', "Display the help message", NULL);
    /* call init_sap_cmd to init root cmd */
    init_sap_command(&rootCmd, name, short_desc, long_desc, exec);
}

void init_sap_command(SAPCommand *cmd, const char *name, const char *short_desc, const char *long_desc, CmdExec exec) {
    assert(cmd != NULL);                /* ensure the command pointer is not null */
    assert(++cmd_cnt <= MAX_CMD_COUNT); /* ensure the command count does not exceed the maximum limit */

    cmd->name = name;                   /* set the command name */
    cmd->short_desc = short_desc;       /* set the short description of the command */
    cmd->long_desc = long_desc;         /* set the long description of the command */
    cmd->flag_cnt = 0;                  /* initialize the flag count to 0 */
    cmd->parse_by_self = 0;             /* set parse_by_self to 0 (default parse by the framework) */
    cmd->default_flag = NULL;           /* initialize the default flag to null */
    cmd->exec_self_parse = NULL;        /* initialize the self-parse execution function to null */
    cmd->exec = (exec == NULL) ? void_exec : exec; /* set the execution function, use void_exec if null */
    add_flag(cmd, &helpFlag);           /* add the help flag to the command */

    init_tree_node(&cmd->tree_node);    /* initialize the tree node for the command */
}

void set_cmd_self_parse(SAPCommand *cmd, CmdExecWithArg self_parse_exec) {
    /* ensure the incoming command pointer is not null */
    assert(cmd != NULL);
    /* set the command to parse arguments by itself */
    cmd->parse_by_self = 1;
    /* if the provided self-parse execution function is null */
    if (self_parse_exec == NULL) {
        /* use the default void self-parse execution function */
        cmd->exec_self_parse = void_self_parse_exec;
    } else {
        /* use the provided self-parse execution function */
        cmd->exec_self_parse = self_parse_exec;
    }
}

SAPCommand* add_subcmd(SAPCommand *parent, SAPCommand *child) {
    /* ensure the parent command pointer is not null */
    assert(parent!= NULL);
    /* ensure the child command pointer is not null */
    assert(child!= NULL);

    /* attempt to append the child node to the parent node */
    if (append_child(&parent->tree_node, &child->tree_node) == NULL) {
        return NULL;
    }

    return parent;
}

int do_parse_subcmd(int argc, char *argv[]) {
    /* initialize a pointer to the command to be executed */
    SAPCommand *cmd2run = NULL;
    /* initialize the depth counter */
    int depth = 0;

    /* allocate memory for a flag to specify the command to get help */
    /* the helpCmd's default flag */
    Flag *cmd2get_help = (Flag *) malloc(sizeof(Flag));
    init_flag(cmd2get_help, "cmd", 'c', "Specify the command to get help", NULL);   /* initialize the flag */
    add_helpcmd();  /* add the help subcommand to the root command */
    add_default_flag(&helpCmd, cmd2get_help);   /* add the flag to the help command as the default flag */
    check_shorthand();  /* check the duplicate shorthand */

    /* find the command to execute considering flags */
    cmd2run = find_sap_consider_flags(&rootCmd, argc, argv, &depth);

    /* if the command to execute is found */
    if (cmd2run != NULL) {
        /* execute the command and return its result */
        return call_exec(cmd2run, argc - depth, argv + depth);
    } else {
        /* print an error message for unknown commands */
        printf("Unknown command: %s. See '%s help'.\n", argv[depth], rootCmd.name);
        /* return -1 to indicate an unknown command */
        return -1;
    }
}

void free_root_cmd() {
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
    }
    #undef not_stack_select
    free(helpCmd.default_flag);
    free_node_tree(&rootCmd.tree_node);
}

/* ---- global frame functions that will be called by user ---- */
