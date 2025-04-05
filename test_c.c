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

int main(int argc, char *argv[]) {
    int ret = 0;

    init_root_cmd("test_c", "test_c command", "test_c command description", void_exec);
    SAPCommand cmd1;
    SAPCommand cmd2;
    SAPCommand cmd3;
    init_sap_command(&cmd1, "cmd1", "cmd1 command", "cmd1 command description", void_exec);
    init_sap_command(&cmd2, "cmd2", "cmd2 command", "cmd2 command description", void_exec);
    init_sap_command(&cmd3, "cmd3", "cmd3 command", "cmd3 command description", void_exec);

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
    add_flag(&rootCmd, &rootFlag);
    add_flag(&cmd1, &flag1);
    add_flag(&cmd1, &flag2);
    add_flag(&cmd2, &flag3);
    // SAPCommand* parent_of_cmd1= get_parent_cmd(cmd1);

    ret = do_parse_subcmd(argc, argv);

    free_root_cmd();

    return ret;
}
