# Contributing to gtest_output-sql

Thank you for your interest in contributing! This document provides guidelines and instructions for contributing to the project.

## Code of Conduct

Be respectful, inclusive, and constructive in all interactions with other contributors.

## Getting Started

1. **Fork the repository** on GitHub
2. **Clone your fork** locally:
   ```sh
   git clone https://github.com/YOUR_USERNAME/gtest_output-sql.git
   cd gtest_output-sql
   ```
3. **Add upstream remote** to stay in sync:
   ```sh
   git remote add upstream https://github.com/AndrewJSender/gtest_output-sql.git
   ```

## Development Workflow

### Creating a Feature Branch

```sh
git checkout -b feature/short-description
```

Branch naming conventions:
- `feature/description` — New features
- `fix/description` — Bug fixes
- `docs/description` — Documentation updates
- `refactor/description` — Code refactoring

### Building and Testing

Initialize submodules:
```sh
git submodule update --init --recursive
```

Build the project:
```sh
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
```

Run tests:
```sh
ctest --verbose
```

## Making Changes

### Code Style

- Follow the existing code style and conventions
- Use 2-space indentation for C++
- Use meaningful variable and function names
- Add comments for complex logic
- Keep functions focused and reasonably sized

### Commit Messages

Write clear, descriptive commit messages:

```
type: Brief summary (50 chars or less)

Optional longer explanation of the change. Explain what and why,
not just how. Keep lines under 72 characters.

Fixes #123 (if applicable)
```

Commit types:
- `feat:` — New feature
- `fix:` — Bug fix
- `docs:` — Documentation changes
- `refactor:` — Code refactoring without changing behavior
- `test:` — Adding or updating tests
- `chore:` — Build, dependencies, or tooling

### Testing

- Add tests for new features
- Update existing tests when changing behavior
- Ensure all tests pass before submitting a PR:
  ```sh
  cd build && ctest
  ```

## Submitting Changes

### Before Submitting a Pull Request

1. **Sync with upstream**:
   ```sh
   git fetch upstream
   git rebase upstream/main
   ```

2. **Ensure tests pass**:
   ```sh
   cmake --build . --config Debug
   ctest --verbose
   ```

3. **Clean up your branch**:
   ```sh
   git log upstream/main..HEAD
   ```

### Creating a Pull Request

1. Push your branch to your fork:
   ```sh
   git push origin feature/your-feature
   ```

2. Go to the [main repository](https://github.com/AndrewJSender/gtest_output-sql)

3. Click "New Pull Request"

4. Select your branch and fill in the PR template:
   - **Title**: Clear, concise description
   - **Description**: Explain what the PR does and why
   - **Related Issues**: Link to issue #123 with "Fixes #123" or "Related to #123"
   - **Checklist**: Verify all items are complete

### PR Guidelines

- Keep PRs focused on a single feature or fix
- Include both code and test changes
- Update documentation if needed
- Respond to reviewer feedback promptly

## Reporting Issues

### Bug Reports

Include:
- **Title**: Clear, specific description
- **Environment**: OS, compiler, C++ standard version
- **Steps to reproduce**: Detailed instructions
- **Expected behavior**: What should happen
- **Actual behavior**: What actually happened
- **Reproduction code**: Minimal example if possible

### Feature Requests

Include:
- **Title**: Clear description of the feature
- **Motivation**: Why is this feature needed?
- **Proposed solution**: How should it work?
- **Alternative approaches**: Other possible solutions

## Documentation

- Update README.md for user-facing changes
- Update CHANGELOG.md following the format
- Add code comments for non-obvious logic
- Keep documentation clear and accessible

## Questions?

- Check existing issues first
- Look at the README and existing code
- Open a discussion or issue if stuck

Thank you for contributing! 🙏
