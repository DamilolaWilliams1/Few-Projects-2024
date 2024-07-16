/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
GarethsEQAudioProcessor::GarethsEQAudioProcessor()
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
    //createEditorIfNeeded();
}

GarethsEQAudioProcessor::~GarethsEQAudioProcessor()
{
}

//==============================================================================
const juce::String GarethsEQAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool GarethsEQAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool GarethsEQAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool GarethsEQAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double GarethsEQAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int GarethsEQAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int GarethsEQAudioProcessor::getCurrentProgram()
{
    return 0;
}

void GarethsEQAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String GarethsEQAudioProcessor::getProgramName (int index)
{
    return {};
}

void GarethsEQAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}
 
//==============================================================================
void GarethsEQAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;

    spec.maximumBlockSize = samplesPerBlock;

    spec.numChannels = 1;

    spec.sampleRate = sampleRate;

    leftChain.prepare(spec);
    rightChain.prepare(spec);

    
    updateFilters();


}

void GarethsEQAudioProcessor::releaseResources()
{
   
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool GarethsEQAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
   
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

void GarethsEQAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());


    updateFilters();


    juce::dsp::AudioBlock<float> block(buffer);

    auto leftBlock = block.getSingleChannelBlock(0);
    auto rightBlock = block.getSingleChannelBlock(1);

    juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
    juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);

    leftChain.process(leftContext);
    rightChain.process(rightContext);
}

//==============================================================================
bool GarethsEQAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* GarethsEQAudioProcessor::createEditor()
{
    return new GarethsEQAudioProcessorEditor (*this);
    //return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void GarethsEQAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream mos(destData, true);
    apvts.state.writeToStream(mos);
}

void GarethsEQAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
    if (tree.isValid()) {
        apvts.replaceState(tree);
        updateFilters();
    }
}


ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts) {
    ChainSettings settings;

    settings.lowCutFreq = apvts.getRawParameterValue("LowCut Freq")->load();
    settings.highCutFreq = apvts.getRawParameterValue("HighCut Freq")->load();

    settings.peakFreq1 = apvts.getRawParameterValue("Peak Freq 1")->load();
    settings.peakGainInDecibels1 = apvts.getRawParameterValue("Peak Gain 1")->load();
    settings.peakQuality1 = apvts.getRawParameterValue("Peak Quality 1")->load();

    settings.peakFreq2 = apvts.getRawParameterValue("Peak Freq 2")->load();
    settings.peakGainInDecibels2 = apvts.getRawParameterValue("Peak Gain 2")->load();
    settings.peakQuality2 = apvts.getRawParameterValue("Peak Quality 2")->load();

    settings.peakFreq3 = apvts.getRawParameterValue("Peak Freq 3")->load();
    settings.peakGainInDecibels3 = apvts.getRawParameterValue("Peak Gain 3")->load();
    settings.peakQuality3 = apvts.getRawParameterValue("Peak Quality 3")->load();

    settings.lowCutSlope = static_cast<Slope>(apvts.getRawParameterValue("LowCut Slope")->load());
    settings.highCutSlope = static_cast<Slope>(apvts.getRawParameterValue("HighCut Slope")->load());

    return settings;
}

Coefficients makePeakFilter1(const ChainSettings& chainSettings, double sampleRate) {
    return juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, chainSettings.peakFreq1,
        chainSettings.peakQuality1, juce::Decibels::decibelsToGain(chainSettings.peakGainInDecibels1));
}

Coefficients makePeakFilter2(const ChainSettings& chainSettings, double sampleRate) {
    return juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, chainSettings.peakFreq2,
        chainSettings.peakQuality2, juce::Decibels::decibelsToGain(chainSettings.peakGainInDecibels2));
}

Coefficients makePeakFilter3(const ChainSettings& chainSettings, double sampleRate) {
    return juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, chainSettings.peakFreq3,
        chainSettings.peakQuality3, juce::Decibels::decibelsToGain(chainSettings.peakGainInDecibels3));
}

