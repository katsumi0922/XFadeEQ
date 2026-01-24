/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
XFadeEQAudioProcessorEditor::XFadeEQAudioProcessorEditor (XFadeEQAudioProcessor& p, juce::AudioProcessorValueTreeState& apvts)
    : AudioProcessorEditor (&p), valueTreeState(apvts), audioProcessor (p)
{
    addAndMakeVisible(eqSlider);
    eqAttachment.reset(new SliderAttachment(valueTreeState, "g500_A", eqSlider));

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
}

XFadeEQAudioProcessorEditor::~XFadeEQAudioProcessorEditor()
{
}

//==============================================================================
void XFadeEQAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    //g.setColour (juce::Colours::white);
    //g.setFont (juce::FontOptions (15.0f));
    //g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void XFadeEQAudioProcessorEditor::resized()
{
    eqSlider.setBounds(5, 5, 250, 30);
}
