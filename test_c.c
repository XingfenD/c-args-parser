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

    // SAPCommand* parent_of_cmd1= get_parent_cmd(cmd1);

    ret = call_subcmd(argc, argv);

    free_root_cmd();

    return ret;
}
