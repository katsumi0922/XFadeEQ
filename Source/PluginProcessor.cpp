/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

// パラメータ構成の定義
juce::AudioProcessorValueTreeState::ParameterLayout XFadeEQAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "g31p25", 1 }, "31.25 Hz", -12.0f, 12.0f, 0.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "g62p5",  1 }, "62.5 Hz",  -12.0f, 12.0f, 0.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "g125",   1 }, "125 Hz",   -12.0f, 12.0f, 0.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "g250",   1 }, "250 Hz",   -12.0f, 12.0f, 0.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "g500",   1 }, "500 Hz",   -12.0f, 12.0f, 0.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "g1k",    1 }, "1 kHz",    -12.0f, 12.0f, 0.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "g2k",    1 }, "2 kHz",    -12.0f, 12.0f, 0.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "g4k",    1 }, "4 kHz",    -12.0f, 12.0f, 0.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "g8k",    1 }, "8 kHz",    -12.0f, 12.0f, 0.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "g16k",   1 }, "16 kHz",   -12.0f, 12.0f, 0.0f));

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
    leftChain.prepare (spec);
    rightChain.prepare (spec);

    // パラメータ→フィルタの初期同期
    updateFilters();
}

void XFadeEQAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void XFadeEQAudioProcessor::updateFilters()
{
    // パラメータの取得
    auto g31p25 = apvts.getRawPar
        ameterValue ("g31p25")->load();
    auto g62p5  = apvts.getRawParameterValue ("g62p5")->load();
    auto g125   = apvts.getRawParameterValue ("g125")->load();
    auto g250   = apvts.getRawParameterValue ("g250")->load();
    auto g500   = apvts.getRawParameterValue ("g500")->load();
    auto g1k    = apvts.getRawParameterValue ("g1k")->load();
    auto g2k    = apvts.getRawParameterValue ("g2k")->load();
    auto g4k    = apvts.getRawParameterValue ("g4k")->load();
    auto g8k    = apvts.getRawParameterValue ("g8k")->load();
    auto g16k   = apvts.getRawParameterValue ("g16k")->load();

    auto sampleRate = getSampleRate();
    const float Q = 1.4f;

    // 各バンドの係数計算
    auto c1  = juce::dsp::IIR::Coefficients<float>::makePeakFilter (samp　leRate, 31.25f, Q, juce::Decibels::decibelsToGain (g31p25));
    auto c2  = juce::dsp::IIR::Coefficients<float>::makePeakFilter (sampleRate, 62.5f,  Q, juce::Decibels::decibelsToGain (g62p5));
    auto c3  = juce::dsp::IIR::Coefficients<float>::makePeakFilter (sampleRate, 125.0f, Q, juce::Decibels::decibelsToGain (g125));
    auto c4  = juce::dsp::IIR::Coefficients<float>::makePeakFilter (sampleRate, 250.0f, Q, juce::Decibels::decibelsToGain (g250));
    auto c5  = juce::dsp::IIR::Coefficients<float>::makePeakFilter (sampleRate, 500.0f, Q, juce::Decibels::decibelsToGain (g500));
    auto c6  = juce::dsp::IIR::Coefficients<float>::makePeakFilter (sampleRate, 1000.0f, Q, juce::Decibels::decibelsToGain (g1k));
    auto c7  = juce::dsp::IIR::Coefficients<float>::makePeakFilter (sampleRate, 2000.0f, Q, juce::Decibels::decibelsToGain (g2k));
    auto c8  = juce::dsp::IIR::Coefficients<float>::makePeakFilter (sampleRate, 4000.0f, Q, juce::Decibels::decibelsToGain (g4k));
    auto c9  = juce::dsp::IIR::Coefficients<float>::makePeakFilter (sampleRate, 8000.0f, Q, juce::Decibels::decibelsToGain (g8k));
    auto c10 = juce::dsp::IIR::Coefficients<float>::makePeakFilter (sampleRate, 16000.0f, Q, juce::Decibels::decibelsToGain (g16k));

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

    // dsp用に準備
    juce::dsp::AudioBlock<float> block (buffer);

    if (totalNumInputChannels >= 1)
    {
        // 左フィルタ適用
        auto leftBlock = block.getSingleChannelBlock (0);
        juce::dsp::ProcessContextReplacing<float> leftContext (leftBlock);
        leftChain.process (leftContext);
    }

    if (totalNumInputChannels >= 2)
    {
        // 右フィルタ適用
        auto rightBlock = block.getSingleChannelBlock (1);
        juce::dsp::ProcessContextReplacing<float> rightContext (rightBlock);
        rightChain.process (rightContext);
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
