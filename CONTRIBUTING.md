# Contributing to Skrash

Thank you for your interest in contributing to Skrash!

Skrash is a native Scratch 3.0 runtime designed to execute `.sb3` projects faithfully. It also provides an optional Python API for control, debugging, and automation.

Contributions of all kinds are welcome, including bug fixes, runtime improvements, renderer enhancements, tests, documentation, and API improvements.

## Table of Contents

- [Getting Started](#getting-started)
- [Development Requirements](#development-requirements)
- [Building and Testing](#building-and-testing)
- [Types of Contributions](#types-of-contributions)
- [Reporting Bugs](#reporting-bugs)
- [Suggesting Features](#suggesting-features)
- [Submitting Changes](#submitting-changes)
- [Code Guidelines](#code-guidelines)
- [Tests and Quality](#tests-and-quality)

## Getting Started

1. Fork the repository.
2. Clone your fork:

   ```bash
   git clone https://github.com/<your-username>/skrash.git
   cd skrash
   ```

3. Create a branch for your changes:

   ```bash
   git checkout -b fix/short-description
   ```

Keep your changes focused and avoid mixing unrelated modifications in a single pull request.

## Development Requirements

Skrash uses C for its native runtime and renderer, with Python providing an optional API.

The development environment requires:

- GCC
- GNU Make
- Python 3
- `python3-config`
- SDL2 development libraries
- libzip development libraries
- pytest

Make sure the required development headers and libraries are installed before building.

## Building and Testing

The project uses a Makefile to manage compilation and tests.

### Build the project

```bash
make
```

This builds the native Python extension and compiles the C test executables.

**Note:** `make` compiles the test executables but does not execute the tests.

### Run the complete test suite

```bash
make test
```

This builds the project, runs the native C tests, and then runs the Python tests using pytest.

### Run individual test suites

Run the C tests:

```bash
make ctest
```

Run the Python tests:

```bash
make pytest
```

### Build the standalone renderer

```bash
make play
```

### Clean generated files

```bash
make clean
```

Before submitting a pull request, run:

```bash
make test
```

Include any relevant test results in your pull request description.

## Types of Contributions

Contributions can include:

- **Bug fixes:** Correct incorrect or unexpected runtime behavior.
- **Runtime improvements:** Improve execution accuracy, reliability, and compatibility with Scratch projects.
- **Renderer improvements:** Improve rendering behavior and visual fidelity.
- **Python API:** Improve usability, consistency, and documentation.
- **Tests:** Add coverage for existing behavior and newly implemented features.
- **Documentation:** Clarify usage, architecture, limitations, and development workflows.
- **Performance:** Identify and address measurable performance problems.

Small, focused contributions are especially welcome.

## Reporting Bugs

Before opening a bug report:

1. Check whether the issue has already been reported.
2. Make sure you are using a recent version of the repository.
3. Try to reproduce the problem with the smallest possible example.

A useful bug report should include:

- A clear and descriptive title.
- A description of the expected behavior.
- A description of the actual behavior.
- Steps to reproduce the issue.
- Relevant error messages or logs.
- The environment in which the issue occurs.
- A minimal `.sb3` project or code example, when possible.

Avoid combining multiple unrelated bugs into one issue.

## Suggesting Features

Feature requests should explain:

- The problem or limitation being addressed.
- Why the feature would be useful.
- How it could fit into Skrash's existing behavior and architecture.
- Any relevant Scratch 3.0 behavior or reference implementation.

For features intended to preserve Scratch compatibility, provide examples of the corresponding Scratch behavior whenever possible.

## Submitting Changes

When submitting a pull request:

1. Make sure your branch is based on the latest relevant version of the repository.
2. Keep the changes focused on one objective.
3. Add or update tests when appropriate.
4. Run `make test`.
5. Update documentation if your changes affect user-facing behavior or the development workflow.
6. Explain the motivation and implementation in the pull request description.

A good pull request description should contain:

- **What:** What changed?
- **Why:** What problem does it solve?
- **How:** How was it implemented?
- **Testing:** What tests were run, and what were their results?

If the change is still a work in progress, clearly mark the pull request as a draft.

## Code Guidelines

### C

- Follow the conventions already used in the relevant source files.
- Prefer clear and descriptive names.
- Keep functions focused on a specific responsibility.
- Handle errors explicitly.
- Avoid unnecessary complexity and duplicated logic.
- Pay attention to memory management and resource cleanup.
- Consider edge cases when modifying runtime behavior.

### Python

- Follow the existing API conventions.
- Prefer readable and predictable interfaces.
- Document public-facing behavior.
- Handle invalid inputs and runtime errors appropriately.
- Avoid introducing unnecessary dependencies.

### General

- Keep changes as small and understandable as possible.
- Avoid unrelated refactoring.
- Do not introduce breaking changes without explaining their necessity.
- Preserve existing behavior unless the change explicitly intends to modify it.

## Tests and Quality

Tests are important for maintaining the correctness of the runtime.

When implementing a fix or feature:

- Add regression tests for bugs whenever practical.
- Test relevant edge cases.
- Verify that existing behavior remains intact.
- Prefer tests that reproduce the actual problem.
- Avoid tests that merely duplicate implementation details.

For changes affecting Scratch compatibility, tests should ideally demonstrate the expected behavior using a representative Scratch project.

### Scratch Project Fixtures

For integration and compatibility tests, prefer genuine `.sb3` projects created and exported directly from the Scratch editor.

This ensures compatibility with Scratch's actual project structure and makes it easier to inspect, modify, and reproduce test scenarios directly in Scratch.

Scratch-exported projects are especially useful for features that require detailed visual verification.

Reserve manually constructed fixtures for focused unit tests, edge cases, or structures that are difficult or impossible to create in Scratch.

A successful compilation alone does not guarantee that a change works correctly. Run the complete test suite whenever possible.

---

Thank you for helping improve Skrash!
