# Contributing

## How to Contribute

1. Fork the repository.
2. Create a feature branch (`git checkout -b feature/your-feature`).
3. Commit changes (`git commit -m 'Add your feature'`).
4. Push to the branch (`git push origin feature/your-feature`).
5. Open a pull request.

## Conventional Commits

This project enforces conventional commit standards.

- Commit messages must start with a type: `feat:`, `fix:`, `docs:`, `style:`, `refactor:`, `test:`, `chore:`, `perf:`, `ci:`, `build:`, `revert:`
- First line must be lowercase and ≤30 characters.
- To enable validation, copy the hook: `cp scripts/commit-msg .git/hooks/ && chmod +x .git/hooks/commit-msg`
- To rewrite messages, use `scripts/rewrite_msg.sh`

## Pre-commit Hooks

The project uses pre-commit hooks for code quality.

- Install pre-commit: `pip install pre-commit`
- Install hooks: `pre-commit install`
- Run on all files: `pre-commit run --all-files`
- Hooks include: trailing whitespace, YAML linting, file size checks
