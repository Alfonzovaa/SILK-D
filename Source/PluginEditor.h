#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class SILKAudioProcessorEditor : public juce::AudioProcessorEditor,
    public juce::Timer
{
public:
    SILKAudioProcessorEditor(SILKAudioProcessor&);
    ~SILKAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    SILKAudioProcessor& audioProcessor;

    // ===================== CONTROLES (SLIDERS Y ATTACHMENTS) =====================
    juce::Slider saturationSlider;
    juce::Slider outGainSlider;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> saturationAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outGainAttachment;

    // ===================== BOTONES Y ATTACHMENTS =====================
    juce::ToggleButton bypassButton;
    juce::ToggleButton phaseButton;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> phaseAttachment;

    // ===================== IMÁGENES DE LA INTERFAZ =====================
    juce::Image knobImage;
    juce::Image outKnobImage;
    juce::Image backgroundImage;
    juce::Image vuImage;

    juce::Image bypassOn;
    juce::Image bypassOff;

    juce::Image phaseOn;
    juce::Image phaseOff;

    // Área guardada del vúmetro para repintado inteligente
    juce::Rectangle<int> vuArea;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SILKAudioProcessorEditor)
};