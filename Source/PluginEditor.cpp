//
// Created by Max on 21/10/2023.
//

#include "PluginEditor.h"

#include "PluginProcessor.h"
#include "PluginEditor.h"

ChordBenderAudioProcessorEditor::ChordBenderAudioProcessorEditor (ChordBenderAudioProcessor& p)
        : AudioProcessorEditor (&p), processor (p)
//        , editorQueue(messageQueue)
{
    // Set the size of the editor window.
    setSize (400, 300);
    setResizable(true, true);

    titleLabel.setText("chordbender", dontSendNotification);
    titleLabel.setJustificationType(Justification::left);
    titleLabel.setFont(Font(18.0f, Font::italic));
    titleLabel.setColour(Label::textColourId, Colours::white);
    titleLabel.setBounds(0, 0, 200, 20);
    addAndMakeVisible(titleLabel);

    connectedLabel.setText("", dontSendNotification);
    connectedLabel.setJustificationType(Justification::right);
    connectedLabel.setFont(Font(18.0f, Font::italic));
    connectedLabel.setColour(Label::textColourId, Colours::white);
    connectedLabel.setBounds(180, 0, 100, 20);
//    connectedLabel.setColour(Label::backgroundColourId, Colours::red);
    addAndMakeVisible(connectedLabel);

    // Make the text editor multi-line and read-only (if desired)
    textEditor.setMultiLine(true);
    textEditor.setReadOnly(true);
    textEditor.setColour(juce::TextEditor::textColourId, juce::Colours::white);
    textEditor.setColour(juce::TextEditor::backgroundColourId, juce::Colours::black);

    // Add the text editor to the viewport
    viewport.setViewedComponent(&textEditor, false);

    textEditor.setBounds(0, 0, getParentWidth(), getParentHeight() - 20);
    addAndMakeVisible(viewport);
    viewport.setBounds(0, 20, getParentWidth(), getParentHeight() - 20);
    viewport.setLookAndFeel(&customLookAndFeel);

    startTimer(50);

    resized();
}

ChordBenderAudioProcessorEditor::~ChordBenderAudioProcessorEditor()
{
    // Destructor. Any cleanup code can be added here.
}

void ChordBenderAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Clear the background
    g.fillAll (juce::Colours::black);
}

void ChordBenderAudioProcessorEditor::resized()
{
    //viewport.setBounds(getLocalBounds());
    juce::FlexBox flexBox;
    juce::FlexBox titleFlexBox;

    flexBox.flexDirection = juce::FlexBox::Direction::column;  // Vertical layout

    titleFlexBox.flexDirection = juce::FlexBox::Direction::row;  // Horizontal layout
    titleFlexBox.justifyContent = juce::FlexBox::JustifyContent::spaceBetween;  // Put space between the two items
    titleFlexBox.items.add(juce::FlexItem(titleLabel).withFlex(1));  // titleLabel will take up half the width
    titleFlexBox.items.add(juce::FlexItem(connectedLabel).withFlex(1));  // connectedLabel will take up the other half
    titleFlexBox.performLayout(Rectangle<float>(0, 0, getWidth(), 20));

    // Add the top bar and viewport (containing the text editor) to the main flexbox
    flexBox.items.add(juce::FlexItem(titleFlexBox).withHeight(20));
    flexBox.items.add(juce::FlexItem(viewport).withFlex(1));  // The viewport will take up the remaining space

    // Perform the layout
    flexBox.performLayout(getLocalBounds());
}

void ChordBenderAudioProcessorEditor::timerCallback()
{
    repaint();

//    if(processor.getConnected()){
//        connectedLabel.setColour(juce::Label::backgroundColourId, juce::Colours::darkgreen);
//        connectedLabel.setText("Connected to " + processor.getSenderIP(), dontSendNotification);
//        textEditor.setColour(juce::TextEditor::backgroundColourId, juce::Colours::darkgreen);
//    } else {
//        connectedLabel.setColour(juce::Label::backgroundColourId, juce::Colours::red);
//        connectedLabel.setText("Not connected", dontSendNotification);
//        textEditor.setColour(juce::TextEditor::backgroundColourId, juce::Colours::black);
//    }

    // Empty the queue
//    MIDIMessage message;
//    while(editorQueue.try_dequeue(message)){
//        textEditor.moveCaretToEnd();
//        textEditor.insertTextAtCaret(message.toString() + "\n");
//    }
}
