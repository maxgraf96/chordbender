//
// Created by Max on 21/10/2023.
//

#ifndef NETZ_MIDI_RECEIVER_PLUGINEDITOR_H
#define NETZ_MIDI_RECEIVER_PLUGINEDITOR_H

#pragma once

#include <JuceHeader.h>

#include "PluginProcessor.h"
#include "CustomLookAndFeel.h"
#include "UI/PNGKnob.h"

class ChordBenderAudioProcessorEditor  :
        public juce::AudioProcessorEditor,
        public juce::Timer,
        public Slider::Listener
{
public:
    ChordBenderAudioProcessorEditor (ChordBenderAudioProcessor&);
    ~ChordBenderAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void timerCallback() override;

    void sliderValueChanged(Slider* slider) override;

private:
    // Reference to our processor
    ChordBenderAudioProcessor& processor;

    CustomLookAndFeel customLookAndFeel;
    Label titleLabel;
    Label connectedLabel;

    // UI Component for the knob
    Slider bendDurationSlider;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> bendDurationSliderAttachment;
    std::unique_ptr<PNGKnob> bendDurationKnob;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChordBenderAudioProcessorEditor)
};

#endif //NETZ_MIDI_RECEIVER_PLUGINEDITOR_H
