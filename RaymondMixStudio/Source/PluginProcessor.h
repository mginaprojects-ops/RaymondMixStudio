#pragma once
#include <JuceHeader.h>
#include "Engine.h"
#include "Presets.h"

class RaymondProcessor : public juce::AudioProcessor
{
public:
    RaymondProcessor();
    ~RaymondProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "Raymond Mix Studio"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 4.0; }

    int getNumPrograms() override { return kNumPresets; }
    int getCurrentProgram() override { return program; }
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override { return kPresets[juce::jlimit(0, kNumPresets - 1, index)].name; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // File workflow: import audio -> auto BPM -> process -> export WAV
    double loadAudio(const juce::File& file);      // returns detected BPM or -1 on failure
    bool renderToWav(const juce::File& out);
    bool hasAudio() const { return loaded.getNumSamples() > 0; }

    juce::AudioProcessorValueTreeState apvts;
    std::atomic<float> inLevel { 0.f }, outLevel { 0.f };
    std::atomic<double> currentBpm { 120.0 };
    std::atomic<int> bpmSource { 0 }; // 0 = manual/auto-from-file, 1 = host (DAW)

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout makeLayout();
    Params gather();
    Engine engine;
    int program = 0;
    juce::AudioBuffer<float> loaded;
    double loadedSr = 44100.0;
};
