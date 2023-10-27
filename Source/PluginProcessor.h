//
// Created by Max on 21/10/2023.
//

#ifndef CHORD_BENDER_PLUGINPROCESSOR_H
#define CHORD_BENDER_PLUGINPROCESSOR_H

#pragma once

#include <map>
#include <vector>
#include <algorithm>
#include <JuceHeader.h>

class ChordBenderAudioProcessor  : public juce::AudioProcessor
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

    // Not really equal but equal for our means
    bool midiEqual(const MidiMessage& m1, const MidiMessage& m2){
        return m1.getNoteNumber() == m2.getNoteNumber();
    }

    static float mapFloat(float value, float in_min, float in_max, float out_min, float out_max) {
        return (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }

    struct MidiMessageLess {
        bool operator()(const MidiMessage& a, const MidiMessage& b) const {
            return a.getNoteNumber() < b.getNoteNumber();
        }
    };

    std::map<MidiMessage, std::vector<MidiMessage>, MidiMessageLess> findSourceTargetMapping(
            std::vector<MidiMessage>& sourceNotes,
            std::vector<MidiMessage>& targetNotes
            );


    int getClosestTargetNotePitch(const MidiMessage& sourceNote){
        int closestIndex = -1;
        int closestDistance = 1000000;
        for(int i = 0; i < targetNotes.size(); i++){
            int distance = std::abs(sourceNote.getNoteNumber() - targetNotes[i].getNoteNumber());
            if(distance < closestDistance){
                closestDistance = distance;
                closestIndex = i;
            }
        }
        return targetNotes[closestIndex].getNoteNumber();
    }

private:
    int targetNotesReceiveWindowFromFirstNote = 44100; // samples
    // "Timer" for window for listening to target notes in samples
    int targetNotesSampleCounter = 0;

    int bendDuration = 1000; // ms
    int bendProgress = 0;

    int channelCounter = 2; // 1 is still global channel in MPE
    std::vector<MidiMessage> activeNotes;
    std::vector<MidiMessage> sourceNotes;
    std::vector<MidiMessage> targetNotes;
    // When we start bending, this will hold the target pitches for each source note
    std::vector<int> sourceNoteTargetPitches;

    bool acceptActiveNotes = true;
    bool acceptTarget = false;
    bool isBending = false;

    // This one is connected to the message receiver thread
//    moodycamel::ReaderWriterQueue<MIDIMessage> editorQueue;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChordBenderAudioProcessor)
};



#endif //CHORD_BENDER_PLUGINPROCESSOR_H
