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
  inline constexpr int numBands = 10;
  inline constexpr int numEqs = 3;
  inline constexpr float filterQ = 1.4f;
  inline constexpr float gainRangeDb = 12.0f;

  inline constexpr std::array<float, numBands> freqs = { 31.25f, 62.5f, 125.0f, 250.0f, 500.0f, 1000.0f, 2000.0f, 4000.0f, 8000.0f, 16000.0f };
  inline constexpr std::array<std::string_view, numBands> paramIds = { "g31p25", "g62p5", "g125", "g250", "g500", "g1k", "g2k", "g4k", "g8k", "g16k" };
  inline constexpr std::array<std::string_view, numBands> paramNames = { "31.25 Hz", "62.5 Hz", "125 Hz", "250 Hz", "500 Hz", "1 kHz", "2 kHz", "4 kHz", "8 kHz", "16 kHz" };
  inline constexpr std::array<std::string_view, numEqs> suffixes = { "_A", "_B", "_C" };
  inline constexpr std::array<std::string_view, numEqs> subscripts = { " (A)", " (B)", " (C)" };
}