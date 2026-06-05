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
    // 1. Configuración del Slider de Saturación (Dial Central)
    saturationSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    saturationSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);

    // Vinculamos el LookAndFeel personalizado que declaramos en el .h
    saturationSlider.setLookAndFeel(&customLookAndFeel);
    addAndMakeVisible(saturationSlider);

    // VINCULACIÓN CON EL PROCESADOR: Conectamos el slider con el APVTS del audio
    saturationAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), "saturation", saturationSlider);

    // 2. Configuración del Botón Bypass (Izquierda)
    bypassButton.setButtonText("");
    addAndMakeVisible(bypassButton);

    // 3. Configuración del Botón Phase (Derecha)
    phaseButton.setButtonText("");
    addAndMakeVisible(phaseButton);

    // Definimos el tamaño cuadrado óptimo para el diseño de la interfaz
    setSize(500, 500);
}

SILKAudioProcessorEditor::~SILKAudioProcessorEditor()
{
    // Es vital desvincular el LookAndFeel en el destructor para evitar punteros colgantes
    saturationSlider.setLookAndFeel(nullptr);
}

//==============================================================================
void SILKAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Fondo oscuro mate texturizado (Color extraído de la UI de la imagen)
    g.fillAll(juce::Colour::fromString("FF1A1C1E"));

    // ---- ENCABEZADO DE LA MARCA ----
    g.setColour(juce::Colour::fromString("FFCFD3D6"));
    g.setFont(juce::Font("Helvetica", 22.0f, juce::Font::plain));
    g.drawText("BLUEHALL", 40, 35, 120, 30, juce::Justification::left);

    g.setColour(juce::Colour::fromString("FF8E9399"));
    g.setFont(juce::Font("Helvetica", 22.0f, juce::Font::plain));
    g.drawText("STUDIOS", 165, 35, 120, 30, juce::Justification::left);

    // Icono de planeta/red de Bluehall al lado del texto
    g.setColour(juce::Colour::fromString("FF8E9399"));
    g.drawEllipse(275, 42, 16, 16, 1.5f);
    g.drawEllipse(279, 42, 8, 16, 1.0f);
    g.drawLine(271, 50, 295, 50, 1.0f);

    // ---- INDICADORES DE BOTONES (Glow estático de fondo) ----
    // Led Bypass (Morado)
    int bypassX = 135;
    int bypassY = 145;
    g.setColour(juce::Colour::fromString("FF7C3AED"));
    g.drawRoundedRectangle(bypassX, bypassY, 32, 32, 4.0f, 2.0f);
    g.setColour(juce::Colour::fromString("FFA78BFA"));
    g.fillEllipse(bypassX - 15, bypassY + 11, 8, 8);

    g.setColour(juce::Colour::fromString("FF8E9399"));
    g.setFont(juce::Font("Helvetica", 13.0f, juce::Font::plain));
    g.drawText("BYPASS", bypassX - 24, bypassY + 40, 80, 20, juce::Justification::centred);

    // Led Phase (Verde)
    int phaseX = 335;
    int phaseY = 145;
    g.setColour(juce::Colour::fromString("FF10B981"));
    g.drawRoundedRectangle(phaseX, phaseY, 32, 32, 4.0f, 2.0f);
    g.setColour(juce::Colour::fromString("FF34D399"));
    g.fillEllipse(phaseX - 15, phaseY + 11, 8, 8);

    g.setColour(juce::Colour::fromString("FF8E9399"));
    g.drawText("PHASE", phaseX - 24, phaseY + 40, 80, 20, juce::Justification::centred);

    // ---- MARCAS NUMÉRICAS ALREDEDOR DEL DIAL ----
    g.setColour(juce::Colour::fromString("FF8E9399"));
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

    // Texto central "SATURATION"
    g.setColour(juce::Colour::fromString("FFD1D5DB"));
    g.setFont(juce::Font("Helvetica", 22.0f, juce::Font::plain));
    g.drawText("SATURATION", 150, 325, 200, 30, juce::Justification::centred);

    // ---- GRÁFICO DEL VU METER ----
    int vuX = 175;
    int vuY = 370;
    int vuW = 150;
    int vuH = 75;

    // Chasis interno del vúmetro
    g.setColour(juce::Colour::fromString("FF0F1113"));
    g.fillRoundedRectangle(vuX, vuY, vuW, vuH, 6.0f);
    g.setColour(juce::Colour::fromString("FF2D3139"));
    g.drawRoundedRectangle(vuX, vuY, vuW, vuH, 6.0f, 1.5f);

    // Barra de color degradada emulando los ledes (Verde -> Amarillo -> Morado)
    juce::ColourGradient vuGrad(juce::Colour::fromString("FF10B981"), vuX + 20, vuY + 40,
        juce::Colour::fromString("FF8B5CF6"), vuX + vuW - 20, vuY + 40, false);
    vuGrad.addColour(0.6, juce::Colour::fromString("FFF59E0B"));
    g.setGradientFill(vuGrad);

    juce::Path vuArc;
    vuArc.addCentredArc(vuX + vuW / 2, vuY + vuH + 10, 60, 50, 0.0f, -1.0f, 1.0f, true);
    g.strokePath(vuArc, juce::PathStrokeType(5.0f));

    g.setColour(juce::Colour::fromString("FF6B7280"));
    g.setFont(juce::Font("Helvetica", 12.0f, juce::Font::plain));
    g.drawText("VU", vuX, vuY + vuH - 22, vuW, 15, juce::Justification::centred);

    // ---- DETALLES INFERIORES ----
    g.setColour(juce::Colour::fromString("FF9CA3AF"));
    g.setFont(juce::Font("Helvetica", 26.0f, juce::Font::plain));
    g.drawText("SILK D", 40, 435, 150, 35, juce::Justification::left);

}

void SILKAudioProcessorEditor::resized()
{
    // Ubicamos los límites de los componentes interactivos
    saturationSlider.setBounds(175, 150, 150, 150);
    bypassButton.setBounds(135, 145, 32, 32);
    phaseButton.setBounds(335, 145, 32, 32);
}