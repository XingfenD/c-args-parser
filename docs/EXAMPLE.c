/**
 * @file ./docs/EXAMPLE.c
 * @brief the example of using scap framework
 * @author Fendy (xingfen.star@gmail.com)
 * @version 1.0
 * @date 2025-05-09
 * @copyright Copyright (c) 2025
 */

#include <stdio.h>
#include <scap.h>

int exec_without_args(SAPCommand *caller) {
    printf("Current Command Name: %s\n", caller->name);
    if (caller->default_flag != NULL) {
        printf("The default_flag name: %s\n", caller->default_flag->flag_name);
    }
    for (int i = 0; i < caller->flag_cnt; i++) {
        if (caller->flags[i]->type == single_arg) {
            printf("%s flag[%d]: %s, single_arg, value: %s\n", caller->name, i, caller->flags[i]->flag_name, (const char *) caller->flags[i]->value);
        } else if (caller->flags[i]->type == multi_arg) {
            printf("%s flag[%d]: %s, multi_arg, address: %p, value:\n", caller->name, i, caller->flags[i]->flag_name, caller->flags[i]->value);
            for (int j = 0; ; j++) {
                if (((char **) caller->flags[i]->value)[j] == NULL) {
                    printf("\n");
                    break;
                }
                printf("%s ", ((char **) caller->flags[i]->value)[j]);
            }
        } else if (caller->flags[i]->type == no_arg) {
            printf("%s flag[%d]: %s, no_arg, isGiven: %d\n", caller->name, i, caller->flags[i]->flag_name, caller->flags[i]->value != NULL);
        }
    }
    return 0;
}

int exec_with_args(SAPCommand *caller, int argc, char *argv[]) {
    printf("Current Command Name: %s\n", caller->name);
    printf("argc: %d\n", argc);
    for (int i = 0; i < argc; i++) {
        printf("argv[%d]: %s\n", i, argv[i]);
    }
    return 0;
}

int main(int argc, char *argv[]) {

    init_root_cmd("example", "This is short description for example", "This is long description for example", exec_without_args);

    Flag root_s;
    init_flag(&root_s, "root_s", 's', "This is long description for root_s", "default_value for root_s");
    add_flag(&rootCmd, &root_s);

    Flag root_m;
    init_flag(&root_m, "root_m", 'm', "This is long description for root_m", NULL);
    set_flag_type(&root_m, multi_arg);
    add_default_flag(&rootCmd, &root_m);

    Flag root_n;
    init_flag(&root_n, "root_n", 'n', "This is long description for root_n", NULL);
    set_flag_type(&root_n, no_arg);
    add_flag(&rootCmd, &root_n);

    SAPCommand sub1;
    init_sap_command(&sub1, "sub1", "This is short description for sub1", "This is long description for sub1", NULL);
    set_cmd_self_parse(&sub1, exec_with_args);
    add_subcmd(&rootCmd, &sub1);

    int ret = do_parse_subcmd(argc, argv);

    free_root_cmd();

    return ret;
}
