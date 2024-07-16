/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <base/source/fstring.h>


//==============================================================================
TimerAppAudioProcessorEditor::TimerAppAudioProcessorEditor (TimerAppAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (600, 400);
    startTimer(1000);

    startButton.setButtonText("START");
    startButton.onClick = [this]() {startTime(); };
    addAndMakeVisible(startButton);
    
    stopButton.setButtonText("STOP");
    stopButton.onClick = [this] { stopTime(); };
    addAndMakeVisible(stopButton);

    resetButton.setButtonText("RESET");
    resetButton.onClick = [this] { resetTime(); };
    addAndMakeVisible(resetButton);

    saveButton.setButtonText("SAVE");
    saveButton.onClick = [this] { saveTime(); };
    addAndMakeVisible(saveButton);

    logsButton.setButtonText("LOGS");
    logsButton.onClick = [this] { logsTime(); };
    addAndMakeVisible(logsButton);

    timeLabel.setFont(juce::Font(150.0f));
    timeLabel.setText("00:00:00", juce::dontSendNotification);
    timeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(timeLabel);

    chargeButton.setButtonText("CHARGE");
    chargeButton.onClick = [this] { chargeTime(); };
    addAndMakeVisible(chargeButton);

    updateCharge.setButtonText("UPDATE");
    updateCharge.onClick = [this] { upchargeTime(); };

    chargeAmount.setText("Rate per Hour");
    addAndMakeVisible(chargeAmount);

}

TimerAppAudioProcessorEditor::~TimerAppAudioProcessorEditor()
{
    if (isSaved == 0) {
        if(usedComboBox) delete(logsComboBox);
        autostopTime();
        juce::String nameX = "AUTO_GENERATED_SAVE";
        autosaveName(nameX);
    };
}
   

//==============================================================================
void TimerAppAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);

}

void TimerAppAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    startButton.setBounds(90,290,100,50);
    stopButton.setBounds(250, 290, 100, 50);
    resetButton.setBounds(410, 290, 100, 50);
    logsButton.setBounds(390, 0, 100, 50);
    saveButton.setBounds(500, 0, 100, 50);
    timeLabel.setBounds(50, 40, 500, 250);
    chargeAmount.setBounds(0, 0, 100, 50);
    chargeButton.setBounds(110, 0, 100, 50);
    chargeOutput.setBounds(0, 0, 100, 50);
    updateCharge.setBounds(110, 0, 100, 50);
}

void TimerAppAudioProcessorEditor::startTime()
{
    startTimer(1000);
}

void TimerAppAudioProcessorEditor::stopTime()
{
    auto sstr = juce::String(sec).paddedLeft('0', 2);
    auto mstr = juce::String(min).paddedLeft('0', 2);
    auto hstr = juce::String(hr).paddedLeft('0', 2);
    timeLabel.setText(hstr + ":" + mstr + ":" + sstr, juce::dontSendNotification);
    timeString1 = hstr + ":" + mstr + ":" + sstr;
    timeString = hstr + "h_" + mstr + "m_" + sstr + "s";


    int h = hr * 3600;
    int m = min * 60;
    int s = sec;
    timeNo = h + m + s;

    stopTimer();
}

void TimerAppAudioProcessorEditor::resetTime()
{
    timeLabel.setText("00:00:00", juce::dontSendNotification);
    sec = 0;
    hr = 0;
    min = 0;
    stopTimer();
}

void TimerAppAudioProcessorEditor::timerCallback()
{ 
        sec++;
        if (sec == 60) {
            sec = 0;
            min++;
            if (min == 60) {
                min = 0;
                hr++;
            }
        }

        auto sstr = juce::String(sec).paddedLeft('0', 2); 
        auto mstr = juce::String(min).paddedLeft('0', 2); 
        auto hstr = juce::String(hr).paddedLeft('0', 2); 
        timeLabel.setText(hstr + ":" + mstr + ":" + sstr, juce::dontSendNotification);
}

void TimerAppAudioProcessorEditor::saveTime()
{
    stopTime();
    isSaved = 1;
    saveName("");

}

void TimerAppAudioProcessorEditor::loadTextFromFile(const juce::String& filePath)
{
    juce::File xmlFile(filePath);

    // Ensure the file exists
    if (!xmlFile.existsAsFile())
    {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
            "File Error",
            "The specified file does not exist or cannot be read.",
            "OK");
        return;
    }

    // Parse the XML file
    std::unique_ptr<juce::XmlElement> xmlElement(juce::XmlDocument::parse(xmlFile));
    if (!xmlElement)
    {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
            "XML Error",
            "The specified file could not be parsed as XML.",
            "OK");
        return;
    }

    // Assuming your XML has a specific structure, extract data accordingly
    // For example, if you're looking for text within a "text" element
    juce::XmlElement* textElement = xmlElement->getFirstChildElement()->getNextElement(); // Adjust this as needed
    juce::XmlElement* textElement1 = xmlElement->getFirstChildElement()->getNextElement()->getNextElement();
    juce::XmlElement* textElement2 = xmlElement->getFirstChildElement()->getNextElement()->getNextElement()->getNextElement();
    if (textElement && textElement->hasTagName("time"))
    {
        if (textElement1 && textElement1->hasTagName("date")) {
            if (textElement2 && textElement2->hasTagName("charge")) {
                stopTime();
                juce::String textContent = textElement->getAllSubText();
                juce::String textContent1 = textElement1->getAllSubText();
                juce::String textContent2 = textElement2->getAllSubText();
                juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                    "INFO",
                    "Time: " + textContent + "\nDate: " + textContent1 + "\nTotal Earnings: $" + textContent2,
                    "OK");
            }
        }
    }
    else
    {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
            "XML Structure Error",
            "The XML file does not have the expected structure.",
            "OK");
    }
}

