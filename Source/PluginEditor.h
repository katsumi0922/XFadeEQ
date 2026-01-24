/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class XFadeEQAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    XFadeEQAudioProcessorEditor (XFadeEQAudioProcessor&, juce::AudioProcessorValueTreeState& layout);
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
    juce::Slider eqSlider;
    std::unique_ptr<SliderAttachment> eqAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (XFadeEQAudioProcessorEditor)
};
