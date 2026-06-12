/*
  ==============================================================================
    This file contains the basic framework code for a JUCE plugin editor.
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

// ==============================================================================
class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CustomLookAndFeel()
    {
        setColour(juce::Slider::thumbColourId, juce::Colour::fromString("FF2D2D30"));
        setColour(juce::Slider::rotarySliderFillColourId, juce::Colour::fromString("FF8B5CF6"));
        setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour::fromString("FF1E1E1E"));
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
        float sliderPosProportional, float startAngle, float endAngle,
        juce::Slider& slider) override
    {
        auto radius = juce::jmin(width, height) / 2.0f - 6.0f;
        auto centreX = x + width * 0.5f;
        auto centreY = y + height * 0.5f;
        auto rx = centreX - radius;
        auto ry = centreY - radius;
        auto rw = radius * 2.0f;
        auto angle = startAngle + sliderPosProportional * (endAngle - startAngle);

        g.setColour(slider.findColour(juce::Slider::rotarySliderOutlineColourId));
        g.drawEllipse(rx, ry, rw, rw, 3.0f);

        juce::Path fillPath;
        fillPath.addCentredArc(centreX, centreY, radius, radius, 0.0f, startAngle, angle, true);
        g.setColour(slider.findColour(juce::Slider::rotarySliderFillColourId));
        g.strokePath(fillPath, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        auto dialRadius = radius - 4.0f;
        juce::ColourGradient gradient(juce::Colour::fromString("FF3A3A3C"), centreX, centreY - dialRadius,
            juce::Colour::fromString("FF1C1C1E"), centreX, centreY + dialRadius, false);
        g.setGradientFill(gradient);
        g.fillEllipse(centreX - dialRadius, centreY - dialRadius, dialRadius * 2.0f, dialRadius * 2.0f);

        g.setColour(juce::Colour::fromString("FFA855F7"));
        g.drawEllipse(centreX - dialRadius, centreY - dialRadius, dialRadius * 2.0f, dialRadius * 2.0f, 1.0f);

        juce::Path p;
        auto pointerLength = dialRadius * 0.7f;
        auto pointerThickness = 2.0f;
        p.addRectangle(-pointerThickness * 0.5f, -dialRadius, pointerThickness, pointerLength);
        p.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));
        g.setColour(juce::Colours::white.withAlpha(0.8f));
        g.fillPath(p);
    }
};

// ==============================================================================
class SILKAudioProcessorEditor : public juce::AudioProcessorEditor,
    public juce::Timer
{
public:
    SILKAudioProcessorEditor(SILKAudioProcessor&);
    ~SILKAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    SILKAudioProcessor& audioProcessor;

    juce::Slider saturationSlider;
    juce::Slider outGainSlider; // Nueva perilla pequeña OUT
    juce::ToggleButton bypassButton;
    juce::ToggleButton phaseButton;

    CustomLookAndFeel customLookAndFeel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> saturationAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outGainAttachment; // Attachment de la nueva perilla
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> phaseAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SILKAudioProcessorEditor)
};