# Developer Notes for Korg NTS-3 Projects

This repository contains custom effects and units for the Korg NTS-3 kaoss pad kit. This file explains the global environment configuration, nix/direnv setup, build workflow, and developer metadata.

______________________________________________________________________

## 1. Project Overview & Target Hardware

- **Target Device**: Korg NTS-3 kaoss pad kit
- **Target Processor**: ARM Cortex-M7 (STM32H725xE)
- **Development Flow**: Code is compiled locally into a `.nts3unit` package. The user loads this package onto their **physical NTS-3 unit** for testing. There is no automated local hardware test runner.
- **Plugin Targets**: Multiple directories/effects can exist under the `platform/nts-3_kaoss/` directory.
- **Template Projects**: The repository includes template directories (such as `platform/nts-3_kaoss/dummy-genericfx`). These template files provide the boilerplate structure. Any code reviews or static analysis activities should ignore issues inherent to these template files (such as unused stub parameters or general WASM build scaffolding configurations), as they are intended for bootstrapping and may not reflect production standards.

______________________________________________________________________

## 2. Nix & Direnv Build Environment

We use a Nix flake combined with `direnv` to provide a consistent development environment without installing compiler toolchains machine-wide.

- **Direnv**: The [.envrc](.envrc) configuration automatically activates the Nix development shell when you enter the project directory.
- **Nix Flake ([flake.nix](flake.nix))**: Packages like `docker`, `colima`, and `emscripten` are managed by the flake and available in the environment.
- **flake.lock**: The `flake.lock` file locks the exact dependency revisions for reproducibility. It must be committed to Git and only updated intentionally via `nix flake update`.
- **OS Environment**: macOS (host running Colima for the Docker daemon).
- **Compiling**: All target compilation is done inside a Nix-managed NixOS container via Docker.
- **Tooling Updates**: Whenever tooling in `flake.nix` or the `direnv` configuration is updated, the AI must remind the user to exit `agy-cli` and run `/resume` in a fresh shell to load the updated environment.

______________________________________________________________________

## 3. General Build Commands

All build commands must be run from the repository root:

- **Clean build a project**:

  ```bash
  ./docker/run_cmd.sh build --clean nts-3_kaoss/<project_name>
  ```

  *(e.g., `./docker/run_cmd.sh build --clean nts-3_kaoss/filter_gate`)*

- **Standard build a project**:

  ```bash
  ./docker/run_cmd.sh build nts-3_kaoss/<project_name>
  ```

  *(e.g., `./docker/run_cmd.sh build nts-3_kaoss/filter_gate`)*

- **Checking Relocations**:
  To verify the compiled ELF contains no unresolved dynamic symbols or relocations (which fail silently or crash the NTS-3 unit loader):

  ```bash
  arm-none-eabi-readelf -r platform/nts-3_kaoss/<project_name>/build/<project_name>.elf
  ```

- **Code Formatting (CRITICAL for AI)**:
  You MUST automatically format any modified source and header files using `clang-format` immediately after making code changes and before building. Run it using a safe shell loop to avoid globbing errors if some file extensions don't exist in the project:

  ```bash
  for f in platform/nts-3_kaoss/<project_name>/*.{cc,h,c,hpp}; do if [ -f "$f" ]; then clang-format -i "$f"; fi; done
  ```

______________________________________________________________________

## 4. Developer Registry & Metadata

- **Developer Name**: `srcs` (all lowercase)
- **Developer ID**: `0x73726373U`
- **Developer Link**: `https://github.com/alternatesources`

______________________________________________________________________

## 5. Projects

Target-specific configuration, file layouts, DSP implementation details, and custom parameter details are documented inside each project's subdirectory:

- **Filter Gate**: [platform/nts-3_kaoss/filter_gate/GEMINI.md](platform/nts-3_kaoss/filter_gate/GEMINI.md)

______________________________________________________________________

## 6. DSP Coding Guidelines

When developing or refactoring DSP algorithms for the Korg NTS-3:

- **Single-Precision FPU**: The target processor ARM Cortex-M7 uses a hardware FPU configured for single precision (`-mfpu=fpv4-sp-d16`). Avoid `double` variables, literals, or double-precision math functions (`sin`, `cos`, `sqrt`, `pow`, etc.). Always use `float` types, single-precision literals (e.g., `1.0f`), and single-precision math functions (`sinf`, `cosf`, `sqrtf`, `powf`). Double-precision math functions will trigger slow software-emulated double-precision routines, leading to major performance issues on-device. Note that the `-fsingle-precision-constant` compiler option does NOT automatically promote `double` variables or double-precision functions to float.

______________________________________________________________________

## 7. Documentation Style Guidelines

When writing or updating project documentation, README files, or parameter descriptions, follow these tone and style guidelines:

- **Prefer Technical Precision**: Describe what things do accurately and directly rather than trying to "sell" the feature. Focus on the factual mechanics, behavior, and parameters.
- **Avoid Flowery or Promotional Language**: Tone down words that border on a sales pitch. Avoid elevating mundane details into marketing points.
- **Keep it Objective**: Let the functionality speak for itself without adding subjective modifiers.
