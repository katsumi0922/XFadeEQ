/*
  ==============================================================================

    PluginCommon.h
    Created: 27 Jan 2026 7:26:37am
    Author:  katsumi0922

  ==============================================================================
*/

#pragma once
#include <array>

namespace PluginCommon
{
    inline constexpr std::array<float, 10> freqs = { 31.25f, 62.5f, 125.0f, 250.0f, 500.0f, 1000.0f, 2000.0f, 4000.0f, 8000.0f, 16000.0f };
    inline constexpr std::array<std::string_view, 10> paramIds = { "g31p25", "g62p5", "g125", "g250", "g500", "g1k", "g2k", "g4k", "g8k", "g16k" };
    inline constexpr std::array<std::string_view, 10> paramNames = { "31.25 Hz", "62.5 Hz", "125 Hz", "250 Hz", "500 Hz", "1 kHz", "2 kHz", "4 kHz", "8 kHz", "16 kHz" };
    inline constexpr std::array<std::string_view, 3> suffixes = { "_A", "_B", "_C" };
    inline constexpr std::array<std::string_view, 3> subscripts = { " (A)", " (B)", " (C)" };
}