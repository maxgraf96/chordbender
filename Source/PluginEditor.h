//
// Created by Max on 21/10/2023.
//

#ifndef NETZ_MIDI_RECEIVER_PLUGINEDITOR_H
#define NETZ_MIDI_RECEIVER_PLUGINEDITOR_H

#pragma once

#include <JuceHeader.h>
//#include <readerwriterqueue.h>

#include "PluginProcessor.h"
#include "CustomLookAndFeel.h"

class ChordBenderAudioProcessorEditor  :
        public juce::AudioProcessorEditor,
        public juce::Timer
{
public:
    ChordBenderAudioProcessorEditor (ChordBenderAudioProcessor&);
    ~ChordBenderAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void timerCallback() override;

private:
    // Reference to our processor
    ChordBenderAudioProcessor& processor;
//    moodycamel::ReaderWriterQueue<MIDIMessage>& editorQueue;

    CustomLookAndFeel customLookAndFeel;
    Label titleLabel;
    Label connectedLabel;
    juce::TextEditor textEditor;
    juce::Viewport viewport;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChordBenderAudioProcessorEditor)
};

#endif //NETZ_MIDI_RECEIVER_PLUGINEDITOR_H
