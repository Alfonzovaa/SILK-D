/*
  ==============================================================================
    This file contains the basic framework code for a JUCE plugin editor.
  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SILKAudioProcessorEditor::SILKAudioProcessorEditor(SILKAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    // 1. Slider de Saturación Central
    saturationSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    saturationSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    saturationSlider.setLookAndFeel(&customLookAndFeel);
    addAndMakeVisible(saturationSlider);

    saturationAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), "saturation", saturationSlider);

    // 2. NUEVA PERILLA PEQUEÑA "OUT" (Abajo a la Derecha)
    outGainSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    outGainSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    outGainSlider.setLookAndFeel(&customLookAndFeel);
    addAndMakeVisible(outGainSlider);

    outGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), "inputGain", outGainSlider);

    // 3. Botón Bypass
    bypassButton.setButtonText("");
    bypassButton.setAlpha(0.0f);
    addAndMakeVisible(bypassButton);

    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getAPVTS(), "bypass", bypassButton);

    // 4. Botón Phase
    phaseButton.setButtonText("");
    phaseButton.setAlpha(0.0f);
    addAndMakeVisible(phaseButton);

    phaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getAPVTS(), "phase", phaseButton);

    setSize(500, 500);
    startTimerHz(60);
}

SILKAudioProcessorEditor::~SILKAudioProcessorEditor()
{
    stopTimer();
    saturationSlider.setLookAndFeel(nullptr);
    outGainSlider.setLookAndFeel(nullptr);
}

//==============================================================================
void SILKAudioProcessorEditor::timerCallback()
{
    repaint();
}

//==============================================================================
void SILKAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour::fromString("FF1A1C1E"));

    // ---- ENCABEZADO ----
    g.setColour(juce::Colour::fromString("FFCFD3D6"));
    g.setFont(juce::Font("Helvetica", 22.0f, juce::Font::plain));
    g.drawText("BLUEHALL", 40, 35, 120, 30, juce::Justification::left);

    g.setColour(juce::Colour::fromString("FF8E9399"));
    g.drawText("STUDIOS", 165, 35, 120, 30, juce::Justification::left);
    g.drawEllipse(275, 42, 16, 16, 1.5f);
    g.drawEllipse(279, 42, 8, 16, 1.0f);
    g.drawLine(271, 50, 295, 50, 1.0f);

    // ---- BOTONES BYPASS Y PHASE ----
    bool bypassState = audioProcessor.getAPVTS().getRawParameterValue("bypass")->load() > 0.5f;
    bool phaseState = audioProcessor.getAPVTS().getRawParameterValue("phase")->load() > 0.5f;

    int bypassX = 95;
    int bypassY = 145;
    if (bypassState) {
        g.setColour(juce::Colour::fromString("FFA78BFA"));
        g.drawRoundedRectangle(bypassX, bypassY, 32, 32, 4.0f, 2.5f);
        g.fillEllipse(bypassX - 15, bypassY + 11, 8, 8);
    }
    else {
        g.setColour(juce::Colour::fromString("FF4C1D95"));
        g.drawRoundedRectangle(bypassX, bypassY, 32, 32, 4.0f, 1.5f);
        g.setColour(juce::Colour::fromString("FF2E1065"));
        g.fillEllipse(bypassX - 15, bypassY + 11, 8, 8);
    }
    g.setColour(juce::Colour::fromString("FF8E9399"));
    g.setFont(juce::Font("Helvetica", 13.0f, juce::Font::plain));
    g.drawText("BYPASS", bypassX - 24, bypassY + 40, 80, 20, juce::Justification::centred);

    int phaseX = 375;
    int phaseY = 145;
    if (phaseState) {
        g.setColour(juce::Colour::fromString("FF34D399"));
        g.drawRoundedRectangle(phaseX, phaseY, 32, 32, 4.0f, 2.5f);
        g.fillEllipse(phaseX - 15, phaseY + 11, 8, 8);
    }
    else {
        g.setColour(juce::Colour::fromString("FF064E3B"));
        g.drawRoundedRectangle(phaseX, phaseY, 32, 32, 4.0f, 1.5f);
        g.setColour(juce::Colour::fromString("FF022C22"));
        g.fillEllipse(phaseX - 15, phaseY + 11, 8, 8);
    }
    g.setColour(juce::Colour::fromString("FF8E9399"));
    g.drawText("MODE", phaseX - 24, phaseY + 40, 80, 20, juce::Justification::centred);

    // ---- DIAL CENTRAL NUMERACIÓN ----
    g.setFont(juce::Font("Helvetica", 16.0f, juce::Font::plain));
    g.drawText("0", 195, 305, 20, 20, juce::Justification::centred);
    g.drawText("1", 160, 260, 20, 20, juce::Justification::centred);
    g.drawText("2", 150, 210, 20, 20, juce::Justification::centred);
    g.drawText("3", 165, 165, 20, 20, juce::Justification::centred);
    g.drawText("4", 205, 135, 20, 20, juce::Justification::centred);
    g.drawText("5", 240, 125, 20, 20, juce::Justification::centred);
    g.drawText("6", 275, 135, 20, 20, juce::Justification::centred);
    g.drawText("7", 315, 165, 20, 20, juce::Justification::centred);
    g.drawText("8", 330, 210, 20, 20, juce::Justification::centred);
    g.drawText("9", 320, 260, 20, 20, juce::Justification::centred);
    g.drawText("10", 285, 305, 25, 20, juce::Justification::centred);

    g.setColour(juce::Colour::fromString("FFD1D5DB"));
    g.setFont(juce::Font("Helvetica", 22.0f, juce::Font::plain));
    g.drawText("SATURATION", 150, 325, 200, 30, juce::Justification::centred);

    // ---- DECORACIÓN PUNTOS DE LA PERILLA "OUT" ----
    int outCenterX = 425;
    int outCenterY = 405;
    g.setColour(juce::Colour::fromString("FF4B5563"));
    for (int i = 0; i < 12; ++i)
    {
        float angle = juce::jmap((float)i, 0.0f, 11.0f, -2.3f, 2.3f);
        int dotX = outCenterX + std::sin(angle) * 32.0f;
        int dotY = outCenterY - std::cos(angle) * 32.0f;
        g.fillEllipse(dotX - 1.5f, dotY - 1.5f, 3.0f, 3.0f);
    }

    // Texto "OUT" debajo de la perilla chica
    g.setColour(juce::Colour::fromString("FF9CA3AF"));
    g.setFont(juce::Font("Helvetica", 18.0f, juce::Font::plain));
    g.drawText("OUT", outCenterX - 30, outCenterY + 32, 60, 20, juce::Justification::centred);

    // ---- GRÁFICO DEL VU METER LOGARÍTMICO ----
    int vuX = 175;
    int vuY = 370;
    int vuW = 150;
    int vuH = 75;

    g.setColour(juce::Colour::fromString("FF0F1113"));
    g.fillRoundedRectangle(vuX, vuY, vuW, vuH, 6.0f);
    g.setColour(juce::Colour::fromString("FF2D3139"));
    g.drawRoundedRectangle(vuX, vuY, vuW, vuH, 6.0f, 1.5f);

    float vuValue = audioProcessor.getVULevel();
    float startAngle = -1.0f;
    float endAngle = startAngle + (vuValue * 2.0f);

    if (vuValue > 0.005f)
    {
        juce::ColourGradient vuGrad(juce::Colour::fromString("FF10B981"), vuX + 20, vuY + 40,
            juce::Colour::fromString("FF8B5CF6"), vuX + vuW - 20, vuY + 40, false);
        vuGrad.addColour(0.6, juce::Colour::fromString("FFF59E0B"));
        g.setGradientFill(vuGrad);

        juce::Path vuArc;
        vuArc.addCentredArc(vuX + vuW / 2, vuY + vuH + 10, 60, 50, 0.0f, startAngle, endAngle, true);
        g.strokePath(vuArc, juce::PathStrokeType(5.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    g.setColour(juce::Colour::fromString("FF6B7280"));
    g.setFont(juce::Font("Helvetica", 12.0f, juce::Font::plain));
    g.drawText("VU", vuX, vuY + vuH - 22, vuW, 15, juce::Justification::centred);

    // ---- TEXTO DE MARCA INFERIOR ----
    g.setColour(juce::Colour::fromString("FF9CA3AF"));
    g.setFont(juce::Font("Helvetica", 26.0f, juce::Font::plain));
    g.drawText("SILK D", 40, 435, 150, 35, juce::Justification::left);

}

void SILKAudioProcessorEditor::resized()
{
    saturationSlider.setBounds(175, 150, 150, 150);
    bypassButton.setBounds(88, 138, 45, 45);
    phaseButton.setBounds(368, 138, 45, 45);

    // Ubicación física de la nueva perilla OUT (Centro en 425, 405. Tamaño 50x50)
    outGainSlider.setBounds(400, 380, 50, 50);
}