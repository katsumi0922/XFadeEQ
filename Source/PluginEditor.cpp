/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <format>

//==============================================================================
XFadeEQAudioProcessorEditor::XFadeEQAudioProcessorEditor (XFadeEQAudioProcessor& p, juce::AudioProcessorValueTreeState& apvts)
    : AudioProcessorEditor (&p), valueTreeState(apvts), audioProcessor (p)
{
    //eqSlider.setSliderStyle(juce::Slider::LinearVertical);
    //eqSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    //// gainSlider.setRange(0.0f, 1.0f, 0.0f); // APVTS側のrangeに合わせる（or ここは不要）
    //// gainSlider.setSkewFactorFromMidPoint(0.5f); // 必要なら（対数っぽい操作感）
    //addAndMakeVisible(eqSlider);
    //eqAttachment.reset(new SliderAttachment(valueTreeState, "g500_A", eqSlider));

    // 10band*3段 分ループしてeqSlidersを用意
    for (int i = 0; i < PluginCommon::numEqs; ++i)
    {
        for (int j = 0; j < PluginCommon::numBands; ++j)
        {
            int index = i * PluginCommon::numBands + j;

            auto& slider = eqSliders[index];
            slider.setSliderStyle(juce::Slider::LinearVertical);
            slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
            addAndMakeVisible(slider);

            // APTVSと結びつけるためIDの生成
            auto paramId = std::format("{}{}", PluginCommon::paramIds[j], PluginCommon::suffixes[i]);
            // アタッチメント作成
            eqAttachments[index] = std::make_unique<SliderAttachment>(valueTreeState, paramId, slider);
        }
    }

    //xFaderSliderを用意
    xFaderSlider.setSliderStyle(juce::Slider::LinearVertical);
    xFaderSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(xFaderSlider);
    xFaderAttachment.reset(new SliderAttachment(valueTreeState, "xFader", xFaderSlider));


    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (550, 400);
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
    //eqSlider.setBounds(5, 5, 30, 250);
    
    // 仮のスライダーサイズ
    int sliderWidth = 40;
    int sliderHeight = 120;

    // 10band*3段 分ループ
    for (int i = 0; i < PluginCommon::numEqs; ++i)
    {
        for (int j = 0; j < PluginCommon::numBands; ++j)
        {
            int x = 10 + j * sliderWidth;
            int y = 10;
            // 上からB,A,Cと並ぶよう調整
            if (i == 0) y += (sliderHeight + 5);
            else if (i == 2) y += (sliderHeight + 5) * 2;

            // EQ用スライダーを配置
            eqSliders[i * PluginCommon::numBands + j].setBounds(x, y, sliderWidth - 5, sliderHeight);
        }
    }
    // xFader用スライダーを配置
    xFaderSlider.setBounds(10 + PluginCommon::numBands * sliderWidth, 10, sliderWidth - 5, (sliderHeight + 5) * PluginCommon::numEqs);
}
