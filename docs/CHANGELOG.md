# Changelog

All notable changes to the SCAP (Simple C Argument Parser) project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2025-08-27

### Added
- Complete command-line argument parsing framework
- Hierarchical command structure support
- Flexible flag/option handling (long, short, no-arg, single-arg, multi-arg)
- Auto-generated help system
- Memory-safe design with cleanup APIs
- Persistent flag support across command trees
- Self-parsing command capability
- Default flag functionality

### Changed
- **BREAKING**: Unified I/O rules for `find_sap` family functions
- Improved command tree traversal algorithms for better performance
- Enhanced help command to properly parse arguments and flags through framework
- Optimized memory management and reduced potential leaks

### Fixed
- Help command flag parsing issues
- Command tree depth calculation edge cases
- Memory allocation consistency in tree operations

### Technical Details
- Refactored `find_sap_without_sub_root` and `find_sap_consider_flags_without_sub_root` functions
- Improved error handling in argument parsing
- Enhanced documentation with comprehensive API reference

## [Unreleased]

### Planned Features
- Combined short flags support (e.g., `-rvf`)
- Option dependency validation
- Interactive help system
- Performance optimizations for deep command trees

### Known Issues
- Minor edge cases in multi-argument flag handling
- Performance could be improved for very deep command hierarchies
- `find_sap` function cannot correctly identify Unknown Command when handling cases without subcommands

---

**Note**: This changelog was created retroactively based on git commit history and code analysis. Future releases will maintain detailed change logs from the point of creation.