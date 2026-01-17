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

    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "LowGainDb", 1 }, "L", -12.0f, 12.0f, 0.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "MidGainDb", 1 }, "M", -12.0f, 12.0f, 0.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "HighGainDb", 1 }, "H", -12.0f, 12.0f, 0.0f));

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
    auto lowGain = apvts.getRawParameterValue ("LowGainDb")->load();
    auto midGain = apvts.getRawParameterValue ("MidGainDb")->load();
    auto highGain = apvts.getRawParameterValue ("HighGainDb")->load();

    auto sampleRate = getSampleRate();

    // 各バンドの係数計算
    auto lowCoeffs = juce::dsp::IIR::Coefficients<float>::makeLowShelf (sampleRate, 100.0f, 0.707f, juce::Decibels::decibelsToGain (lowGain));
    auto midCoeffs = juce::dsp::IIR::Coefficients<float>::makePeakFilter (sampleRate, 1000.0f, 1.0f, juce::Decibels::decibelsToGain (midGain));
    auto highCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighShelf (sampleRate, 8000.0f, 0.707f, juce::Decibels::decibelsToGain (highGain));

    // 各フィルタへの係数セット
    leftChain.get<LowShelf>().coefficients = lowCoeffs;
    rightChain.get<LowShelf>().coefficients = lowCoeffs;
    leftChain.get<MidPeak>().coefficients = midCoeffs;
    rightChain.get<MidPeak>().coefficients = midCoeffs;
    leftChain.get<HighShelf>().coefficients = highCoeffs;
    rightChain.get<HighShelf>().coefficients = highCoeffs;
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
