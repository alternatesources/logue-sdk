#pragma once
/*
    BSD 3-Clause License

    Copyright (c) 2023, KORG INC.
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this
      list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.

    * Neither the name of the copyright holder nor the names of its
      contributors may be used to endorse or promote products derived from
      this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
    OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

//*/

/*
 *  filter gate - Trance-Gate like filter effect with equal-power LP/BP/HP
 *  morphing.
 *
 */
#include "processor.h"
#include "unit_genericfx.h"
#include "third_party/SvfLinearTrapOptimised2.hpp"
#include <cmath>
#include <algorithm>

namespace {
  /**
   * Calculates the one-pole smoothing coefficient.
   * @param fade_time_ms Fade time in milliseconds.
   * @param sr_reciprocal Sample rate reciprocal (1.0 / sample_rate) in seconds.
   * @return The smoothing coefficient alpha in range [0, 1].
   */
  float calculate_alpha(float fade_time_ms, float sr_reciprocal) {
    // Avoid division by zero
    if (fade_time_ms <= 0.0f) {
      return 1.0f;  // Instant transition
    }

    // Time constant in seconds
    float tau = fade_time_ms / 1000.0f;

    // Calculate coefficient: alpha = 1 - exp(-T / tau)
    // Where T = sr_reciprocal
    return 1.0f - expf(-sr_reciprocal / tau);
  }

  const float time_div_multipliers[9] = {
      1.f / 8.f,  // 1/2
      1.f / 6.f,  // 1/4d
      1.f / 4.f,  // 1/4
      1.f / 3.f,  // 1/8d
      1.f / 2.f,  // 1/8
      2.f / 3.f,  // 1/16d
      1.f,        // 1/16
      4.f / 3.f,  // 1/32d
      2.f         // 1/32
  };
}  // namespace

class Effect : public Processor {
 public:
  uint32_t getBufferSize() const override final { return 0U; }  // No SDRAM buffer needed

  // audio parameters
  enum {
    PARAM_TIME = 0U,
    PARAM_CUTOFF,
    PARAM_RESO,
    PARAM_TYPE,
    PARAM_GAIN,
    PARAM_SMOOTH,
    PARAM_GATE,
    PARAM_DEPTH,
    NUM_PARAMS
  };

  struct Params {
    int32_t time_div;       // 0-17
    int32_t gate;           // 0-100
    float cutoff_hz;        // 20Hz - 20kHz
    float q;                // 0.50 - 300.00
    int32_t type;           // 0-1000
    float gain_coef;        // cubed gain multiplier
    float smoothing_alpha;  // smoothing coefficient alpha [0, 1]
    float depth;            // -100 to 100

    void reset(float sr_reciprocal = 1.f / 48000.f) {
      time_div = 6;                                            // 1/16 note
      gate = 50;                                               // 50% duty cycle
      cutoff_hz = 20000.f;                                     // Default to 20kHz
      q = 50.f / 100.f;                                        // Q = 0.50
      type = 0;                                                // LP
      gain_coef = 1.f;                                         // unity
      smoothing_alpha = calculate_alpha(2.0f, sr_reciprocal);  // Default 2.0 ms
      depth = 100.f;                                           // fully wet
    }

    Params() { reset(); }
  };

  Effect() {}

  inline void setParameter(uint8_t index, int32_t value) override final {
    switch (index) {
      case PARAM_TIME:
        params_.time_div = std::max(0, std::min(17, (int)value));
        break;

      case PARAM_GATE:
        params_.gate = std::max(0, std::min(100, (int)value));
        break;

      case PARAM_CUTOFF:
        params_.cutoff_hz = (float)value;
        updateFilterCoefficients();
        updateRoutingCoefficients();
        break;

      case PARAM_RESO:
        params_.q = std::max(50, std::min(30000, (int)value)) / 100.f;
        updateFilterCoefficients();
        break;

      case PARAM_TYPE:
        params_.type = std::max(0, std::min(1000, (int)value));
        updateRoutingCoefficients();
        break;

      case PARAM_GAIN: {
        float gain_raw = std::max(0, std::min(1000, (int)value)) / 1000.f;
        params_.gain_coef = gain_raw * gain_raw * gain_raw;
      } break;

      case PARAM_SMOOTH: {
        // value 0-500 represents 0.0-50.0 ms
        float fade_time_ms = std::max(0, std::min(500, (int)value)) / 10.f;
        params_.smoothing_alpha = calculate_alpha(fade_time_ms, sample_rate_reciprocal);
      } break;

      case PARAM_DEPTH:
        params_.depth = std::max(-100, std::min(100, (int)value));
        break;

      default:
        break;
    }
  }

