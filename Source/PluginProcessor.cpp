/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "PluginCommon.h"
#include <format>

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout XFadeEQAudioProcessor::createParameterLayout()
{
    // パラメータ構成の定義
    // layoutに必要なパラメータを追加していくと `new juce::GenericAudioProcessorEditor()` でマルっと簡易UI作ってくれる
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // EQ切り替えXFader
    layout.add(std::make_unique<juce::AudioParameterFloat>("xFader", "XFader <-1:C, 0:A, 1:B>", -1.0f, 1.0f, 0.0f));

    // バンド数×EQ数 10×3分layout.add()
    for (int i = 0; i < PluginCommon::numEqs; i++)
    {
        for (int j = 0; j < PluginCommon::numBands; j++)
        {
            auto id = std::format("{}{}", PluginCommon::paramIds[j], PluginCommon::suffixes[i]);
            auto name = std::format("{}{}", PluginCommon::paramNames[j], PluginCommon::subscripts[i]);
            layout.add(std::make_unique<juce::AudioParameterFloat>(id, name, -1.0f * PluginCommon::gainRangeDb, PluginCommon::gainRangeDb, 0.0f));
        }
    }

    return layout;
}

//==============================================================================
XFadeEQAudioProcessor::XFadeEQAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

XFadeEQAudioProcessor::~XFadeEQAudioProcessor()
{
}

//==============================================================================
const juce::String XFadeEQAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool XFadeEQAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool XFadeEQAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool XFadeEQAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double XFadeEQAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int XFadeEQAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int XFadeEQAudioProcessor::getCurrentProgram()
{
    return 0;
}

void XFadeEQAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String XFadeEQAudioProcessor::getProgramName (int index)
{
    return {};
}

void XFadeEQAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void XFadeEQAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // 信号処理のスペック
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = (juce::uint32) samplesPerBlock;
    spec.numChannels = 1;

    // スペックの適用
    for (int i = 0; i < PluginCommon::numBands; ++i)
    {
        leftFiltersA[i].prepare(spec);
        rightFiltersA[i].prepare(spec);
        leftFiltersB[i].prepare(spec);
        rightFiltersB[i].prepare(spec);
        leftFiltersC[i].prepare(spec);
        rightFiltersC[i].prepare(spec);
    }

    // パラレルプロセッシングのためのバッファ
    tempBuffer.setSize(getTotalNumOutputChannels(), samplesPerBlock);
    dryInBuffer.setSize(getTotalNumOutputChannels(), samplesPerBlock);

    // パラメータ→フィルタの初期同期
    updateFilters();
}

void XFadeEQAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void XFadeEQAudioProcessor::updateFiltersRoutine(FilterArray& leftFilters, FilterArray& rightFilters, std::string_view suffix )
{
    auto sampleRate = getSampleRate();

    for (int i = 0; i < PluginCommon::numBands; ++i)
    {
        // パラメータの取得
        auto id = std::format("{}{}", PluginCommon::paramIds[i], suffix);
        auto gainCtx = apvts.getRawParameterValue(id);
        auto gain = gainCtx->load();

        // 係数計算
        auto coeffs = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, PluginCommon::freqs[i], PluginCommon::filterQ, juce::Decibels::decibelsToGain(gain));

        // 係数セット
        leftFilters[i].coefficients = coeffs;
        rightFilters[i].coefficients = coeffs;
    }
}

void XFadeEQAudioProcessor::updateFilters()
{
    updateFiltersRoutine(leftFiltersA, rightFiltersA, PluginCommon::suffixes[0]);
    updateFiltersRoutine(leftFiltersB, rightFiltersB, PluginCommon::suffixes[1]);
    updateFiltersRoutine(leftFiltersC, rightFiltersC, PluginCommon::suffixes[2]);
}



#ifndef JucePlugin_PreferredChannelConfigurations
bool XFadeEQAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void XFadeEQAudioProcessor::processAndAdd(FilterArray& filters, float weight, const juce::AudioBuffer<float>& dryInBuffer, juce::AudioBuffer<float>& buffer, int channel)
{
    // process()およびProcessContextReplacingが破壊的処理
    // なので、dry音をtemporaryに処理して加算する関数を用意したがDSPライブラリにある気がする
    // 探したけどちょうどいいのが見当たらなかった

    // dryInBufferをtempBufferへコピー(必要なchだけ)
    tempBuffer.copyFrom(channel, 0, dryInBuffer, channel, 0, dryInBuffer.getNumSamples());

    // フィルタ適用
    juce::dsp::AudioBlock<float> block(tempBuffer);
    block = block.getSingleChannelBlock(channel);
    juce::dsp::ProcessContextReplacing<float> context(block);
    for (int i = 0; i < PluginCommon::numBands; ++i)
    {
        filters[i].process(context);
    }
    
    // フィルタ後のtempBufferをbufferへ重み付け加算
    buffer.addFrom(channel, 0, tempBuffer, channel, 0, buffer.getNumSamples(), weight);
}

void XFadeEQAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // つまみ反映
    updateFilters();
    // EQ切り替え
    float xFader = (float) (apvts.getRawParameterValue("xFader")->load());
    float weightA = 1.0f - std::abs(xFader);
    float weightB = (xFader > 0.0f) ? std::abs(xFader) : 0.0f;
    float weightC = (xFader < 0.0f) ? std::abs(xFader) : 0.0f;

    // ドライ音を取っておく
    dryInBuffer.makeCopyOf(buffer);
    // 出力バッファをクリア
    buffer.clear();

    if (totalNumInputChannels >= 1)
    {
        // 左フィルタ適用
        processAndAdd(leftFiltersA, weightA, dryInBuffer, buffer, 0);
        processAndAdd(leftFiltersB, weightB, dryInBuffer, buffer, 0);
        processAndAdd(leftFiltersC, weightC, dryInBuffer, buffer, 0);
    }

    if (totalNumInputChannels >= 2)
    {
        // 右フィルタ適用
        processAndAdd(rightFiltersA, weightA, dryInBuffer, buffer, 1);
        processAndAdd(rightFiltersB, weightB, dryInBuffer, buffer, 1);
        processAndAdd(rightFiltersC, weightC, dryInBuffer, buffer, 1);
    }
}


//==============================================================================
bool XFadeEQAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* XFadeEQAudioProcessor::createEditor()
{
    // 汎用エディタ生成
    return new juce::GenericAudioProcessorEditor (*this);

    //return new XFadeEQAudioProcessorEditor(*this, apvts);
}

//==============================================================================
void XFadeEQAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void XFadeEQAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new XFadeEQAudioProcessor();
}
