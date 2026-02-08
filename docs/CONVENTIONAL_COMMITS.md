# Quick Reference: Conventional Commits

This is a quick reference for writing conventional commits that work with our automated release system.

## Commit Message Format

```
<type>[optional scope]: <description>

[optional body]

[optional footer]
```

## Types and Their Effects

| Type | Effect | Version | Example |
|------|--------|---------|---------|
| `feat:` | New feature | Minor (0.1.0 → 0.2.0) | `feat: add particle color picker` |
| `fix:` | Bug fix | Patch (0.1.0 → 0.1.1) | `fix: resolve crash on startup` |
| `feat!:` | Breaking feature | Major (0.1.0 → 1.0.0) | `feat!: redesign shader API` |
| `fix!:` | Breaking fix | Major (0.1.0 → 1.0.0) | `fix!: change config format` |
| `docs:` | Documentation | Patch (0.1.0 → 0.1.1) | `docs: update README` |
| `style:` | Code style | Patch (0.1.0 → 0.1.1) | `style: format code` |
| `refactor:` | Code refactoring | Patch (0.1.0 → 0.1.1) | `refactor: simplify renderer` |
| `test:` | Add/update tests | Patch (0.1.0 → 0.1.1) | `test: add camera tests` |
| `chore:` | Maintenance | Patch (0.1.0 → 0.1.1) | `chore: update dependencies` |
| `perf:` | Performance | Patch (0.1.0 → 0.1.1) | `perf: optimize rendering` |

## Quick Examples

### For a bug fix (patch release)
```bash
git commit -m "fix: resolve memory leak in particle system"
```

### For a new feature (minor release)
```bash
git commit -m "feat: add support for custom particle colors"
```

### For breaking changes (major release)
**Option 1: Use ! in type**
```bash
git commit -m "feat!: redesign configuration file format"
```

**Option 2: Use BREAKING CHANGE footer**
```bash
git commit -m "feat: redesign configuration file format

BREAKING CHANGE: Config file format changed from JSON to YAML"
```

### With scope
```bash
git commit -m "feat(renderer): add HDR support"
git commit -m "fix(camera): correct rotation calculations"
```

## Tips

✅ **DO:**
- Keep description clear and concise
- Use present tense ("add" not "added")
- Start with lowercase letter
- Don't end with period

❌ **DON'T:**
- Mix multiple types in one commit
- Forget the colon after type
- Use past tense

## What Happens Next

When you push to master:
1. GitHub Actions analyzes your commits
2. Determines version bump (major/minor/patch)
3. Updates VERSION and CHANGELOG.md
4. Creates git tag (e.g., v0.2.0)
5. Publishes GitHub release

## More Information

See [RELEASE_PROCESS.md](RELEASE_PROCESS.md) for full documentation.
