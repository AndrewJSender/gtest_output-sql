# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Initial public release
- SQLite output support for GoogleTest via `--gtest_output=sqlite:filename.db`
- Event listener implementation for capturing test execution events
- Database schema with `program`, `environment`, `suite`, and `test` tables
- Example application demonstrating usage
- Comprehensive README with schema documentation and query examples

### Changed
- None yet

### Fixed
- None yet

### Deprecated
- None yet

### Removed
- None yet

### Security
- None yet

## [0.1.0] - 2026-09-08

### Added
- Initial release with basic SQLite output functionality
- Support for capturing test program execution metadata
- Support for capturing test suites and individual test results
- Support for environment setup/teardown events
- CMake build configuration
- MIT License

[Unreleased]: https://github.com/AndrewJSender/gtest_output-sql/compare/v0.1.0...HEAD
[0.1.0]: https://github.com/AndrewJSender/gtest_output-sql/releases/tag/v0.1.0
