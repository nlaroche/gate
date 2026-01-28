#pragma once

namespace ParameterIDs
{
    // Pattern Parameters
    inline constexpr const char* pattern       = "pattern";        // 0-7 preset patterns
    inline constexpr const char* steps         = "steps";          // 4-16 steps
    inline constexpr const char* rate          = "rate";           // 1/1 to 1/32 note divisions

    // Step Parameters (encoded as single value for simplicity)
    inline constexpr const char* stepData      = "stepData";       // 16-bit encoded step on/off

    // Envelope Parameters
    inline constexpr const char* attack        = "attack";         // 0.1-100ms
    inline constexpr const char* hold          = "hold";           // 0-100%
    inline constexpr const char* release       = "release";        // 0.1-500ms
    inline constexpr const char* curve         = "curve";          // -100 to +100 (log to exp)

    // Modulation Parameters
    inline constexpr const char* swing         = "swing";          // 0-100%
    inline constexpr const char* humanize      = "humanize";       // 0-100% timing randomization
    inline constexpr const char* velocity      = "velocity";       // 0-100% velocity sensitivity

    // Mix Parameters
    inline constexpr const char* depth         = "depth";          // 0-100%
    inline constexpr const char* mix           = "mix";            // 0-100%
    inline constexpr const char* output        = "output";         // -24 to +12 dB

    // State
    inline constexpr const char* bypass        = "bypass";         // Toggle
}
