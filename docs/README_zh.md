# SCAP - 简单C语言参数解析器

[![License](https://img.shields.io/badge/License-Apache_2.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

轻量级命令行参数解析框架，支持层级化命令结构和自动帮助生成。

## 功能特性

🌳 **层级化命令**
支持嵌套多级子命令结构

⚙️ **灵活选项处理**

- 长选项 (`--option`)
- 短选项 (`-o`)
- 无参数标志
- 单值/多值参数
- 默认参数捕获

📘 **自动帮助生成**

- 命令描述
- 选项用法
- 错误诊断

🚢 **内存安全**

- 内置内存管理
- 防泄漏设计
- 清理API

## 快速入门

### 1. 编译测试样例

```bash
git clone https://github.com/yourusername/c-args-parser.git
cd c-args-parser
make
```

### 2. 基础实现

```c
#include <scap.h>

int main(int argc, char *argv[]) {
    // 初始化根命令
    init_root_cmd("cli", "示例CLI", "演示SCAP功能", NULL);

    // 添加子命令
    SAPCommand build_cmd;
    init_sap_command(&build_cmd, "build", "构建项目", NULL, build_handler);
    add_subcmd(&rootCmd, &build_cmd);

    // 解析参数
    int ret = do_parse_subcmd(argc, argv);
    free_root_cmd();
    return ret;
}
```

## 核心概念

### 命令结构

```c
typedef struct _SAPCommand {
    const char *name;        // 命令标识符
    const char *short_desc; // 简短描述
    Flag *flags[MAX_OPT_COUNT]; // 关联选项
    // ... (内部管理字段)
} SAPCommand;
```

### 选项配置

```c
typedef struct _Flag {
    const char *flag_name;  // 长选项形式
    char shorthand;         // 短选项字符
    FlagType type;          // 参数类型规范
    // ... (值存储字段)
} Flag;
```

## API参考

### 关键函数

| 函数            | 描述               |
|----------------|-------------------|
| `init_root_cmd()` | 初始化根命令节点 |
| `add_subcmd()`    | 添加嵌套子命令    |
| `add_flag()`      | 注册命令选项      |
| `set_flag_type()` | 定义参数要求      |
| `do_parse_subcmd()` | 执行解析流程    |

## 使用示例

### 多值选项

```c
// 配置多参数选项
Flag input_files;
init_flag(&input_files, "input", 'i', "输入文件", NULL);
set_flag_type(&input_files, multi_arg);
add_default_flag(&cmd, &input_files);

// 访问值
char **files = input_files.value;
while (*files) {
    process_file(*files++);
}
```

### 自定义解析命令

```c
int custom_parser(SAPCommand *caller, int argc, char *argv[]) {
    printf("自定义处理命令：%s\n", caller->name);
    for (int i = 0; i < argc; i++) {
        printf("参数 %d: %s\n", i, argv[i]);
    }
    return 0;
}

// 启用自定义解析
set_cmd_self_parse(&special_cmd, custom_parser);
```

## 项目状态

✅ **生产就绪**
核心解析功能稳定

🔧 **开发中功能**

- [ ] 组合短标志 (例如 `-rvf`)
- [ ] 选项依赖检查

🐞 **最近修复与改进**

- ✅ 统一了find_sap函数的输入输出规则 (v1.0 - 2025年8月)
- ✅ 帮助命令现在能正确解析参数和标志
- ✅ 改进了命令树遍历算法

🐞 **已知问题**

- `find_sap`函数在处理没有子命令的情况下不能正确识别Unknown Command

  示例：以下两个命令的执行结果不一样：
  ```
  # 正确识别未知命令
  build/test_c help sub1 unknown_cmd
  # 输出: Unknown command: unknown_cmd. See 'example help'.

  # 未正确识别未知命令，反而显示了子命令的帮助信息
  build/test_c help sub1 sub1_sub1 unknown_cmd
  # 输出: [显示sub1_sub1的帮助信息]
  ```

## 许可证

基于 [Apache 2.0 许可证](https://opensource.org/licenses/Apache-2.0) 开源