void GarethsEQAudioProcessor::updatePeakFilter(const ChainSettings& chainSettings) {
    auto peakCoeffiecients1 = makePeakFilter1(chainSettings, getSampleRate());
    auto peakCoeffiecients2 = makePeakFilter2(chainSettings, getSampleRate());
    auto peakCoeffiecients3 = makePeakFilter3(chainSettings, getSampleRate());

    updateCoefficients(leftChain.get<ChainPositions::Peak1>().coefficients, peakCoeffiecients1);
    updateCoefficients(leftChain.get<ChainPositions::Peak2>().coefficients, peakCoeffiecients2);
    updateCoefficients(leftChain.get<ChainPositions::Peak3>().coefficients, peakCoeffiecients3);

    updateCoefficients(rightChain.get<ChainPositions::Peak1>().coefficients, peakCoeffiecients1);
    updateCoefficients(rightChain.get<ChainPositions::Peak2>().coefficients, peakCoeffiecients2);
    updateCoefficients(rightChain.get<ChainPositions::Peak3>().coefficients, peakCoeffiecients3);
}

void updateCoefficients(Coefficients& old, const Coefficients& replacements) {
    *old = *replacements;
}

void GarethsEQAudioProcessor::updateLowCutFilters(const ChainSettings& chainSettings) {
    auto cutCoefficients = makeLowCutFilter(chainSettings, getSampleRate());


    auto& leftLowCut = leftChain.get<ChainPositions::LowCut>();
    auto& rightLowCut = rightChain.get<ChainPositions::LowCut>();

    updateCutFilter(leftLowCut, cutCoefficients, chainSettings.lowCutSlope);
    updateCutFilter(rightLowCut, cutCoefficients, chainSettings.lowCutSlope);
}

void GarethsEQAudioProcessor::updateHighCutFilters(const ChainSettings& chainSettings) {
    auto highCutCoefficients = makeHighCutFilter(chainSettings, getSampleRate());


    auto& leftHighCut = leftChain.get<ChainPositions::HighCut>();
    auto& rightHighCut = rightChain.get<ChainPositions::HighCut>();

    updateCutFilter(leftHighCut, highCutCoefficients, chainSettings.highCutSlope);
    updateCutFilter(rightHighCut, highCutCoefficients, chainSettings.highCutSlope);
}

void GarethsEQAudioProcessor::updateFilters() {
    auto chainSettings = getChainSettings(apvts);

    updateLowCutFilters(chainSettings);
    updatePeakFilter(chainSettings);
    updateHighCutFilters(chainSettings);
}

juce::AudioProcessorValueTreeState::ParameterLayout 
    GarethsEQAudioProcessor::createParameterLayout() {


    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>("LowCut Freq",
        "LowCut Freq",
        juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f),
        20.f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("HighCut Freq",
        "HighCut Freq",
        juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f),
        20000.f));



    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak Freq 1",
        "Peak Freq 1",
        juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f),
        1000.f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak Gain 1",
        "Peak Gain 1",
        juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f),
        0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak Quality 1",
        "Peak Quality 1",
        juce::NormalisableRange<float>(0.1f, 10.f, 0.05f, 1.f),
        1.f));



    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak Freq 2",
        "Peak Freq 2",
        juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f),
        750.f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak Gain 2",
        "Peak Gain 2",
        juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f),
        0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak Quality 2",
        "Peak Quality 2",
        juce::NormalisableRange<float>(0.1f, 10.f, 0.05f, 1.f),
        1.f));






    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak Freq 3",
        "Peak Freq 3",
        juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f),
        400.f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak Gain 3",
        "Peak Gain 3",
        juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f),
        0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak Quality 3",
        "Peak Quality 3",
        juce::NormalisableRange<float>(0.1f, 10.f, 0.05f, 1.f),
        1.f));




    juce::StringArray stringArray;
    for (int i = 0; i < 4; ++i)
    {
        juce::String str;
        str << (12 + i * 12);
        str << " db/Oct";
        stringArray.add(str);
    }

    layout.add(std::make_unique<juce::AudioParameterChoice>("LowCut Slope", "LowCut Slope", stringArray, 0));
    layout.add(std::make_unique<juce::AudioParameterChoice>("HighCut Slope", "HighCut Slope", stringArray, 0));

    layout.add(std::make_unique<juce::AudioParameterBool>("LowCut Bypassed", "LowCut Bypassed", false));
    layout.add(std::make_unique<juce::AudioParameterBool>("Peak Bypassed", "Peak Bypassed", false));
    layout.add(std::make_unique<juce::AudioParameterBool>("HighCut Bypassed", "HighCut Bypassed", false));
    layout.add(std::make_unique<juce::AudioParameterBool>("Analyzer Enabled", "Analyzer Enabled", true));

    return layout;
}




//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GarethsEQAudioProcessor();
}
