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

int cmd1_exec(SAPCommand* caller) {
    printf("cmd1_exec: %s\n", caller->name);
    for (int i = 0; i < caller->flag_cnt; i++) {
        printf("cmd1_exec flag[%d]: %s, value: %s\n", i, caller->flags[i]->flag_name, (const char *) caller->flags[i]->value);
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

    init_root_cmd("test_c", "test_c command", "test_c command description", void_exec);
    SAPCommand cmd1;
    SAPCommand cmd2;
    SAPCommand cmd3;
    init_sap_command(&cmd1, "cmd1", "cmd1 command", "cmd1 command description", cmd1_exec);
    init_sap_command(&cmd2, "cmd2", "cmd2 command", "cmd2 command description", void_exec);
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
    init_flag(&flag3, "flag3", 'c', "flag3 usage", "default_value");
    add_default_flag(&rootCmd, &rootFlag);
    add_flag(&cmd1, &flag1);
    add_flag(&cmd1, &flag2);
    add_default_flag(&cmd2, &flag3);

    // for (int i = 0; i < argc; i++) {
    //     printf("argv[%d]: %s\n", i, argv[i]);
    // }
    // printf("argc: %d\n", argc);

    ret = do_parse_subcmd(argc, argv);

    free_root_cmd();

    return ret;
}