  inline const char * getParameterStrValue(uint8_t index, int32_t value) const override final {
    if (index == PARAM_TIME) {
      switch (value) {
        case 0:
          return "1/2";
        case 1:
          return "1/4d";
        case 2:
          return "1/4";
        case 3:
          return "1/8d";
        case 4:
          return "1/8";
        case 5:
          return "1/16d";
        case 6:
          return "1/16";
        case 7:
          return "1/32d";
        case 8:
          return "1/32";
        case 9:
          return "1/2free";
        case 10:
          return "1/4dfree";
        case 11:
          return "1/4free";
        case 12:
          return "1/8dfree";
        case 13:
          return "1/8free";
        case 14:
          return "1/16dfree";
        case 15:
          return "1/16free";
        case 16:
          return "1/32dfree";
        case 17:
          return "1/32free";
        default:
          break;
      }
    }

    return nullptr;
  }

  // life-cycle methods
  void init(float * allocated_buffer) override final {
    (void)allocated_buffer;

    sample_rate_reciprocal = 1.f / getSampleRate();
    current_tempo_bpm = 120.f;

    current_tick_counter = 0;
    samples_since_last_tick = 0;
    samples_per_tick = getSampleRate() * (15.f / 120.f);
    is_touching = false;
    touch_tick_counter = 0;
    touch_fraction = 0.f;

    currentMixL = 0.f;
    currentMixR = 0.f;

    params_.reset(sample_rate_reciprocal);
    updateFilterCoefficients();
    updateRoutingCoefficients();

    flt_l.resetState();
    flt_r.resetState();
  }

  void teardown() override final {}

  void reset() override final {
    current_tick_counter = 0;
    samples_since_last_tick = 0;
    is_touching = false;
    touch_tick_counter = 0;
    touch_fraction = 0.f;

    currentMixL = 0.f;
    currentMixR = 0.f;

    flt_l.resetState();
    flt_r.resetState();
  }

  void setTempo(float bpm) override final {
    if (bpm > 0.f) {
      current_tempo_bpm = bpm;
      samples_per_tick = getSampleRate() * (15.f / bpm);
    }
  }

  // audio processing callbacks
  void process(const float * __restrict in, float * __restrict out, uint32_t frames) override final {
    const int32_t time_div_idx = params_.time_div % 9;
    const float time_division_mult = time_div_multipliers[time_div_idx];
    const float gate_fraction = params_.gate / 100.f;
    const float depth_mix_target = fabsf(params_.depth) / 100.f;
    const float smoothing_alpha = params_.smoothing_alpha;
    const float gain_coef = params_.gain_coef;

    for (const float * out_end = out + frames * 2; out != out_end; in += 2, out += 2) {
      const float in_l = in[0];
      const float in_r = in[1];

      // 1. Calculate Gate State
      bool target_state = is_touching && isGateActive(time_division_mult, gate_fraction);

      // 2. Target and Current Mix
      float targetMix = target_state ? depth_mix_target : 0.f;

      currentMixL += smoothing_alpha * (targetMix - currentMixL);
      if (currentMixL < 0.f) currentMixL = 0.f;
      if (currentMixL > 1.f) currentMixL = 1.f;

      currentMixR += smoothing_alpha * (targetMix - currentMixR);
      if (currentMixR < 0.f) currentMixR = 0.f;
      if (currentMixR > 1.f) currentMixR = 1.f;

      // 3. Run filters
      float lp_l, bp_l, hp_l, ap_l;
      float lp_r, bp_r, hp_r, ap_r;

      flt_l.tick_multimode(in_l, lp_l, bp_l, hp_l, ap_l);
      flt_r.tick_multimode(in_r, lp_r, bp_r, hp_r, ap_r);

      // 4. Compute audible filter result (with cubed gain applied)
      float audible_l = computeAudibleFilter(lp_l, bp_l, hp_l) * gain_coef;
      float audible_r = computeAudibleFilter(lp_r, bp_r, hp_r) * gain_coef;

      // 5. Compute dry signal (mix of raw input and all-pass)
      float dry_l = ap_mix * ap_l + raw_mix * in_l;
      float dry_r = ap_mix * ap_r + raw_mix * in_r;

      // 6. Crossfade dry and wet (audible filter)
      float gain_dry_l = sqrtf(1.f - currentMixL);
      float gain_wet_l = sqrtf(currentMixL);

      float gain_dry_r = sqrtf(1.f - currentMixR);
      float gain_wet_r = sqrtf(currentMixR);

      out[0] = gain_dry_l * dry_l + gain_wet_l * audible_l;
      out[1] = gain_dry_r * dry_r + gain_wet_r * audible_r;
    }
  }

  inline void touchEvent(uint8_t id, uint8_t phase, uint32_t x, uint32_t y) override final {
    (void)id;
    (void)x;
    (void)y;

    if (phase == k_unit_touch_phase_began) {
      is_touching = true;
      if (params_.time_div >= 9) {
        touch_tick_counter = current_tick_counter;
        touch_fraction = samples_since_last_tick / samples_per_tick;
      }
    } else if (phase == k_unit_touch_phase_ended) {
      is_touching = false;
    }
  }

  inline void tempo4ppqnTick(uint32_t counter) override final {
    if (params_.time_div < 9) {
      current_tick_counter = counter;
      samples_since_last_tick = 0;
    }
  }

