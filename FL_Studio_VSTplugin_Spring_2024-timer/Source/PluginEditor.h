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
class TimerAppAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    TimerAppAudioProcessorEditor (TimerAppAudioProcessor&);
    ~TimerAppAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void startTime();

    void stopTime();

    void autostopTime();

    void resetTime();

    void saveTime();

    void logsTime();

    void saveName(const juce::String& name);

    void autosaveName(const juce::String& name) const;

    void chargeTime();

    void upchargeTime();

    void loadTextFromFile(const juce::String& filePath);

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.

    void timerCallback() override;
   
    TimerAppAudioProcessor& audioProcessor;

    int sec = 0, hr = 0, min = 0;

    juce::TextButton startButton;
    juce::TextButton stopButton;
    juce::TextButton resetButton;

    juce::TextButton saveButton;
    juce::TextButton logsButton;

    juce::TextEditor chargeAmount;
    juce::Label chargeOutput;
    juce::TextButton updateCharge;
    juce::TextButton chargeButton;
    juce::String totalCharge = "0";
    
    juce::String nameI;
    juce::Label timeLabel;
    juce::String timeString;
    juce::String timeString1;
    double timeNo;

    int isSaved = 0;
    int usedComboBox = 0;
    std::unique_ptr<juce::AlertWindow> alert = std::make_unique<juce::AlertWindow>("save", "choose0", juce::MessageBoxIconType::NoIcon, nullptr);
    juce::ComboBox* logsComboBox;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TimerAppAudioProcessorEditor)
};