#include <string>
void TimerAppAudioProcessorEditor::logsTime()
{
    juce::String currentDate = juce::Time::getCurrentTime().formatted("%Y-%m-%d_%H-%M-%S");
    

    juce::FileChooser fileChooser ("Select a file to load...",
                                   juce::File(""), // Direct path
                                   "*.xml");


           if (fileChooser.browseForFileToOpen()) // Opens the dialog to choose the file
           {

               juce::File file = fileChooser.getResult(); // Get the file the user selected
 
               loadTextFromFile(file.getFullPathName()); // Load the text from the selected file
               
           }
           else {
            
           }
}


#include <juce_core/juce_core.h> // Include necessary JUCE header for XML handling

void TimerAppAudioProcessorEditor::saveName(const juce::String& name)
{
    juce::String currentDate = juce::Time::getCurrentTime().formatted("%d-%m-%Y");
    std::unique_ptr<juce::XmlElement> xml(new juce::XmlElement("root"));
    xml->createNewChildElement("username")->addTextElement(name);
    xml->createNewChildElement("time")->addTextElement(timeString1);
    xml->createNewChildElement("date")->addTextElement(currentDate);
    xml->createNewChildElement("charge")->addTextElement(totalCharge);
    
    static const juce::String filename = juce::File::getCurrentWorkingDirectory().getFileName();


    juce::FileChooser fileChooser ("Select a file to load...",
                                   juce::File(""), // Direct path
                                   "*."+currentDate+".xml");


           if (fileChooser.browseForFileToSave(true)) // Opens the dialog to choose the file
           {

               juce::File file = fileChooser.getResult(); // Get the file the user selected
               xml->writeToFile(file, "");
               
           }
           else {
            
           }
    
    
    
    
    isSaved = 0;
    alert->showMessageBoxAsync(juce::MessageBoxIconType::NoIcon, "INFO", "XML SAVED", "OK", nullptr);
}

void TimerAppAudioProcessorEditor::autostopTime()
{
    auto sstr = juce::String(sec).paddedLeft('0', 2);
    auto mstr = juce::String(min).paddedLeft('0', 2);
    auto hstr = juce::String(hr).paddedLeft('0', 2);
    timeString = hstr + "h_" + mstr + "m_" + sstr + "s";
    timeString1 = hstr + ":" + mstr + ":" + sstr;
    stopTimer();
}


void TimerAppAudioProcessorEditor::autosaveName(const juce::String& nam) const
{
    juce::String currentDate = juce::Time::getCurrentTime().formatted("%d-%m-%Y");
    std::unique_ptr<juce::XmlElement> xml1(new juce::XmlElement("sroot"));
    xml1->createNewChildElement("username")->addTextElement(nam);
    xml1->createNewChildElement("time")->addTextElement(timeString1);
    xml1->createNewChildElement("date")->addTextElement(currentDate);
    xml1->createNewChildElement("charge")->addTextElement(totalCharge);
    
    juce::FileChooser fileChooser ("Select a file to load...",
                                   juce::File(""), // Direct path
                                   "*."+currentDate+".xml");


           if (fileChooser.browseForFileToSave(true)) // Opens the dialog to choose the file
           {

               juce::File file = fileChooser.getResult(); // Get the file the user selected
               xml1->writeToFile(file, "");
               
           }
           else {
            
           }
    
    
}

void TimerAppAudioProcessorEditor::chargeTime()
{
    int length = 0;
    stopTime();
    for (char check : chargeAmount.getText()) {
        int i = isdigit(check);
        if (i == 0) {
            alert->showMessageBoxAsync(juce::MessageBoxIconType::NoIcon, "INFO", "ENTER A VALID NUMBER", "OK", nullptr);
            break;
        }
        length++;
    }

    if (length == chargeAmount.getText().length()) {
        double ratePerhour = chargeAmount.getText().getIntValue();
        double amountTimespent = timeNo / 3600;
        double value = ratePerhour * amountTimespent;
        totalCharge = juce::String(value);
        juce::String display = juce::String(value);
        chargeOutput.setText("$" + display, juce::dontSendNotification);

        chargeButton.setVisible(false);
        chargeAmount.setVisible(false);

        addAndMakeVisible(updateCharge);
        addAndMakeVisible(chargeOutput);
    }
}

void TimerAppAudioProcessorEditor::upchargeTime()
{
    updateCharge.setVisible(false);
    chargeOutput.setVisible(false);
   
    chargeButton.setVisible(true);
    chargeAmount.setVisible(true);
}