 private:
  void updateFilterCoefficients() {
    // Note: We always pass LOW_PASS_FILTER here because tick_multimode() derives
    // LP/BP/HP/AP outputs directly from the internal SVF state variables,
    // ignoring the specific filter type mix coefficients set by updateCoefficients().
    flt_l.updateCoefficients(params_.cutoff_hz, params_.q, SvfLinearTrapOptimised2::LOW_PASS_FILTER, getSampleRate());
    flt_r.updateCoefficients(params_.cutoff_hz, params_.q, SvfLinearTrapOptimised2::LOW_PASS_FILTER, getSampleRate());
  }

  /**
   * @brief Updates the internal mixing coefficients for the active filter type and routing.
   *
   * Calculates the crossfade values (morph_gain_low, morph_gain_band, morph_gain_high)
   * between lowpass, bandpass, and highpass based on the current `type` parameter.
   * Also updates the raw and allpass mix coefficients based on the cutoff frequency.
   */
  void updateRoutingCoefficients() {
    if (params_.type > 0 && params_.type < 500) {
      float mix_val = params_.type / 500.f;
      morph_gain_low = sqrtf(1.f - mix_val);
      morph_gain_band = sqrtf(mix_val);
      morph_gain_high = 0.f;
    } else if (params_.type > 500 && params_.type < 1000) {
      float mix_val = (params_.type - 500.f) / 500.f;
      morph_gain_low = 0.f;
      morph_gain_band = sqrtf(1.f - mix_val);
      morph_gain_high = sqrtf(mix_val);
    } else if (params_.type == 0) {
      morph_gain_low = 1.f;
      morph_gain_band = 0.f;
      morph_gain_high = 0.f;
    } else if (params_.type == 500) {
      morph_gain_low = 0.f;
      morph_gain_band = 1.f;
      morph_gain_high = 0.f;
    } else {
      morph_gain_low = 0.f;
      morph_gain_band = 0.f;
      morph_gain_high = 1.f;
    }

    if (params_.cutoff_hz >= 5000.f) {
      ap_mix = 0.f;
      raw_mix = 1.f;
    } else if (params_.cutoff_hz > 2000.f) {
      float t = (params_.cutoff_hz - 2000.f) / 3000.f;
      raw_mix = t;
      ap_mix = 1.f - t;
    } else {
      ap_mix = 1.f;
      raw_mix = 0.f;
    }
  }

  /**
   * @brief Determines whether the gate should be open or closed for the current sample.
   *
   * Calculates the current phase within the rhythmic time division cycle. In tempo-synced
   * modes, this is based on absolute transport position. In free modes, it's relative
   * to the time the user touched the pad.
   *
   * @param time_division_mult Multiplier for the current time division.
   * @param gate_fraction The active portion of the cycle (duty cycle).
   * @return true if the gate is active (open) for this sample.
   */
  inline bool isGateActive(float time_division_mult, float gate_fraction) {
    samples_since_last_tick++;

    uint32_t tick_diff = 0;
    float frac_diff = samples_since_last_tick / samples_per_tick;

    if (params_.time_div >= 9) {
      // Free Mode: Offset from touch start
      tick_diff = current_tick_counter - touch_tick_counter;
      frac_diff -= touch_fraction;
    } else {
      // Tempo-Synced Mode: Absolute position
      tick_diff = current_tick_counter % 24;
    }

    float relative_pos = (float)tick_diff + frac_diff;
    if (relative_pos < 0.f) relative_pos = 0.f;  // safeguard against float inaccuracy

    float phase = relative_pos * time_division_mult;
    phase = phase - (uint32_t)phase;

    return (params_.depth >= 0.f) ? (phase >= (1.0f - gate_fraction)) : (phase < gate_fraction);
  }

  /**
   * @brief Computes the mixed output of the state variable filter.
   *
   * Blends the outputs of the underlying lowpass, bandpass, and highpass filter
   * states using the morphing coefficients calculated in `updateRoutingCoefficients`.
   *
   * @param lp The current lowpass filter output.
   * @param bp The current bandpass filter output.
   * @param hp The current highpass filter output.
   * @return The resulting mixed filter output.
   */
  inline float computeAudibleFilter(float lp, float bp, float hp) const {
    if (params_.type == 0) {
      return lp;
    } else if (params_.type < 500) {
      return morph_gain_low * lp + morph_gain_band * bp;
    } else if (params_.type == 500) {
      return bp;
    } else if (params_.type < 1000) {
      return morph_gain_band * bp + morph_gain_high * hp;
    } else {
      return hp;
    }
  }

  SvfLinearTrapOptimised2 flt_l;
  SvfLinearTrapOptimised2 flt_r;

  float sample_rate_reciprocal;
  float current_tempo_bpm;

  uint32_t current_tick_counter;
  int32_t samples_since_last_tick;
  float samples_per_tick;
  bool is_touching;

  uint32_t touch_tick_counter;
  float touch_fraction;

  float currentMixL;
  float currentMixR;

  float morph_gain_low;
  float morph_gain_band;
  float morph_gain_high;
  float ap_mix;
  float raw_mix;

  Params params_;
};
