/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

NoteTakerAudioProcessor::NoteTakerAudioProcessor()
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


NoteTakerAudioProcessor::~NoteTakerAudioProcessor()
{
}


const juce::String NoteTakerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool NoteTakerAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool NoteTakerAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool NoteTakerAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double NoteTakerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int NoteTakerAudioProcessor::getNumPrograms()
{
    return 1;  
}

int NoteTakerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void NoteTakerAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String NoteTakerAudioProcessor::getProgramName (int index)
{
    return {};
}

void NoteTakerAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}


void NoteTakerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{

}

void NoteTakerAudioProcessor::releaseResources()
{

}

#ifndef JucePlugin_PreferredChannelConfigurations
bool NoteTakerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else

    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;


   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void NoteTakerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());


    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

    
    }
}

bool NoteTakerAudioProcessor::hasEditor() const
{
    return true; 
}

juce::AudioProcessorEditor* NoteTakerAudioProcessor::createEditor()
{
    return new NoteTakerAudioProcessorEditor (*this);
}


void NoteTakerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{

}

void NoteTakerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{

}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NoteTakerAudioProcessor();
}
