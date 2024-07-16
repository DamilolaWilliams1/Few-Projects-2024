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



class NoteTakerAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Button::Listener
{
public:
    NoteTakerAudioProcessorEditor (NoteTakerAudioProcessor& p);
    ~NoteTakerAudioProcessorEditor() override;

    void paint (juce::Graphics& g) override;
    void resized() override;
    void saveTextToFile(const juce::String& filename);
    void buttonClicked(juce::Button* button) override;  // This function is called when a button is clicked
    void loadTextFromFile(const juce::String& filePath);
    

private:
    NoteTakerAudioProcessor& processor;
    juce::TextEditor textEditor;
    juce::TextButton saveButton;
    juce::TextButton loadButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NoteTakerAudioProcessorEditor)
};
