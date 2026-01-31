/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "PluginCommon.h"

//==============================================================================
/**
*/
class XFadeEQAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    XFadeEQAudioProcessorEditor (XFadeEQAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~XFadeEQAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    XFadeEQAudioProcessor& audioProcessor;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    juce::AudioProcessorValueTreeState& valueTreeState;

    // 10band*3段 分のSliderとAttachmentを配列で定義
    static constexpr int numEqSliders = PluginCommon::numEqs * PluginCommon::numBands;
    std::array<juce::Slider, numEqSliders> eqSliders;
    std::array<std::unique_ptr<SliderAttachment>, numEqSliders> eqAttachments;

    // xFader用
    juce::Slider xFaderSlider;
    std::unique_ptr<SliderAttachment> xFaderAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (XFadeEQAudioProcessorEditor)
};
