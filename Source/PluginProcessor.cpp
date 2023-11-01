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
), apvts(*this, nullptr, "PARAMETERS", createParameterLayout())
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

    auto* playhead = getPlayHead();
    if(playhead != nullptr){
        if(playhead->getPosition().hasValue()){
            if(!playhead->getPosition()->getIsPlaying()){
                // Reset everything
                activeNotes.clear();
                sourceNotes.clear();
                targetNotes.clear();
                sourceNoteTargetPitches.clear();
                acceptActiveNotes = true;
                acceptTarget = false;
                isBending = false;
            }
        }
    }

    if(isBending){
        // Send master pitch bend message
        auto masterPitchBendMessage = juce::MidiMessage::pitchWheel(1, 8192);
        keepMidiMessages.addEvent(masterPitchBendMessage, 0);
        // Bend start
        if(bendProgress == 0){
            sourceNoteTargetPitches.clear();
            // We're done receiving target notes
            // Let's figure out the bends
            // For each source note, we need to find the matching target note
            std::map<MidiMessage, std::vector<MidiMessage>, ChordBenderAudioProcessor::MidiMessageLess> mapping = findSourceTargetMapping(sourceNotes, targetNotes);

            // Let's start with the simple case that we have 1 source note and 2 target notes
            // Let's assume 2 cases:
            // 1. sourceNotes.size() >= targetNotes.size()
            if(sourceNotes.size() >= targetNotes.size()){
                for(auto& sourceNote : sourceNotes){
                    sourceNoteTargetPitches.push_back(mapping[sourceNote][0].getNoteNumber());
                }
            }

            // 2. sourceNotes.size() < targetNotes.size()
            // In this case, we need to spawn new source notes, and map them to the target notes
            if(sourceNotes.size() < targetNotes.size()){
                for(auto& sourceNote : sourceNotes){
                    if(mapping[sourceNote].size() == 1){
                        // This source note has only one target note, so we can just use that
                        sourceNoteTargetPitches.push_back(mapping[sourceNote][0].getNoteNumber());
                    } else {
                        // This source note has multiple target notes, so we need to spawn new source notes
                        // and map them to the target notes
                        int toSpawnRemaining = mapping[sourceNote].size() - 1;
                        for(auto& targetNote : mapping[sourceNote]){
                            // Add the target note pitch to the source note target pitches vector
                            sourceNoteTargetPitches.push_back(targetNote.getNoteNumber());

                            if(toSpawnRemaining > 0){
                                // Create a new source note
                                auto channel = channelCounter++;
                                if(channelCounter > 16)
                                    channelCounter = 2;

                                auto srcNoteNumber = sourceNote.getNoteNumber();
                                auto vel = sourceNote.getVelocity();
                                juce::MidiMessage newSourceNote = juce::MidiMessage::noteOn(channel, srcNoteNumber, sourceNote.getVelocity());
                                // Add the new source note to the source notes vector
                                sourceNotes.push_back(newSourceNote);
                                // Add the target note to the target notes vector
                                targetNotes.push_back(targetNote);
                                // Add midi message to out so it's played
                                keepMidiMessages.addEvent(newSourceNote, 0);
                                toSpawnRemaining--;
                            }
                        }
                    }
                }
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
            // Get the channel from sourceChannels
            const auto channel = sourceNote.getChannel();
            // Calculate the pitch bend value
            const auto pitchBendDiffSemitones = targetPitch - sourcePitch;

            // map pitchBendDiffSemitones to the actual pitch wheel value, considering that we can bend
            // +/- 48 semitones (MPE default)
            const int pitchBendTarget = (int) (((float)pitchBendDiffSemitones / 48.0f * 8192.0f) + 8191.0f);

            auto bendProgressNormalized = bendProgress / (double) bendDuration;
            // clamp bendProgressNormalized to [0, 1]
            if(bendProgressNormalized > 1){
                bendProgressNormalized = 1;
            }

            const auto pitchBendValueMapped = (int) mapFloat(
                    (float) bendProgressNormalized,
                    0.0f, 1.0f,
                    8192.0f,
                    (float) pitchBendTarget
            );

            // Construct pitch bend message manually
            auto statusByte = 0xE0 | (channel - 1);
            auto lsb = pitchBendValueMapped & 0x7F;
            auto msb = (pitchBendValueMapped >> 7) & 0x7F;

            auto pitchBendMessage = juce::MidiMessage(statusByte, lsb, msb);
            keepMidiMessages.addEvent(pitchBendMessage, 0);
        }

        // Update the bend progress
        auto bufferUpdateRateMS = (int)((float)buffer.getNumSamples() / (float) getSampleRate() * 1000.0f);
        bendProgress += bufferUpdateRateMS;
        if(bendProgress >= bendDuration){
            // We're done bending
            isBending = false;
            activeNotes.clear();
            // Clear the source notes
            sourceNotes.clear();

            // First, all source notes off
            for(int i = 2; i < 16; i++){
                auto noteOffMessage = juce::MidiMessage::allNotesOff(i);
                keepMidiMessages.addEvent(noteOffMessage, 0);
            }
            // Previous target notes become new source notes -> note on
            for(auto& sourceNoteTargetPitch : sourceNoteTargetPitches){
                auto channel = channelCounter++;
                if(channelCounter > 16)
                    channelCounter = 2;
                auto targetNoteNumber = sourceNoteTargetPitch;
                uint8 vel = 70;
                juce::MidiMessage newSourceNotePBReset = juce::MidiMessage::pitchWheel(channel, 8192);
                juce::MidiMessage newSourceNote = juce::MidiMessage::noteOn(channel, targetNoteNumber, vel);
                sourceNotes.push_back(newSourceNote);
                keepMidiMessages.addEvent(newSourceNotePBReset, 0);
                keepMidiMessages.addEvent(newSourceNote, 0);
            }

            // Clear the target notes
            targetNotes.clear();
            // Clear the source note target pitches
            sourceNoteTargetPitches.clear();
            // We're now accepting active notes
            acceptActiveNotes = false;
            // We're no longer accepting target notes
            acceptTarget = true;
        } else {
            // Swap midi buffers
            midiMessages.swapWith(keepMidiMessages);

            // Clear audio buffer
            buffer.clear();
            return;
        }
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
            const auto channel = channelCounter;
            channelCounter++;
            if(channelCounter > 16)
                channelCounter = 2;
            // Create a new note-on message
            juce::MidiMessage noteOnMessage = juce::MidiMessage::noteOn(channel, noteNumber, velocity);

            if(acceptActiveNotes){
                if(activeNotes.empty()){
                    // Before the first new active note, reset pitch bend on all channels
                    for(int i = 2; i < 16; i++){
                        auto pitchBendResetMessage = juce::MidiMessage::pitchWheel(i, 8192);
                        keepMidiMessages.addEvent(pitchBendResetMessage, 0);
                    }
                }
                auto pitchBendResetMessage = juce::MidiMessage::pitchWheel(channel, 8192);
                activeNotes.push_back(noteOnMessage);
                keepMidiMessages.addEvent(pitchBendResetMessage, 0);
                keepMidiMessages.addEvent(noteOnMessage, 0);
            } else {
                if(targetNotes.empty()){
                    // From the point where we receive the first target note, we start a timer
                    // When that timer is done, we bend the notes
                    targetNotesSampleCounter = 0;
                }
                targetNotes.push_back(noteOnMessage);
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

            // Convert active notes to source notes
            for(auto it = activeNotes.begin(); it != activeNotes.end(); ++it){
                if(midiEqual(*it, newMessage)){
                    // Get channel from active note
                    int channel = (*it).getChannel();
                    activeNotes.erase(it);
                    // Source notes must be in round robin fashion, midi message type doesn't matter
                    // we just neet to keep track of the note number and the channel
                    juce::MidiMessage srcMessage = juce::MidiMessage::noteOn(channel, noteNumber, velocity);
                    sourceNotes.push_back(srcMessage);

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

std::map<MidiMessage, std::vector<MidiMessage>, ChordBenderAudioProcessor::MidiMessageLess> ChordBenderAudioProcessor::findSourceTargetMapping(
        std::vector<MidiMessage>& sourceNotes,
        std::vector<MidiMessage>& targetNotes
) {
    std::map<MidiMessage, std::vector<MidiMessage>, ChordBenderAudioProcessor::MidiMessageLess> mapping;

    // Sort both vectors based on note number
    std::sort(sourceNotes.begin(), sourceNotes.end(), [](const MidiMessage& a, const MidiMessage& b) {
        return a.getNoteNumber() < b.getNoteNumber();
    });
    std::sort(targetNotes.begin(), targetNotes.end(), [](const MidiMessage& a, const MidiMessage& b) {
        return a.getNoteNumber() < b.getNoteNumber();
    });

    if (sourceNotes.size() >= targetNotes.size()) {
        std::vector<bool> sourceMapped(sourceNotes.size(), false);

        // For each target note, find the closest unmapped source note
        for (const MidiMessage& target : targetNotes) {
            int minDistance = INT_MAX;
            int closestIndex = -1;

            for (size_t i = 0; i < sourceNotes.size(); ++i) {
                if (!sourceMapped[i]) {
                    int distance = std::abs(target.getNoteNumber() - sourceNotes[i].getNoteNumber());
                    if (distance < minDistance) {
                        minDistance = distance;
                        closestIndex = i;
                    }
                }
            }

            if (closestIndex != -1) {
                mapping[sourceNotes[closestIndex]].push_back(target);
                sourceMapped[closestIndex] = true;
            }
        }

        // For unmapped source notes, map to the closest target note
        for (size_t i = 0; i < sourceNotes.size(); ++i) {
            if (!sourceMapped[i]) {
                MidiMessage closest = targetNotes[0];
                int minDistance = std::abs(sourceNotes[i].getNoteNumber() - closest.getNoteNumber());

                for (const MidiMessage& target : targetNotes) {
                    int distance = std::abs(sourceNotes[i].getNoteNumber() - target.getNoteNumber());
                    if (distance < minDistance) {
                        closest = target;
                        minDistance = distance;
                    }
                }

                mapping[sourceNotes[i]].push_back(closest);
            }
        }
    } else {
        // Divide the target notes into segments based on the number of source notes
        size_t segmentSize = targetNotes.size() / sourceNotes.size();
        size_t remainder = targetNotes.size() % sourceNotes.size();

        size_t targetIndex = 0;
        for (const MidiMessage& source : sourceNotes) {
            std::vector<MidiMessage> segment;
            for (size_t i = 0; i < segmentSize; ++i, ++targetIndex) {
                segment.push_back(targetNotes[targetIndex]);
            }

            // Distribute the remainder among the segments
            if (remainder > 0) {
                segment.push_back(targetNotes[targetIndex]);
                ++targetIndex;
                --remainder;
            }

            mapping[source] = segment;
        }
    }

    return mapping;
}

AudioProcessorValueTreeState::ParameterLayout ChordBenderAudioProcessor::createParameterLayout(){
    std::vector<std::unique_ptr<RangedAudioParameter>> params;
    params.push_back(std::make_unique<AudioParameterInt>("bendDuration", "Bend Duration", 100, 3000, 250));
    return {params.begin(), params.end()};
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
