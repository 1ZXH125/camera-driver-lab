# Contributing to camera-driver-lab

Thank you for your interest in improving `camera-driver-lab`.

This repository is designed as a practical learning and portfolio project for Linux camera driver engineers. Contributions should keep the project realistic, focused, and useful for bring-up and debugging workflows.

## Goals

Contributions should help with at least one of the following:

- camera bring-up and validation workflows
- V4L2 capture experiments
- sensor register table handling
- log analysis for common camera failures
- MIPI CSI-2 or device tree learning notes
- reusable examples and debugging references

## What to contribute

Good contributions include:

- new runnable tools in `src/`
- better sample logs or register tables in `examples/`
- practical bring-up notes in `docs/`
- bug fixes or cleanup in existing utilities
- improvements to the README and usage examples

## What to avoid

Please avoid contributions that are:

- unrelated to Linux camera driver workflows
- overly generic without a clear practical use case
- copied from vendor code or proprietary material
- dependent on private data, secrets, or confidential logs

## Repository structure

- `src/` — runnable tools and small experiments
- `docs/` — bring-up notes, troubleshooting guides, and protocol summaries
- `examples/` — sample logs, sensor tables, and configuration snippets
- `tests/` — regression inputs and fixtures
- `scripts/` — helper scripts for analysis or generation tasks

## Suggested workflow

1. Check the existing docs and tools first
2. Keep changes small and easy to review
3. Add or update an example if you introduce a new parser or analyzer
4. Document how to build and run the change
5. Verify the code compiles cleanly

## Coding guidelines

- Prefer simple, readable code over clever abstractions
- Keep tools focused on one task
- Use clear names for camera-specific concepts
- Handle errors explicitly
- Avoid adding dependencies unless they are clearly needed

## Documentation guidelines

When adding or changing a tool, update the relevant docs:

- `README.md` for top-level usage and project overview
- `src/README.md` for tool summaries
- `docs/README.md` for new notes
- `examples/README.md` for sample inputs

## Testing

For runnable tools:

- build the project with `make`
- run the tool with the provided example inputs when possible
- confirm the output is stable and understandable

For docs-only changes:

- check that file names and links are correct
- make sure the new content matches the project style

## Commit message style

Use short, descriptive commit messages such as:

- `docs: add camera bring-up practice note`
- `tools: add camera log analyzer`
- `examples: add sample CSI timeout log`
- `fix: improve v4l2 capture frame counting`

## Suggested pull request description

A good PR description should include:

- what was added or changed
- why it is useful for camera bring-up or debugging
- how it was tested
- any limitations or follow-up ideas

## License and ownership

Only contribute content that you have the right to share.

If you add material from another source, make sure it is allowed by that source's license and attribution requirements.

## Final note

The best contributions to this repository are practical, specific, and grounded in real camera driver work.
