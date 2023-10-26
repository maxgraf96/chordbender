//
// Created by Max on 21/10/2023.
//

#ifndef CHORD_BENDER_PLUGINPROCESSOR_H
#define CHORD_BENDER_PLUGINPROCESSOR_H

#pragma once

#include <JuceHeader.h>
#include <readerwriterqueue.h>

class ChordBenderAudioProcessor  : public juce::AudioProcessor, public juce::HighResolutionTimer
{
public:
    ChordBenderAudioProcessor();
    ~ChordBenderAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isMidiEffect() const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    void hiResTimerCallback() override;

    // Not really equal but equal for our means
    bool midiEqual(const MidiMessage& m1, const MidiMessage& m2){
        return m1.getChannel() == m2.getChannel() &&
               m1.getNoteNumber() == m2.getNoteNumber();
    }

private:
    int bendDuration = 200; // ms
    std::vector<MidiMessage> activeNotes;
    std::vector<MidiMessage> sourceNotes;
    std::vector<MidiMessage> targetNotes;

    std::atomic<bool> acceptTarget = false;

    // This one is connected to the message receiver thread
//    moodycamel::ReaderWriterQueue<MIDIMessage> editorQueue;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChordBenderAudioProcessor)
};



#endif //CHORD_BENDER_PLUGINPROCESSOR_H
