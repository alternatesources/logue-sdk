# Developer Notes for Korg NTS-3 "filter gate" Project

This document provides context, architecture guidelines, build recipes, and development workflows specific to the **filter gate** project.

______________________________________________________________________

## 1. Project Overview & Metadata

- **Project Name**: filter gate
- **SDK Module Target**: `genericfx` (Generic Effect)
- **Developer Registry**:
  - **Developer Name**: `srcs`
  - **Developer ID**: `0x73726373U` (must be updated/verified in [header.c](header.c) and registered in [developer_ids.md](../../../developer_ids.md))
  - **Unit ID**: `0x01U`

______________________________________________________________________

## 2. File Layout

The code for the filter gate unit is located in [platform/nts-3_kaoss/filter_gate/](./):

- [header.c](header.c): Defines parameter descriptors (names, ranges, types) and default XY mappings.
- [effect.h](effect.h): The core C++ `Effect` class containing parameters, LFO accumulator, and the DSP sample processing block (`process()`).
- [unit.cc](unit.cc): Bridges the C API endpoints (`unit_init`, `unit_render`, `unit_set_param_value`, `unit_touch_event`, etc.) to the `Effect` instance.
- [third_party/SvfLinearTrapOptimised2.hpp](third_party/SvfLinearTrapOptimised2.hpp): State Variable Filter C++ class implementing low-pass, band-pass, high-pass, and all-pass filters.
- [Makefile](Makefile) / [config.mk](config.mk): Project build definitions.

______________________________________________________________________

## 3. Build & Output Artifacts

- **Build Target Path**: `platform/nts-3_kaoss/filter_gate` (relative path for build script: `nts-3_kaoss/filter_gate`)
- **Output Artifact**: The resulting stripped unit is created at:
  [filter_gate.nts3unit](filter_gate.nts3unit)
- **Checking Relocations**:
  To verify the compiled ELF contains no unresolved dynamic symbols or relocations (which fail silently or crash the NTS-3 unit loader):
  ```bash
  arm-none-eabi-readelf -r platform/nts-3_kaoss/filter_gate/build/filter_gate.elf
  ```

______________________________________________________________________

## 4. DSP Implementation Details & Constraints

### Avoid Standard Formatting Functions

- The NTS-3 dynamic loader fails to resolve symbols like `snprintf` or floating-point printing (`_printf_float`).
- **Requirement**: Avoid `<cstdio>` formatting functions in parameters. Use custom lightweight integer-to-string conversion utilities (e.g. `format_gate`, `format_cutoff_hz` in [effect.h](effect.h)).

### Static Linking of Math Functions

- If math helper functions (`powf`, `sqrtf`, `tan`) generate PLT dynamic relocations, the NTS-3 loader throws an `ELF error : Resolve Symbol`.
- **Requirement**: Force all library dependencies to statically resolve. The `LDOPT` option in [Makefile](Makefile) must include:
  `-Wl,--exclude-libs,ALL -Wl,--no-undefined`

### Dry Path All-Pass Filtering

- To prevent phasing issues when crossfading between the dry and filtered signals, the "dry" path runs through a matching **all-pass filter** instance (`ap_l`, `ap_r`) instead of being a raw bypass signal.

### Smoothing Parameter Type & Formula

- The `SMOOTH` parameter is configured as a millisecond parameter (`k_unit_param_type_msec`), represented on-screen from `0.0` to `50.0 ms`.
- The smoothing coefficient α (alpha) is computed using:
  $$α = 1 - e^{-T / \tau}$$
  Where $T$ is `sample_rate_reciprocal` and $\tau$ (tau) is the time constant in seconds (`fade_time_ms / 1000.0f`).

### GATE & DEPTH Polarity Interaction

- The `GATE` parameter controls the duty cycle (percentage of the gate cycle spent in the filtered state).
- The `DEPTH` parameter polarity determines the phase alignment of the gate:
  - **Positive Depth (`DEPTH >= 0`):** The filter is active during the **latter** portion of the cycle (Tail Mode).
  - **Negative Depth (`DEPTH < 0`):** The filter is active during the **first** portion of the cycle (Lead Mode).
