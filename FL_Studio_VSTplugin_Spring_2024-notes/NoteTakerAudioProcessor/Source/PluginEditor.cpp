#include "PluginProcessor.h"
#include "PluginEditor.h"
//#include <juce_gui_basics/juce_gui_basics.h>


class PopupWindow : public juce::DocumentWindow, public juce::Button::Listener
{
public:
    PopupWindow(NoteTakerAudioProcessorEditor* editor)
    : juce::DocumentWindow ("File label",
                            juce::Desktop::getInstance().getDefaultLookAndFeel()
                            .findColour (juce::ResizableWindow::backgroundColourId),
                            juce::DocumentWindow::allButtons),
    parentEditor(editor)
    {
        setUsingNativeTitleBar (true);
        contentComponent.addAndMakeVisible(textEditor);
        contentComponent.addAndMakeVisible(saveButtonPop);
        
        saveButtonPop.setButtonText("Save");
        saveButtonPop.addListener(this);
        
        
        textEditor.setReadOnly(false);
        textEditor.setVisible(true);
        
        setContentOwned (&contentComponent, false);
        contentComponent.setVisible(true);
        
        setSize(200, 100);
        centreWithSize (getWidth(), getHeight());
        setVisible (true);
        setAlwaysOnTop(true);
    }
    
    void closeButtonPressed() override
    {
        delete this;
    }
    
    void resized() override
    {
        contentComponent.setBounds(getLocalBounds());
        textEditor.setBounds (10, 10, getWidth() - 20, getHeight() - 50);
        saveButtonPop.setBounds(10, getHeight() - 30, getWidth() - 20, 20);
    }
    
    void buttonClicked(juce::Button* button) override
    {
        if (button == &saveButtonPop)
        {
            juce::String filename = textEditor.getText();
            parentEditor->saveTextToFile(filename);
            closeButtonPressed();
        }
    }
    
private:
    juce::Component contentComponent;
    juce::TextEditor textEditor;
    juce::TextButton saveButtonPop;
    NoteTakerAudioProcessorEditor* parentEditor;
};



//==============================================================================
NoteTakerAudioProcessorEditor::NoteTakerAudioProcessorEditor (NoteTakerAudioProcessor& p)
: AudioProcessorEditor (&p), processor (p)
{
    setSize (600, 450);
    addAndMakeVisible(textEditor);
    addAndMakeVisible(saveButton);
    addAndMakeVisible(loadButton);
    
    saveButton.setButtonText("Save");
    loadButton.setButtonText("Load");
    saveButton.addListener(this);
    loadButton.addListener(this);
    textEditor.setMultiLine(true, true);
    textEditor.setReturnKeyStartsNewLine(true);
    textEditor.setText("text from initialization");
}

NoteTakerAudioProcessorEditor::~NoteTakerAudioProcessorEditor()
{
    
}

void NoteTakerAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    
}

void NoteTakerAudioProcessorEditor::resized()
{
    textEditor.setBounds (getLocalBounds());
    saveButton.setBounds(10, getHeight() - 30, (getWidth() - 20)/2-10, 20);
    loadButton.setBounds(10 + (getWidth() - 20)/2, getHeight() - 30,(getWidth() - 20)/2-10, 20);
    
}

void NoteTakerAudioProcessorEditor::saveTextToFile(const juce::String& filename)
{
    juce::String text = textEditor.getText();
    juce::String currentDate = juce::Time::getCurrentTime().formatted("%Y-%m-%d_%H-%M-%S");
    
    // Create an XmlElement object
    std::unique_ptr<juce::XmlElement> xml(new juce::XmlElement("root"));
    
    // Add a child element with the text from the TextEditor
    xml->createNewChildElement("text")->addTextElement(text);
    
    // Write the XML to a file   
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
    
    
    DBG("XML file saved.");
}


void NoteTakerAudioProcessorEditor::buttonClicked(juce::Button* button)
{
    if (button == &saveButton)
    {
        saveTextToFile("");
    }
    else if (button == &loadButton)
    {

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
}



void NoteTakerAudioProcessorEditor::loadTextFromFile(const juce::String& filePath)
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
    juce::XmlElement* textElement = xmlElement->getFirstChildElement(); // Adjust this as needed
    if (textElement && textElement->hasTagName("text"))
    {
        juce::String textContent = textElement->getAllSubText();
        // Update the TextEditor with the loaded content
        textEditor.setText(textContent, false); // 'false' indicates not to send a notification
    }
    else
    {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                                               "XML Structure Error",
                                               "The XML file does not have the expected structure.",
                                               "OK");
    }
}



