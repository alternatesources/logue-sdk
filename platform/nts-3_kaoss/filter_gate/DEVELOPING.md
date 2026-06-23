# Developer & Build Guide for filter gate (Korg NTS-3)

This document contains technical implementation details, project layout, and building/verification commands for developers working on the **filter gate** custom effect plugin.

For user-facing instructions and parameter references, see [README.md](file:///Users/nate/repos/logue-sdk/platform/nts-3_kaoss/filter_gate/README.md).
For high-level project guidelines, see the project-specific [GEMINI.md](file:///Users/nate/repos/logue-sdk/platform/nts-3_kaoss/filter_gate/GEMINI.md) or the root [GEMINI.md](file:///Users/nate/repos/logue-sdk/GEMINI.md).

---

## File Layout

The source code for the filter gate custom effect is located in [platform/nts-3_kaoss/filter_gate/](file:///Users/nate/repos/logue-sdk/platform/nts-3_kaoss/filter_gate/):

- [header.c](file:///Users/nate/repos/logue-sdk/platform/nts-3_kaoss/filter_gate/header.c): Defines plugin metadata, parameters (display ranges, defaults, type descriptors), and default XY touch pad assignments.
- [effect.h](file:///Users/nate/repos/logue-sdk/platform/nts-3_kaoss/filter_gate/effect.h): The core C++ `Effect` class containing parameters, crossfade envelope logic, and the DSP sample processing block (`process()`).
- [unit.cc](file:///Users/nate/repos/logue-sdk/platform/nts-3_kaoss/filter_gate/unit.cc): Bridges the Korg C API endpoints (`unit_init`, `unit_render`, `unit_set_param_value`, `unit_touch_event`, etc.) to the `Effect` C++ instance.
- [third_party/SvfLinearTrapOptimised2.hpp](file:///Users/nate/repos/logue-sdk/platform/nts-3_kaoss/filter_gate/third_party/SvfLinearTrapOptimised2.hpp): State Variable Filter implementation (used for LP/BP/HP morphing, and as matching all-pass filter blocks for the dry signal path).
- [Makefile](file:///Users/nate/repos/logue-sdk/platform/nts-3_kaoss/filter_gate/Makefile) / [config.mk](file:///Users/nate/repos/logue-sdk/platform/nts-3_kaoss/filter_gate/config.mk): Project build definitions and flags.

---

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
- [filter_gate.nts3unit](file:///Users/nate/repos/logue-sdk/platform/nts-3_kaoss/filter_gate/filter_gate.nts3unit)

---

## Technical DSP Architecture & Constraints

### 1. Dry Path All-Pass Matching
To prevent phase cancellation issues when mixing the dry (unfiltered) and wet (filtered) signals, the dry signal path is processed through a dedicated pair of all-pass filters (`ap_l`, `ap_r`) provided by the `SvfLinearTrapOptimised2` class. This aligns the phase shift of the bypass path with the filter, ensuring a cohesive sound during crossfades.

### 3. Smoothing Parameter Type & Formula
The `SMOOTH` parameter is configured as a millisecond parameter (`k_unit_param_type_msec`), represented on-screen from `0.0` to `50.0 ms`.
The smoothing coefficient $\alpha$ (alpha) is computed using:
$$\alpha = 1 - e^{-T / \tau}$$
Where $T$ is the sampling interval ($1 / 48000$ s) and $\tau$ is the time constant in seconds (`fade_time_ms / 1000.0f`).

### 4. Dynamic Loader Constraints (No snprintf)
The NTS-3 dynamic loader fails to resolve symbols like `snprintf` or floating-point printing (`_printf_float`).
- **Requirement:** Avoid `<cstdio>` formatting functions in parameters. Use custom lightweight integer-to-string conversion utilities (e.g. `format_gate` or `format_cutoff_hz` in [effect.h](file:///Users/nate/repos/logue-sdk/platform/nts-3_kaoss/filter_gate/effect.h)).

### 5. Static Linking of Math Functions
If helper functions like `powf`, `sqrtf`, or `tan` generate PLT dynamic relocations, the NTS-3 loader fails.
- **Requirement:** Force all library dependencies to statically resolve. The `LDOPT` option in [Makefile](file:///Users/nate/repos/logue-sdk/platform/nts-3_kaoss/filter_gate/Makefile) must include:
  `-Wl,--exclude-libs,ALL -Wl,--no-undefined`

### 6. Single-Precision FPU
The STM32H725xE processor uses a hardware FPU configured for single precision (`-mfpu=fpv4-sp-d16`). Avoid `double` variables, literals, or double-precision math functions. Always use `float` types, single-precision literals (e.g., `1.0f`), and single-precision math functions (`sinf`, `cosf`, `sqrtf`, `powf`).
