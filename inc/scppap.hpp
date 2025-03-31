/**
 * @file ./inc/scppap.hpp
 * @brief 
 * @author Fendy (xingfen.star@gmail.com)
 * @version 1.0
 * @date 2025-03-30
 * @copyright Copyright (c) 2025
 */

#pragma once

#include <vector>
#include <string>

class SAPCommand {
    private:
    std::string use;
    std::string short_desc;
    std::string long_desc;
    int (*run)(std::vector<std::string> args);
    std::vector<SAPCommand*> subcommands;

    public:
    ~SAPCommand();
    char *get_use();
};

