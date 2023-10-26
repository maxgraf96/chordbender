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
    juce::MidiBuffer keepMidiMessages;

    if(isBending){
        // Bend start
        if(bendProgress == 0){
            // We're done receiving target notes
            // Let's figure out the bends
            // For each source note, we need to find the closest target note
            sourceNoteTargetPitches.clear();
            for(const auto& sourceNote : sourceNotes){
                int targetNotePitch = getClosestTargetNotePitch(sourceNote);
                sourceNoteTargetPitches.push_back(targetNotePitch);
            }
        }
        // We're bending
        // For each source note, send the adequate pitch bend message, depending on the bendProgress / bendDuration
        for(int i = 0; i < sourceNotes.size(); i++){
            // Get the source note
            const auto sourceNote = sourceNotes[i];
            // Get the target pitch
            const auto targetPitch = sourceNoteTargetPitches[i];
            // Get the current pitch
            const auto sourcePitch = sourceNote.getNoteNumber();
            // Get the channel
            const auto channel = sourceNote.getChannel();
            // Calculate the pitch bend value
            const auto pitchBendDiffSemitones = targetPitch - sourcePitch;

            const auto isBendingUp = pitchBendDiffSemitones > 0;

            // map pitchBendDiffSemitones to the actual pitch wheel value, considering that we can bend
            // +/- 12 semitones
            const int pitchBendTarget = 8191 + pitchBendDiffSemitones * 8192 / 12;

            auto bendProgressNormalized = bendProgress / (double) bendDuration;
            // clamp bendProgressNormalized to [0, 1]
            if(bendProgressNormalized > 1){
                bendProgressNormalized = 1;
            } else if(bendProgressNormalized < 0){
                bendProgressNormalized = 0;
            }

            MidiMessage pitchBendMessage;
            if(isBendingUp){
                // We're bending up
                // Map from [0, bendDuration] to [8192, pitchBendTarget]
                // TODO proper map function
                const auto pitchBendValueMapped = (int) (1.0f - bendProgressNormalized) * 8191 + (pitchBendTarget * bendProgressNormalized);
                // Create the pitch bend message
                pitchBendMessage = juce::MidiMessage::pitchWheel(channel, pitchBendValueMapped);
            } else {
                // We're bending down
                // Map from [0, bendDuration] to [8192, pitchBendTarget]
                const auto pitchBendValueMapped = (int) ((1 - bendProgressNormalized) * 8192);
                // Create the pitch bend message
                pitchBendMessage = juce::MidiMessage::pitchWheel(channel, pitchBendValueMapped);
            }

            // Add the pitch bend message to the buffer
            keepMidiMessages.addEvent(pitchBendMessage, 0);
        }

        // Update the bend progress
        bendProgress += (int) ((float)buffer.getNumSamples() / 44100.0f * 1000.0f);
        if(bendProgress > bendDuration){
            // We're done bending
            isBending = false;
            activeNotes.clear();
            // Clear the source notes
            sourceNotes.clear();
            // Clear the target notes
            targetNotes.clear();
            // Clear the source note target pitches
            sourceNoteTargetPitches.clear();
            // We're now accepting active notes
            acceptActiveNotes = true;
            // We're no longer accepting target notes
            acceptTarget = false;
        }

        // Swap midi buffers
        midiMessages.swapWith(keepMidiMessages);

        // Clear audio buffer
        buffer.clear();
        return;
    }

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

            if(acceptActiveNotes){
                activeNotes.push_back(newMessage);
                keepMidiMessages.addEvent(msg, metadata.samplePosition);
            } else {
                if(targetNotes.empty()){
                    // From the point where we receive the first target note, we start a timer
                    // When that timer is done, we bend the notes
                    targetNotesSampleCounter = 0;
                }
                targetNotes.push_back(newMessage);
            }
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

            // This block here is just for when we're in the state of receiving target notes, but the user
            // decides to discard one of them. In that case, we need to remove the note from the targetNotes
            // vector.
            if(acceptTarget){
                for(auto it = targetNotes.begin(); it != targetNotes.end(); ++it){
                    if(midiEqual(*it, newMessage)){
                        targetNotes.erase(it);
                        break;
                    }
                }
                continue;
            }

            for(auto it = activeNotes.begin(); it != activeNotes.end(); ++it){
                if(midiEqual(*it, newMessage)){
                    activeNotes.erase(it);
                    sourceNotes.push_back(newMessage);
                    if(activeNotes.empty()){
                        // So, receiving target notes now
                        targetNotes.clear();
                        acceptActiveNotes = false;
                        acceptTarget = true;
                    }
                    break;
                }
            }
        }
    }

    if(acceptTarget){
        if(targetNotes.empty()){
            targetNotesSampleCounter = 0;
        } else {
            targetNotesSampleCounter += buffer.getNumSamples();
            if(targetNotesSampleCounter >= targetNotesReceiveWindowFromFirstNote){
                // Time's up, got some target notes, let's bend!
                acceptTarget = false;
                bendProgress = 0;
                isBending = true;
            }
        }
    }

    // Swap midiMessages with keepMidiMessages
    midiMessages.swapWith(keepMidiMessages);


    // Clear audio buffer
    buffer.clear();
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
