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

    titleLabel.setText("chordbender", dontSendNotification);
    titleLabel.setJustificationType(Justification::left);
    titleLabel.setFont(Font("Helvetica", 18.0f, Font::italic));
    titleLabel.setColour(Label::textColourId, Colours::white);
    titleLabel.setBounds(0, 0, 200, 20);
    addAndMakeVisible(titleLabel);
    // rotate the title label
    titleLabel.setTransform(AffineTransform::rotation(-juce::MathConstants<float>::pi / 23.5, titleLabel.getLocalBounds().getCentreX(), titleLabel.getLocalBounds().getCentreY()));

    connectedLabel.setText("", dontSendNotification);
    connectedLabel.setJustificationType(Justification::right);
    connectedLabel.setFont(Font("Helvetica", 18.0f, Font::italic));
    connectedLabel.setColour(Label::textColourId, Colours::white);
    connectedLabel.setBounds(180, 0, 100, 20);
//    connectedLabel.setColour(Label::backgroundColourId, Colours::red);
    addAndMakeVisible(connectedLabel);

    bendDurationSlider.setRange(100, 3000, 1); // Set range with step size of 1
    addAndMakeVisible(bendDurationSlider);
    bendDurationSlider.setVisible(false);

    // Attach the slider to the parameter in the treeState
    bendDurationSliderAttachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(processor.apvts, "bendDuration", bendDurationSlider);
    bendDurationSlider.addListener(this); // Add the editor as a listener to the slider

    bendDurationKnob = std::make_unique<PNGKnob>(BinaryData::knob_png, BinaryData::knob_pngSize, 60, 60);
    bendDurationKnob->setBounds(0, 0, 140, 100);
    addAndMakeVisible(bendDurationKnob.get());

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

    // Render a line between the title and the viewport
    // Set the color for the border
    g.setColour (juce::Colours::lightgrey);
    // Draw a line for the top border
    g.drawLine (0, 60, getWidth(), 0, 1); // The last parameter is the line thickness
}

void ChordBenderAudioProcessorEditor::resized()
{
    //viewport.setBounds(getLocalBounds());
    juce::FlexBox flexBox;
    juce::FlexBox titleFlexBox;
    juce::FlexBox knobFlexBox;

    flexBox.flexDirection = juce::FlexBox::Direction::column;  // Vertical layout

    titleFlexBox.flexDirection = juce::FlexBox::Direction::row;  // Horizontal layout
    titleFlexBox.justifyContent = juce::FlexBox::JustifyContent::spaceBetween;  // Put space between the two items
    auto titleMargin = juce::FlexItem::Margin(10, 0, 0, 10);
    titleFlexBox.items.add(juce::FlexItem(titleLabel).withFlex(1).withMargin(titleMargin));  // titleLabel will take up half the width
    titleFlexBox.items.add(juce::FlexItem(connectedLabel).withFlex(1));  // connectedLabel will take up the other half
    titleFlexBox.performLayout(Rectangle<float>(0, 0, getWidth(), 30));

    // Add the top bar and viewport (containing the text editor) to the main flexbox
    flexBox.items.add(juce::FlexItem(titleFlexBox).withHeight(30));
    flexBox.items.add(juce::FlexItem(knobFlexBox).withFlex(1));  // The viewport will take up the remaining space

    knobFlexBox.flexDirection = juce::FlexBox::Direction::row;  // Horizontal layout
    knobFlexBox.alignItems = juce::FlexBox::AlignItems::center;
    knobFlexBox.justifyContent = juce::FlexBox::JustifyContent::center;
    knobFlexBox.items.add(juce::FlexItem(*bendDurationKnob).withWidth(140).withHeight(200));
    knobFlexBox.performLayout(Rectangle<float>(0, 0, getWidth(), getHeight() - 20));

    // Perform the layout
    flexBox.performLayout(getLocalBounds());
}

void ChordBenderAudioProcessorEditor::sliderValueChanged(Slider* slider)
{
    if (slider == &bendDurationSlider)
    {
        float normalizedValue = (slider->getValue() - slider->getMinimum()) / (slider->getMaximum() - slider->getMinimum());
        bendDurationSlider.setValue(normalizedValue);
    }
}

void ChordBenderAudioProcessorEditor::timerCallback()
{
    repaint();
}
