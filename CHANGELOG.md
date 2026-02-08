# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- **Unit Testing Infrastructure**: Comprehensive unit testing framework using Google Test
  - Created `docs/TESTING_STANDARDS.md` with AAA (Arrange-Act-Assert) pattern guidelines
  - Established "one reason to fail" principle for test design
  - Configured Google Test 1.12.1 in CMake build system
  - Created modular test directory structure (`tests/core/`, `tests/mocks/`)
  
- **Camera Unit Tests**: 27 comprehensive tests covering camera functionality
  - Constructor initialization and default values
  - View matrix calculations with various camera positions
  - Movement operations (forward, backward, left, right)
  - Camera update and transformation logic
  - Pitch clamping edge cases (±89 degrees)
  - Projection matrix validation (near/far planes)
  - Speed control and setter functions
  
- **MockOpenGL Framework**: GPU-free testing infrastructure
  - Mock implementation of OpenGL functions for unit testing
  - Call tracking and state management
  - Uniform location caching
  - 31 tests validating mock behavior
  - Enables testing graphics code in CI without GPU
  
- **CI Unit Test Workflow**: Automated testing on every PR
  - Runs on Ubuntu with gcc and clang compilers
  - Tests in both Debug and Release configurations
  - Coverage reporting with gcovr (Debug/gcc builds)
  - Test results visible in PR checks
  - Fails PR if tests don't pass
  
- **Coding Standards**: Established comprehensive coding standards based on Microsoft C++ Core Guidelines
  - Created `.clang-format` configuration file for automatic code formatting
  - Created `.clang-tidy` configuration file for static analysis and linting
  - Added `docs/CODING_STANDARDS.md` with detailed coding guidelines and best practices
  - Implemented GitHub Actions workflow for automated code quality checks
  - Updated README.md with links to coding standards documentation

### Style Guidelines
- **Indentation**: 4 spaces (no tabs)
- **Line Length**: 120 characters maximum
- **Braces**: Allman style for functions/classes, K&R for control structures
- **Naming Conventions**:
  - PascalCase for classes, structs, enums
  - camelCase for functions and methods
  - snake_case for variables, parameters, and file names
  - UPPER_CASE for constants and macros

### Testing
- **Test Coverage**: 58 unit tests passing, targeting ≥80% coverage for tested modules
- **Test Execution**: All tests run without requiring GPU or OpenGL context
- **Test Performance**: Full test suite completes in <5 seconds

### CI/CD
- Added code quality workflow that runs on all pull requests
- Automated clang-format verification
- Automated clang-tidy analysis (informational, non-blocking)
- Documentation completeness checks
- Unit test execution and coverage reporting

## [Previous Releases]

For changes prior to this version, see git history.
