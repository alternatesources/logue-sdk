# Filter Gate for Korg NTS-3 Kaoss Pad

A rhythmic trance gate effect that uses a morphing filter instead of a volume gate. The gate cuts between the full input signal and a version processed with a variable (low-pass, band-pass, or high-pass) filter, utilizing smoothing and phase-correct fades for clean, click-free performance.

---

## Features

- **Tempo-Aligned and Free Modes:** Lock the gate to the NTS-3 internal tempo or external clock, or run in a free-running mode that triggers and resets phase when you touch the pad.
- **Filter Type Morphing:** Smooth crossfading from Low-Pass (LP) → Band-Pass (BP) → High-Pass (HP).
- **Phasing-Free Bypass:** The dry bypass signal runs through matching all-pass filters at low cutoffs to prevent phase cancellation when mixed with the filtered signal.

---

## Default Pad Mappings

By default, the effect maps the following controls to the touch pad and controls:
- **X-Axis:** Filter Cutoff Frequency (`CUTOFF`)
- **Y-Axis:** Filter Resonance (`RESO` / Q-Factor)
- **FX DEPTH:** Dry/Wet Blend & Direction (`DEPTH`)

---

## Parameter Reference

The Filter Gate plugin exposes 8 adjustable parameters:

| ID | Parameter | Display Range | Default VALUE | Description |
| :-: | :--- | :--- | :--- | :--- |
| **0** | `TIME` | `0` to `17` | `6` (`1/16`) | Sets the time division for the gate rhythm (tempo-aligned or free-running). |
| **1** | `CUTOFF` | `20` to `20000` | `20000` (`20.0 kHz`) | Sets the filter cutoff frequency (20 Hz to 20.0 kHz, exponentially mapped). |
| **2** | `RESO` | `50` to `30000` | `50` (`0.50`) | Controls filter resonance (Q-factor from `0.50` to `300.00`). |
| **3** | `TYPE` | `0` to `1000` | `0` (LP) | Morphs the filter type: LP (0) → BP (500) → HP (1000). |
| **4** | `GAIN` | `0` to `1000` | `1000` (Unity) | Adjusts the output volume of the filtered (wet) signal path. |
| **5** | `SMOOTH` | `0` to `500` | `20` (`2.0 ms`) | Adjusts the crossfade transition time (`0.0` to `50.0 ms`) between states. |
| **6** | `GATE` | `0` to `100` | `50` (`50%`) | Controls the gate duty cycle (percentage of time spent in the filtered/wet state). |
| **7** | `DEPTH` | `-100` to `100` | `100` (`100%`) | Controls overall dry/wet balance and gate direction polarity. |

---

## Detailed Parameter Guide

### 1. TIME (Time Division)

The `TIME` parameter determines the speed of the gate and how it responds to touch:
- **Tempo-Aligned Modes (`0` to `8`):** Synchronizes with the NTS-3 internal tempo or external clock.
  - `0`: `1/2` (Half note)
  - `1`: `1/4d` (Dotted quarter note)
  - `2`: `1/4` (Quarter note)
  - `3`: `1/8d` (Dotted eighth note)
  - `4`: `1/8` (Eighth note)
  - `5`: `1/16d` (Dotted sixteenth note)
  - `6`: `1/16` (Sixteenth note, **Default**)
  - `7`: `1/32d` (Dotted thirty-second note)
  - `8`: `1/32` (Thirty-second note)
- **Free-Running Modes (`9` to `17`):**
  - Indicated on the screen with a `free` suffix (e.g., `1/16free`).
  - Touching the pad resets the gate cycle back to `0`, allowing you to manually align the gate rhythm to the beat of a performance.
- **Tip:** When assigning this parameter to the touch pad or FX DEPTH, it is recommended to set the **MIN** and **MAX** ranges to stay within either the tempo-aligned modes (`0` to `8`) or the free-running modes (`9` to `17`). Sweeping across the boundary between the two groups during a performance can cause sudden, disjointed rhythmic shifts.

### 2. CUTOFF (Filter Cutoff Frequency)

Sets the filter's cutoff frequency, mapped exponentially to provide a smooth, musical sweep across the hearing spectrum:
- `20` → 20 Hz
- `20000` → 20.0 kHz (Default)
- **Screen Display:** Handled automatically by the hardware using the standard Hz/kHz parameter format.

### 3. RESO (Filter Resonance / Q-Factor)

Controls the resonance or sharpness of the filter:
- Range: `0.50` (value `50`) to `300.00` (value `30000`).
- Lower values result in a flat, clean response. High values produce sharp, self-oscillating resonant peaks.
- **Tip:** By default, the Y-axis resonance modulation is restricted to a **MAX** limit of `10.00` (value `1000`) with a default **VALUE** of `0.70` (value `70`) to prevent extreme resonance volume spikes. You can increase the **MAX** setting to allow higher resonance if desired.

### 4. TYPE (Filter Type Morph)

Allows continuous morphing between low-pass, band-pass, and high-pass filters. It uses an equal-power crossfade to keep the overall signal level consistent as you change filter characters:
- `0`: Pure **Low-Pass** (LP)
- `1` to `499`: Morphs LP → **Band-Pass** (BP)
- `500`: Pure **Band-Pass** (BP)
- `501` to `999`: Morphs BP → **High-Pass** (HP)
- `1000`: Pure **High-Pass** (HP)

### 5. GAIN (Wet Signal Gain)

Adjusts the volume of the filtered signal, using a cubed response curve to mimic a natural logarithmic volume sweep:
- `0` → Muted (silent)
- `1000` → Unity Gain (Default)

### 6. SMOOTH (Crossfade Smoothing Time)

Controls the duration of the crossfade transition when the gate switches between the unfiltered and filtered states (`0.0` to `50.0 ms`, default `2.0 ms`):
- Lower values provide sharp rhythmic gating but extremely low values may cause clicking.
- Higher values soften the gating effect, transforming it into a smooth tremolo or rhythmic wah.

### 7. GATE (Duty Cycle)

Controls the percentage of each cycle spent in the filtered (wet) state versus the unfiltered (dry) state. The phase alignment (whether the filter is active at the beginning or end of the cycle) is determined by the polarity of the `DEPTH` parameter:
- **Tail Mode (Positive `DEPTH`):** The filter is active during the **latter** portion of the cycle.
  - *Example (Default `GATE = 50`):* The first 50% of the cycle is unfiltered, and the remaining 50% is filtered.
  - *Example (`GATE = 100`):* The filter is active 100% of the cycle.
- **Lead Mode (Negative `DEPTH`):** The filter is active during the **first** portion of the cycle.
  - *Example (`GATE = 50`):* The first 50% of the cycle is filtered, and the remaining 50% is unfiltered.
  - *Example (`GATE = 0`):* The filter is active 0% of the cycle (entirely unfiltered).

### 8. DEPTH (Dry/Wet Balance & Direction)

Controls the balance between the unprocessed (dry) signal and the filtered (wet) signal, as well as the active gate alignment direction:
- `100` → 100% Wet, Tail Mode (Default)
- `0` → 100% Dry (no gating effect)
- `-100` → 100% Wet, Lead Mode (Inverted direction)
- **Tip:** By default, the touch pad mapping for depth restricts the **MIN** limit to `0` (keeping the gate in Tail Mode). You can adjust the **MIN** to `-100` to allow sweeps that cross over into the inverted Lead Mode.
