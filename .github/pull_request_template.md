## Description
<!-- Brief description of the changes -->

## ⚠️ PR Title Check
<!-- IMPORTANT: Ensure your PR title follows Conventional Commits format -->
<!-- The PR title becomes the squash commit message and determines version bumps -->
<!-- Format: type(scope): description -->
<!-- Examples: "feat: add color picker", "fix(camera): rotation bug" -->

**Current PR title follows format?**
- [ ] Yes, PR title starts with: feat, fix, docs, style, refactor, test, chore, perf, ci, build, or revert

## Type of Change
<!-- Mark the relevant option with an "x" -->

- [ ] Bug fix (non-breaking)
- [ ] New feature (non-breaking)
- [ ] Breaking change
- [ ] Documentation update
- [ ] Code refactor
- [ ] Performance improvement
- [ ] Other (please describe)

## Related Issues
<!-- Link related issues: Closes #123 -->

## Testing
<!-- Describe how you tested the changes -->

## Checklist
<!-- Ensure all items are completed before requesting review -->

- [ ] My commits follow [Conventional Commits](docs/CONVENTIONAL_COMMITS.md) format
  - Example: `feat: add particle color picker` or `fix(camera): correct rotation calculations`
- [ ] Code passes `clang-format` checks
- [ ] Code passes `clang-tidy` analysis
- [ ] Unit tests pass locally (`ctest` or `cmake --build build --target test`)
- [ ] Build succeeds (`cmake -B build && cmake --build build`)
- [ ] No unrelated changes included

## Notes
<!-- Any additional notes for reviewers -->
