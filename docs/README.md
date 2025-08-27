# SCAP - Simple C Argument Parser

[![License](https://img.shields.io/badge/License-Apache_2.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

A lightweight command-line argument parsing framework for C with subcommand support and auto-generated help.

## Features

🌳 **Hierarchical Commands**
Build nested command structures with multiple subcommand levels

⚙️ **Flexible Option Handling**

- Long options (`--option`)
- Short options (`-o`)
- No-argument flags
- Single/Multi-value parameters
- Default argument capture

📘 **Auto-generated Help**

- Command descriptions
- Option usage
- Error diagnostics

🚢 **Memory Safe**

- Built-in memory management
- Leak-free design
- Cleanup API

## Quick Start

### 1. Compile the test executable file

```bash
git clone https://github.com/yourusername/c-args-parser.git
cd c-args-parser
make
```

### 2. Basic Implementation

```c
#include <scap.h>

int main(int argc, char *argv[]) {
    // Initialize root command
    init_root_cmd("cli", "Sample CLI", "Demonstrates SCAP capabilities", NULL);

    // Add subcommand
    SAPCommand build_cmd;
    init_sap_command(&build_cmd, "build", "Build project", NULL, build_handler);
    add_subcmd(&rootCmd, &build_cmd);

    // Parse arguments
    int ret = do_parse_subcmd(argc, argv);
    free_root_cmd();
    return ret;
}
```

## Core Concepts

### Command Structure

```c
typedef struct _SAPCommand {
    const char *name;        // Command identifier
    const char *short_desc; // Brief description
    Flag *flags[MAX_OPT_COUNT]; // Associated options
    // ... (internal management fields)
} SAPCommand;
```

### Option Configuration

```c
typedef struct _Flag {
    const char *flag_name;  // Long option form
    char shorthand;         // Short option character
    FlagType type;          // Argument type specification
    // ... (value storage fields)
} Flag;
```

## API Reference

### Key Functions

| Function            | Description                  |
| ------------------- | ---------------------------- |
| `init_root_cmd()`   | Initialize root command node |
| `add_subcmd()`      | Add nested subcommand        |
| `add_flag()`        | Register command option      |
| `set_flag_type()`   | Define argument requirements |
| `do_parse_subcmd()` | Execute parsing process      |

## Usage Examples

### Multi-value Option

```c
// Configure multi-arg option
Flag input_files;
init_flag(&input_files, "input", 'i', "Input files", NULL);
set_flag_type(&input_files, multi_arg);
add_default_flag(&cmd, &input_files);

// Access values
char **files = input_files.value;
while (*files) {
    process_file(*files++);
}
```

### Self-parsing Command

```c
int custom_parser(SAPCommand *caller, int argc, char *argv[]) {
    printf("Custom handling for %s\n", caller->name);
    for (int i = 0; i < argc; i++) {
        printf("Arg %d: %s\n", i, argv[i]);
    }
    return 0;
}

// Enable custom parsing
set_cmd_self_parse(&special_cmd, custom_parser);
```

## Project Status

✅ **Production Ready**
Core parsing functionality stable

🔧 **Under Development**

- [ ] Combined short flags (e.g. `-rvf`)
- [ ] Option dependency checks
- [ ] Interactive help system

🐞 **Recent Fixes & Improvements**

- ✅ Unified I/O rules for find_sap functions (v1.0 - Aug 2025)
- ✅ Help command now properly parses arguments and flags
- ✅ Improved command tree traversal algorithms

🐞 **Known Issues**

- Minor edge cases in multi-argument flag handling
- Performance optimization needed for deep command trees
- `find_sap` function cannot correctly identify Unknown Command when handling cases without subcommands

## License

Open-source under [Apache 2.0 License](https://opensource.org/licenses/Apache-2.0)
