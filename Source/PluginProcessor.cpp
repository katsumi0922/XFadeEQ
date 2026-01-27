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
    for (int i = 0; i < PluginCommon::suffixes.size(); i++)
    {
        for (int j = 0; j < PluginCommon::freqs.size(); j++)
        {
            //auto id = std::string(PluginCommon::paramIds[j]) + suffixes[i];
            auto id = std::format("{}{}", PluginCommon::paramIds[j], PluginCommon::suffixes[i]);
            //auto name = paramNames[j] + subscripts[i];
            auto name = std::format("{}{}", PluginCommon::paramNames[j], PluginCommon::subscripts[i]);
            layout.add(std::make_unique<juce::AudioParameterFloat>(id, name, -12.0f, 12.0f, 0.0f));
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
    leftChainA.prepare(spec);   rightChainA.prepare(spec);
    leftChainB.prepare(spec);   rightChainB.prepare(spec);
    leftChainC.prepare(spec);   rightChainC.prepare(spec);

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

void XFadeEQAudioProcessor::updateFiltersRoutine(Chain& leftChain, Chain& rightChain, std::string_view suffix )
{
    // ★もっとスマートなやり方がある気がするけど思いつかない ベクターを活かせてない
    // 係数セットのところがtemplate引数なのでループ化しにくい 本来なら設計をちゃんとした上で実装するべきで、エイヤでやってるのでしょうがない
    // 多分、今回のような同じ種類のプロセッサ(filter)を10個並べてるようなケースではProcessorChain使わないほうが良い
    
    // パラメータの取得
    auto g31p25 = apvts.getRawParameterValue (std::format("g31p25{}", suffix))->load();
    auto g62p5  = apvts.getRawParameterValue (std::format("g62p5{}", suffix))->load();
    auto g125   = apvts.getRawParameterValue (std::format("g125{}", suffix))->load();
    auto g250   = apvts.getRawParameterValue (std::format("g250{}", suffix))->load();
    auto g500   = apvts.getRawParameterValue (std::format("g500{}", suffix))->load();
    auto g1k    = apvts.getRawParameterValue (std::format("g1k{}", suffix))->load();
    auto g2k    = apvts.getRawParameterValue (std::format("g2k{}", suffix))->load();
    auto g4k    = apvts.getRawParameterValue (std::format("g4k{}", suffix))->load();
    auto g8k    = apvts.getRawParameterValue (std::format("g8k{}", suffix))->load();
    auto g16k   = apvts.getRawParameterValue (std::format("g16k{}", suffix))->load();

    auto sampleRate = getSampleRate();
    const float Q = 1.4f;

    // 各バンドの係数計算
    auto c1 = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, 31.25f, Q, juce::Decibels::decibelsToGain(g31p25));
    auto c2 = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, 62.5f, Q, juce::Decibels::decibelsToGain(g62p5));
    auto c3 = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, 125.0f, Q, juce::Decibels::decibelsToGain(g125));
    auto c4 = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, 250.0f, Q, juce::Decibels::decibelsToGain(g250));
    auto c5 = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, 500.0f, Q, juce::Decibels::decibelsToGain(g500));
    auto c6 = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, 1000.0f, Q, juce::Decibels::decibelsToGain(g1k));
    auto c7 = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, 2000.0f, Q, juce::Decibels::decibelsToGain(g2k));
    auto c8 = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, 4000.0f, Q, juce::Decibels::decibelsToGain(g4k));
    auto c9 = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, 8000.0f, Q, juce::Decibels::decibelsToGain(g8k));
    auto c10 = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, 16000.0f, Q, juce::Decibels::decibelsToGain(g16k));

    // 各フィルタへの係数セット
    leftChain.get<Band1>().coefficients = c1;  rightChain.get<Band1>().coefficients = c1;
    leftChain.get<Band2>().coefficients = c2;  rightChain.get<Band2>().coefficients = c2;
    leftChain.get<Band3>().coefficients = c3;  rightChain.get<Band3>().coefficients = c3;
    leftChain.get<Band4>().coefficients = c4;  rightChain.get<Band4>().coefficients = c4;
    leftChain.get<Band5>().coefficients = c5;  rightChain.get<Band5>().coefficients = c5;
    leftChain.get<Band6>().coefficients = c6;  rightChain.get<Band6>().coefficients = c6;
    leftChain.get<Band7>().coefficients = c7;  rightChain.get<Band7>().coefficients = c7;
    leftChain.get<Band8>().coefficients = c8;  rightChain.get<Band8>().coefficients = c8;
    leftChain.get<Band9>().coefficients = c9;  rightChain.get<Band9>().coefficients = c9;
    leftChain.get<Band10>().coefficients = c10; rightChain.get<Band10>().coefficients = c10;
}

void XFadeEQAudioProcessor::updateFilters()
{
    updateFiltersRoutine(leftChainA, rightChainA, PluginCommon::suffixes[0]);
    updateFiltersRoutine(leftChainB, rightChainB, PluginCommon::suffixes[1]);
    updateFiltersRoutine(leftChainC, rightChainC, PluginCommon::suffixes[2]);
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

void XFadeEQAudioProcessor::processAndAdd(Chain& chain, float weight, const juce::AudioBuffer<float>& dryInBuffer, juce::AudioBuffer<float>& buffer, int channel)
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
    chain.process(context);
    
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
        processAndAdd(leftChainA, weightA, dryInBuffer, buffer, 0);
        processAndAdd(leftChainB, weightB, dryInBuffer, buffer, 0);
        processAndAdd(leftChainC, weightC, dryInBuffer, buffer, 0);
    }

    if (totalNumInputChannels >= 2)
    {
        // 右フィルタ適用
        processAndAdd(rightChainA, weightA, dryInBuffer, buffer, 1);
        processAndAdd(rightChainB, weightB, dryInBuffer, buffer, 1);
        processAndAdd(rightChainC, weightC, dryInBuffer, buffer, 1);
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
