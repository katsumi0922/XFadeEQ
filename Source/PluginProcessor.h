/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <juce_dsp/juce_dsp.h>
#include <string_view>
#include "PluginCommon.h"
#include <array>

//==============================================================================
/**
*/
class XFadeEQAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    XFadeEQAudioProcessor();
    ~XFadeEQAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // パラメータレイアウト作成
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState apvts { *this, nullptr, "Parameters", createParameterLayout() };

private:
    // エイリアス
    using Filter = juce::dsp::IIR::Filter<float>;
    using FilterArray = std::array<Filter, PluginCommon::numBands>;

    // 処理チェイン宣言
    FilterArray leftFiltersA, rightFiltersA;
    FilterArray leftFiltersB, rightFiltersB;
    FilterArray leftFiltersC, rightFiltersC;

    // パラレルプロセッシングのためのバッファ
    juce::AudioBuffer<float> tempBuffer;
    juce::AudioBuffer<float> dryInBuffer;

    void updateFilters();
    void updateFiltersRoutine(FilterArray& leftFilters, FilterArray& rightFilters, std::string_view suffix);
    void processAndAdd(FilterArray& filters, float weight, const juce::AudioBuffer<float>& dryInBuffer, juce::AudioBuffer<float>& buffer, int channel);

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (XFadeEQAudioProcessor)
};
