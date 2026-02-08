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

- **Particle Unit Tests**: 25 comprehensive tests covering Particle data structure
  - Default constructor validation (64000 particle grid generation)
  - Custom constructor with N particles and translation arrays
  - `changeTranslations()` data replacement and null pointer handling
  - `changeVelocities()` data loading and null pointer handling
  - Memory management and destructor validation
  - Particle count consistency across operations
  - Edge cases (zero particles, single particle)

- **SettingsIO Unit Tests**: 37 comprehensive tests covering SettingsIO functionality
  - Default and file-based constructor validation
  - Binary file reading at specific frames
  - Frame clamping (negative values, beyond max)
  - `togglePlay()` playback state management
  - `getFrames()` frame count calculation
  - Comprehensive getter method validation for all simulation parameters
  - File error handling and graceful degradation
  
- **MockOpenGL Framework**: GPU-free testing infrastructure
  - Mock implementation of OpenGL functions for unit testing
  - Extended buffer operation mocking (glGenBuffers, glDeleteBuffers, glBindBuffer, etc.)
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

### Fixed
- **Particle Class**: Fixed uninitialized velocities pointer in custom constructor causing undefined behavior
- **SettingsIO Class**: Implemented missing destructor to prevent linker errors

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
- **Test Coverage**: 120 unit tests passing (100% pass rate)
  - Camera: 27 tests
  - Shader: 31 tests  
  - Particle: 25 tests
  - SettingsIO: 37 tests
- **Module Coverage**:
  - particle.hpp: 80.4% line coverage, 75.0% function coverage
  - settingsIO.hpp: 86.0% line coverage, 88.0% function coverage
- **Test Execution**: All tests run without requiring GPU or OpenGL context
- **Test Performance**: Full test suite completes in <1 second

### CI/CD
- Added code quality workflow that runs on all pull requests
- Automated clang-format verification
- Automated clang-tidy analysis (informational, non-blocking)
- Documentation completeness checks
- Unit test execution and coverage reporting

## [Previous Releases]

For changes prior to this version, see git history.
