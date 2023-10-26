//
// Created by Max on 21/10/2023.
//

#include "PluginProcessor.h"
#include "PluginEditor.h"

ChordBenderAudioProcessor::ChordBenderAudioProcessor()
        : AudioProcessor (BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
#endif
)
{
}

ChordBenderAudioProcessor::~ChordBenderAudioProcessor() {

}

const juce::String ChordBenderAudioProcessor::getName() const {
    return JucePlugin_Name;
}

bool ChordBenderAudioProcessor::acceptsMidi() const {
    return true;
}

bool ChordBenderAudioProcessor::producesMidi() const {
    return true;
}

bool ChordBenderAudioProcessor::isMidiEffect() const {
    return true;
}

double ChordBenderAudioProcessor::getTailLengthSeconds() const {
    return 0.0;
}

int ChordBenderAudioProcessor::getNumPrograms() {
    return 1;
}

int ChordBenderAudioProcessor::getCurrentProgram() {
    return 0;
}

void ChordBenderAudioProcessor::setCurrentProgram(int index) {
}

const juce::String ChordBenderAudioProcessor::getProgramName(int index) {
    return {};
}

void ChordBenderAudioProcessor::changeProgramName(int index, const juce::String &newName) {
}

void ChordBenderAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
}

void ChordBenderAudioProcessor::releaseResources() {

}

void ChordBenderAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages) {
    // Process incoming MIDI messages and generate output MIDI messages
    // Look for MIDI messages in the queue

    // Look for note ons
    for (const auto metadata : midiMessages) {
        const auto msg = metadata.getMessage();
        if (msg.isNoteOn()) {
            // Get the note number
            const auto noteNumber = msg.getNoteNumber();
            // Get the velocity
            const auto velocity = msg.getVelocity();
            // Get the channel
            const auto channel = msg.getChannel();
            // Create a new note on message
            juce::MidiMessage newMessage = juce::MidiMessage::noteOn(channel, noteNumber, velocity);

            activeNotes.push_back(newMessage);
        }
        if (msg.isNoteOff()){
            // Get the note number
            const auto noteNumber = msg.getNoteNumber();
            // Get the velocity
            const auto velocity = msg.getVelocity();
            // Get the channel
            const auto channel = msg.getChannel();
            // Create a new note on message
            juce::MidiMessage newMessage = juce::MidiMessage::noteOff(channel, noteNumber, velocity);

            for(auto it = activeNotes.begin(); it != activeNotes.end(); ++it){
                if(midiEqual(*it, newMessage)){
                    activeNotes.erase(it);
                    sourceNotes.push_back(newMessage);

                    // Start timer here

                    break;
                }
            }
        }
    }


    // Clear audio buffer
    buffer.clear();
}

void ChordBenderAudioProcessor::hiResTimerCallback() {

    stopTimer();
}

bool ChordBenderAudioProcessor::hasEditor() const {
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor *ChordBenderAudioProcessor::createEditor() {
    return new ChordBenderAudioProcessorEditor(*this);
}

void ChordBenderAudioProcessor::getStateInformation(juce::MemoryBlock &destData) {
    // Store the current state of your plugin in the destData object for saving presets or sessions
}

void ChordBenderAudioProcessor::setStateInformation(const void *data, int sizeInBytes) {
    // Restore the state of your plugin from the data object when loading presets or sessions
}

// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
    return new ChordBenderAudioProcessor();
}
