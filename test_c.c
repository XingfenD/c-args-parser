/**
 * @file test_c.c
 * @brief 
 * @author Fendy (xingfen.star@gmail.com)
 * @version 1.0
 * @date 2025-03-30
 * @copyright Copyright (c) 2025
 */

#include <stdio.h>
#include <scap.h>

int root_exec(SAPCommand* caller) {
    printf("root_exec: %s\n", caller->name);
    printf("root_exec default_flag: %s\n", caller->default_flag->flag_name);
    for (int i = 0; i < caller->flag_cnt; i++) {
        if (caller->flags[i]->type == single_arg) {
            printf("root_exec flag[%d]: %s, single_arg, value: %s\n", i, caller->flags[i]->flag_name, (const char *) caller->flags[i]->value);
        } else if (caller->flags[i]->type == multi_arg) {
            printf("root_exec flag[%d]: %s, multi_arg, address: %p, value:\n", i, caller->flags[i]->flag_name, caller->flags[i]->value);
            for (int j = 0;;j++) {
                if (((char **) caller->flags[i]->value)[j] == NULL) {
                    printf("\n");
                    break;
                }
                printf("%s ", ((char **) caller->flags[i]->value)[j]);
            }
        } else if (caller->flags[i]->type == no_arg) {
            printf("root_exec flag[%d]: %s, no_arg, isNULL: %d\n", i, caller->flags[i]->flag_name, caller->flags[i]->value == NULL);
        }
    }
    return 0;
}

int cmd1_exec(SAPCommand* caller) {
    printf("cmd1_exec: %s\n", caller->name);
    for (int i = 0; i < caller->flag_cnt; i++) {
        printf("cmd1_exec flag[%d]: %s, value: %s\n", i, caller->flags[i]->flag_name, (const char *) caller->flags[i]->value);
    }
    return 0;
}

int cmd2_exec(SAPCommand* caller) {
    printf("cmd2_exec: %s\n", caller->name);
    printf("cmd2_exec default_flag: %s\n", caller->default_flag->flag_name);
    for (int i = 0; i < caller->flag_cnt; i++) {
        if (caller->flags[i]->type == single_arg) {
            printf("cmd2_exec flag[%d]: %s, single_arg, value: %s\n", i, caller->flags[i]->flag_name, (const char *) caller->flags[i]->value);
        } else if (caller->flags[i]->type == multi_arg) {
            printf("cmd2_exec flag[%d]: %s, multi_arg, address: %p, value:\n", i, caller->flags[i]->flag_name, caller->flags[i]->value);
            for (int j = 0;;j++) {
                if (((char **) caller->flags[i]->value)[j] == NULL) {
                    printf("\n");
                    break;
                }
                printf("%s ", ((char **) caller->flags[i]->value)[j]);
            }
        } else if (caller->flags[i]->type == no_arg) {
            printf("cmd2_exec flag[%d]: %s, no_arg, isNULL: %d\n", i, caller->flags[i]->flag_name, caller->flags[i]->value == NULL);
        }
    }
    return 0;
}

int cmd3_exec(SAPCommand* caller, int argc, char *argv[]) {
    printf("cmd3_exec caller->name: %s\n", caller->name);
    if (argc >= 1) {
        printf("cmd3_exec argv[0]: %s\n", argv[0]);
    }
    return 0;
}

int main(int argc, char *argv[]) {
    int ret = 0;

    init_root_cmd("test_c", "test_c command", "test_c command description", root_exec);
    SAPCommand cmd1;
    SAPCommand cmd2;
    SAPCommand cmd3;
    init_sap_command(&cmd1, "cmd1", "cmd1 command", "cmd1 command description", cmd1_exec);
    init_sap_command(&cmd2, "cmd2", "cmd2 command", "cmd2 command description", cmd2_exec);
    init_sap_command(&cmd3, "cmd3", "cmd3 command", "cmd3 command description", void_exec);
    set_cmd_self_parse(&cmd3, cmd3_exec);
    add_subcmd(&rootCmd, &cmd1);
    add_subcmd(&rootCmd, &cmd2);
    add_subcmd(&cmd2, &cmd3);

    Flag flag1;
    Flag flag2;
    Flag flag3;
    Flag rootFlag;
    init_flag(&rootFlag, "rootFlag", 'r', "rootFlag usage", "default_value");
    init_flag(&flag1, "flag1", 'a', "flag1 usage", "default_value");
    init_flag(&flag2, "flag2", 'b', "flag2 usage", "default_value");
    init_flag(&flag3, "multi", 'm', "multi arg flag", NULL);
    set_flag_type(&flag3, multi_arg);
    set_flag_type(&rootFlag, multi_arg);
    add_default_flag(&rootCmd, &rootFlag);
    add_flag(&cmd1, &flag1);
    add_flag(&cmd1, &flag2);
    add_default_flag(&cmd2, &flag3);

    ret = do_parse_subcmd(argc, argv);

    free_root_cmd();

    return ret;
}
