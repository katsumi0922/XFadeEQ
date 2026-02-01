/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <format>
#include "../JuceLibraryCode/JuceHeader.h"  // 画像バイナリ参照用

//==============================================================================
XFadeEQAudioProcessorEditor::XFadeEQAudioProcessorEditor (XFadeEQAudioProcessor& p, juce::AudioProcessorValueTreeState& apvts)
    : AudioProcessorEditor (&p), valueTreeState(apvts), audioProcessor (p)
{
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

    // xFaderSliderを用意
    xFaderSlider.setSliderStyle(juce::Slider::LinearVertical);
    xFaderSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(xFaderSlider);
    xFaderAttachment.reset(new SliderAttachment(valueTreeState, "xFader", xFaderSlider));

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (550, 410);
}

XFadeEQAudioProcessorEditor::~XFadeEQAudioProcessorEditor()
{
}

//==============================================================================
void XFadeEQAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    g.setFont(juce::FontOptions(15.0f));

    for (int i = 0; i < PluginCommon::numEqs; i++)
    {
        int y = 10;
        // 上からB,A,Cと並ぶよう調整
        if (i == 0) y += (sliderHeight + 5);
        else if (i == 2) y += (sliderHeight + 5) * 2;

        // EQ_A/B/Cのラベルを配置
        g.drawFittedText(std::format("EQ{}", PluginCommon::suffixes[i]), 10, y, sliderWidth + 10, sliderHeight, juce::Justification::centred, 1);
    }

    for (int j = 0; j < PluginCommon::numBands; j++)
    {
        int x = 50 + (j * sliderWidth);

        // 周波数ラベルを配置
        g.drawFittedText(std::string(PluginCommon::freqLabels[j]), x, 10 + (sliderHeight + 5) * PluginCommon::numEqs, sliderWidth, 5, juce::Justification::centred, 1);
    }

    // xFaderの図を配置
    juce::Image xFaderDiagram = juce::ImageCache::getFromMemory(BinaryData::xfader_diagram_drawio_png, BinaryData::xfader_diagram_drawio_pngSize);
    g.drawImage(xFaderDiagram,
        50 + PluginCommon::numBands * sliderWidth,
        10 + (sliderHeight + 5) * (PluginCommon::numEqs - 1) + sliderHeight / 2,
        100,
        60,
        0,
        0,
        xFaderDiagram.getWidth(), 
        xFaderDiagram.getHeight()
    );
}

void XFadeEQAudioProcessorEditor::resized()
{
    // 10band*3段 分ループ
    for (int i = 0; i < PluginCommon::numEqs; ++i)
    {
        for (int j = 0; j < PluginCommon::numBands; ++j)
        {
            int x = 50 + (j * sliderWidth);
            int y = 10;
            // 上からB,A,Cと並ぶよう調整
            if (i == 0) y += (sliderHeight + 5);
            else if (i == 2) y += (sliderHeight + 5) * 2;

            // EQ用スライダーを配置
            eqSliders[i * PluginCommon::numBands + j].setBounds(x, y, sliderWidth, sliderHeight);
        }
    }

    // xFader用スライダーを配置
    xFaderSlider.setBounds(50 + PluginCommon::numBands * sliderWidth + 30, 10 + sliderHeight / 2, sliderWidth, (sliderHeight + 5) * (PluginCommon::numEqs - 1));
}
