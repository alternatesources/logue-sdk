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
 *  File: header.c
 *
 *  NTS-3 kaoss pad kit generic effect unit header definition for filter gate
 *
 */

#include "unit_genericfx.h"  // Note: Include base definitions for genericfx units

// ---- Unit header definition  --------------------------------------------------------------------

const __unit_header genericfx_unit_header_t unit_header = {
    .common = {
        .header_size = sizeof(genericfx_unit_header_t),            // Size of this header. Leave as is.
        .target = UNIT_TARGET_PLATFORM | k_unit_module_genericfx,  // Target platform and module pair for this unit
        .api = UNIT_API_VERSION,                                   // API version for which unit was built. See runtime.h
        .dev_id = 0x73726373U,                                     // Developer ID (srcs)
        .unit_id = 0x01U,                                          // ID for this unit.
        .version = 0x00010000U,                                    // v1.0.0
        .name = "filter gate",                                     // Name for this unit, will be displayed on device
        .num_params = 8,                                           // Number of valid parameter descriptors (8 as per NTS-3 spec)

        .params = {
            // Format: min, max, center (unused), default, type, frac. bits, frac. mode, <reserved>, name

            // 0. TIME: 0 to 17 (1/2, 1/4d, 1/4, 1/8d, 1/8, 1/16d, 1/16, 1/32d, 1/32, 1/2free, 1/4dfree, 1/4free, 1/8dfree, 1/8free, 1/16dfree, 1/16free, 1/32dfree, 1/32free)
            {0, 17, 0, 6, k_unit_param_type_strings, 0, 0, 0, {"TIME"}},

            // 1. CUTOFF: 20Hz to 20000Hz
            {20, 20000, 20, 20000, k_unit_param_type_hertz, 0, 0, 0, {"CUTOFF"}},

            // 2. RESO: 50 to 30000 (divided by 100 -> Q = 0.50 to 300.00)
            {50, 30000, 50, 50, k_unit_param_type_none, 2, 1, 0, {"RESO"}},

            // 3. TYPE: 0 to 1000 (mix of LP -> BP -> HP)
            {0, 1000, 0, 0, k_unit_param_type_none, 0, 0, 0, {"TYPE"}},

            // 4. GAIN: 0 to 1000 (cubed gain multiplier)
            {0, 1000, 0, 1000, k_unit_param_type_none, 0, 0, 0, {"GAIN"}},

            // 5. SMOOTH: 0 to 500 (representing 0.0 to 50.0 ms)
            {0, 500, 0, 20, k_unit_param_type_msec, 1, 1, 0, {"SMOOTH"}},

            // 6. GATE: 0 to 100 (duty cycle percent)
            {0, 100, 0, 50, k_unit_param_type_percent, 0, 0, 0, {"GATE"}},

            // 7. DEPTH: -100 to 100 (dry/wet parameter type)
            {-100, 100, 0, 100, k_unit_param_type_drywet, 0, 0, 0, {"DEPTH"}}},
    },
    .default_mappings = {// Format: assign, curve, curve polarity, min, max, default value

                         // TIME not assigned to XY
                         {k_genericfx_param_assign_none, k_genericfx_curve_log, k_genericfx_curve_unipolar, 0, 8, 6},

                         // CUTOFF mapped to X axis with exponential curve
                         {k_genericfx_param_assign_x, k_genericfx_curve_exp, k_genericfx_curve_unipolar, 20, 20000, 20000},

                         // RESO mapped to Y axis with exponential curve
                         {k_genericfx_param_assign_y, k_genericfx_curve_exp, k_genericfx_curve_unipolar, 50, 1000, 70},

                         // TYPE not assigned to XY
                         {k_genericfx_param_assign_none, k_genericfx_curve_linear, k_genericfx_curve_unipolar, 0, 1000, 0},

                         // GAIN not assigned to XY
                         {k_genericfx_param_assign_none, k_genericfx_curve_linear, k_genericfx_curve_unipolar, 0, 1000, 1000},

                         // SMOOTH not assigned to XY
                         {k_genericfx_param_assign_none, k_genericfx_curve_exp, k_genericfx_curve_unipolar, 0, 500, 20},

                         // GATE not assigned to XY by default
                         {k_genericfx_param_assign_none, k_genericfx_curve_linear, k_genericfx_curve_unipolar, 0, 100, 50},

                         // DEPTH assigned to depth control
                         {k_genericfx_param_assign_depth, k_genericfx_curve_linear, k_genericfx_curve_unipolar, 0, 100, 100}}};
