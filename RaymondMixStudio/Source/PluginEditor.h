#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class RaymondLNF : public juce::LookAndFeel_V4
{
public:
    RaymondLNF();
    void drawRotarySlider(juce::Graphics&, int x, int y, int w, int h, float pos, float a0, float a1, juce::Slider&) override;
};

class RaymondEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit RaymondEditor(RaymondProcessor&);
    ~RaymondEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override {}

private:
    using SA = juce::AudioProcessorValueTreeState::SliderAttachment;
    using BA = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using CA = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    struct Knob { juce::Slider s; std::unique_ptr<SA> att; juce::String label; juce::Rectangle<int> r; };

    void addKnob(const juce::String& id, const juce::String& label, juce::Rectangle<int> r, std::function<juce::String(double)> fmt);
    void timerCallback() override;

    RaymondProcessor& proc;
    RaymondLNF lnf;
    std::vector<std::unique_ptr<Knob>> knobs;
    juce::ComboBox preset, rvType, dlType;
    juce::ToggleButton bypass { "BYPASS" }, rvOn { "REVERB" }, dlOn { "DELAY" }, autoT { "AUTO TEMPO" };
    juce::TextButton loadBtn { "Load Audio" }, exportBtn { "Export WAV" };
    juce::Label tempo, status;
    juce::Slider bal;
    std::unique_ptr<CA> rvA, dlA;
    std::unique_ptr<BA> byA, rvOnA, dlOnA, atA;
    std::unique_ptr<SA> balA;
    std::unique_ptr<juce::FileChooser> chooser;
    float orb = 0.f, inL = 0.f, outL = 0.f;
};
