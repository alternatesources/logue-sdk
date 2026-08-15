# Developer & Build Guide for filter gate (Korg NTS-3)

This document contains technical implementation details, project layout, and building/verification commands for developers working on the **filter gate** custom effect plugin.

For user-facing instructions and parameter references, see [README.md](README.md).
For high-level project guidelines, see the project-specific [GEMINI.md](GEMINI.md) or the root [GEMINI.md](../../../GEMINI.md).

______________________________________________________________________

## File Layout

The source code for the filter gate custom effect is located in [platform/nts-3_kaoss/filter_gate/](./):

- [header.c](header.c): Defines plugin metadata, parameters (display ranges, defaults, type descriptors), and default XY touch pad assignments.
- [effect.h](effect.h): The core C++ `Effect` class containing parameters, crossfade envelope logic, and the DSP sample processing block (`process()`).
- [unit.cc](unit.cc): Bridges the Korg C API endpoints (`unit_init`, `unit_render`, `unit_set_param_value`, `unit_touch_event`, etc.) to the `Effect` C++ instance.
- [third_party/SvfLinearTrapOptimised2.hpp](third_party/SvfLinearTrapOptimised2.hpp): State Variable Filter implementation (used for LP/BP/HP morphing, and as matching all-pass filter blocks for the dry signal path).
- [Makefile](Makefile) / [config.mk](config.mk): Project build definitions and flags.

______________________________________________________________________

## Build Workflow & Commands

All build commands must be run from the root of the repository:

### 1. Code Formatting

Format modified source and header files using `clang-format` before building:

```bash
clang-format -i platform/nts-3_kaoss/filter_gate/*.cc platform/nts-3_kaoss/filter_gate/*.h platform/nts-3_kaoss/filter_gate/*.c platform/nts-3_kaoss/filter_gate/*.hpp
```

### 2. Standard Build

Compile the project using the Nix-managed Docker build environment:

```bash
./docker/run_cmd.sh build nts-3_kaoss/filter_gate
```

### 3. Clean Build

Perform a clean build to rebuild all dependencies:

```bash
./docker/run_cmd.sh build --clean nts-3_kaoss/filter_gate
```

### 4. Verifying Relocations

To prevent the physical NTS-3 unit from throwing a silent loader crash or "Resolve Symbol" ELF error, verify that the compiled ELF contains no unresolved dynamic symbols or relocations:

```bash
arm-none-eabi-readelf -r platform/nts-3_kaoss/filter_gate/build/filter_gate.elf
```

### Output Artifact

The compiled, ready-to-load package is generated at:

- [filter_gate.nts3unit](filter_gate.nts3unit)

______________________________________________________________________

## Technical DSP Architecture & Constraints

### 1. Dry Path All-Pass Matching

To prevent phase cancellation issues when mixing the dry (unfiltered) and wet (filtered) signals, the dry signal path is processed through a dedicated pair of all-pass filters (`ap_l`, `ap_r`) provided by the `SvfLinearTrapOptimised2` class. This aligns the phase shift of the bypass path with the filter, ensuring a cohesive sound during crossfades.

### 2. Smoothing Parameter Type & Formula

The `SMOOTH` parameter is configured as a millisecond parameter (`k_unit_param_type_msec`), represented on-screen from `0.0` to `50.0 ms`.
The smoothing coefficient $\\alpha$ (alpha) is computed using:
$$\\alpha = 1 - e^{-T / \\tau}$$
Where $T$ is the sampling interval ($1 / 48000$ s) and $\\tau$ is the time constant in seconds (`fade_time_ms / 1000.0f`).
