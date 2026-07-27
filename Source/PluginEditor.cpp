#include "PluginProcessor.h"
#include "PluginEditor.h"

SILKAudioProcessorEditor::SILKAudioProcessorEditor(SILKAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setSize(500, 500);

    // ===================== SATURATION (DRIVE) =====================
    saturationSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    saturationSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    saturationSlider.setMouseDragSensitivity(300);
    saturationSlider.setAlpha(0.0f);
    addAndMakeVisible(saturationSlider);

    saturationAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), "saturation", saturationSlider);

    // ===================== OUT =====================
    outGainSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    outGainSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    outGainSlider.setMouseDragSensitivity(150);
    outGainSlider.setAlpha(0.0f);
    addAndMakeVisible(outGainSlider);

    outGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), "outGain", outGainSlider);

    // ===================== BUTTONS =====================
    addAndMakeVisible(bypassButton);
    addAndMakeVisible(phaseButton);

    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getAPVTS(), "bypass", bypassButton);

    phaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getAPVTS(), "phase", phaseButton);

    // ===================== ESTADO INICIAL DE BOTONES =====================
    phaseButton.setToggleState(false, juce::dontSendNotification);
    bypassButton.setAlpha(0.0f);
    phaseButton.setAlpha(0.0f);

    // ===================== CARGA DE IMÁGENES =====================
    knobImage = juce::ImageCache::getFromMemory(
        BinaryData::Knob_Saturation_png,
        BinaryData::Knob_Saturation_pngSize
    );

    outKnobImage = juce::ImageCache::getFromMemory(
        BinaryData::PERILLA_LOW_1_png,
        BinaryData::PERILLA_LOW_1_pngSize
    );

    backgroundImage = juce::ImageCache::getFromMemory(
        BinaryData::BACKGROUND_png,
        BinaryData::BACKGROUND_pngSize
    );

    vuImage = juce::ImageCache::getFromMemory(
        BinaryData::VU_png,
        BinaryData::VU_pngSize
    );

    bypassOn = juce::ImageCache::getFromMemory(BinaryData::BYPASS_ON_png, BinaryData::BYPASS_ON_pngSize);
    bypassOff = juce::ImageCache::getFromMemory(BinaryData::BYPASS_OFF_png, BinaryData::BYPASS_OFF_pngSize);

    phaseOn = juce::ImageCache::getFromMemory(BinaryData::MODE_ON_png, BinaryData::MODE_ON_pngSize);
    phaseOff = juce::ImageCache::getFromMemory(BinaryData::MODE_HARD_png, BinaryData::MODE_HARD_pngSize);

    startTimerHz(50);
}

SILKAudioProcessorEditor::~SILKAudioProcessorEditor()
{
    stopTimer();
}

void SILKAudioProcessorEditor::timerCallback()
{
    repaint(vuArea);
}

void SILKAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);

    // ===================== BACKGROUND =====================
    if (backgroundImage.isValid())
        g.drawImage(backgroundImage, getLocalBounds().toFloat());
    else
        g.fillAll(juce::Colours::black);

    float angleStart = juce::MathConstants<float>::pi * 0.0f;
    float angleEnd = angleStart + juce::MathConstants<float>::pi * 1.61f;

    // ===================== SATURATION KNOB (DRIVE) =====================
    const float centerX = 251.0f;
    const float centerY = 203.0f;
    const float knobSize = 170.0f;

    auto knobBounds = juce::Rectangle<float>(
        centerX - knobSize / 2.0f,
        centerY - knobSize / 2.0f,
        knobSize,
        knobSize
    );

    g.saveState();

    float value = (float)saturationSlider.getValue();
    float angle = juce::jmap(value,
        (float)saturationSlider.getMinimum(),
        (float)saturationSlider.getMaximum(),
        angleStart,
        angleEnd);

    g.addTransform(juce::AffineTransform::rotation(
        angle,
        knobBounds.getCentreX(),
        knobBounds.getCentreY()
    ));

    if (knobImage.isValid())
        g.drawImage(knobImage, knobBounds, juce::RectanglePlacement::centred);

    g.restoreState();

    // ===================== OUT KNOB =====================
    auto outBounds = outGainSlider.getBounds().toFloat();

    g.saveState();

    float outValue = (float)outGainSlider.getValue();
    float outAngle = juce::jmap(outValue,
        (float)outGainSlider.getMinimum(),
        (float)outGainSlider.getMaximum(),
        angleStart,
        angleEnd);

    g.addTransform(juce::AffineTransform::rotation(
        outAngle,
        outBounds.getCentreX(),
        outBounds.getCentreY()
    ));

    if (outKnobImage.isValid())
        g.drawImage(outKnobImage, outBounds, juce::RectanglePlacement::centred);

    g.restoreState();

    // ===================== BYPASS DIBUJO DE IMÁGENES =====================
    {
        auto b = bypassButton.getBounds().toFloat();
        bool state = bypassButton.getToggleState();

        g.drawImage(state ? bypassOn : bypassOff,
            b,
            juce::RectanglePlacement::centred);
    }

    // ===================== MODE DIBUJAR IMÁGENES =====================
    {
        auto p = phaseButton.getBounds().toFloat();

        bool isHard = audioProcessor.getAPVTS()
            .getRawParameterValue("phase")->load() > 0.5f;

        const juce::Image& modeImg =
            isHard
            ? juce::ImageCache::getFromMemory(BinaryData::MODE_HARD_png, BinaryData::MODE_HARD_pngSize)
            : juce::ImageCache::getFromMemory(BinaryData::MODE_ON_png, BinaryData::MODE_ON_pngSize);

        if (modeImg.isValid())
            g.drawImage(modeImg,
                p,
                juce::RectanglePlacement::centred);
    }

    // =========================================================================
    // VÚMETRO: BARRA OPTIMIZADA Y REDUCIDA EN PROPORCIÓN
    // =========================================================================
    float currentLevel = audioProcessor.getVULevel();

    if (vuImage.isValid())
    {
        g.saveState();

        // Recorte protector proporcional para el tamaño más compacto
        g.reduceClipRegion(juce::Rectangle<int>(vuArea.getX() + 3, vuArea.getY(), vuArea.getWidth() - 6, vuArea.getHeight()));

        // Proporciones de márgenes internos reducidas para la nueva escala
        float usableWidth = (float)vuArea.getWidth() - 20.0f;
        float barWidthCurrent = juce::jmap(currentLevel, 0.0f, 1.0f, 0.0f, usableWidth);

        // Ajustamos la barra a la escala del nuevo cuadro reducido (10px de sangría lateral, 8px arriba)
        juce::Rectangle<float> ledBar((float)vuArea.getX() + 10.0f, (float)vuArea.getY() + 8.0f, barWidthCurrent, (float)vuArea.getHeight() - 16.0f);

        juce::ColourGradient ledGradient;
        ledGradient.point1 = { (float)vuArea.getX() + 10.0f, (float)vuArea.getY() };
        ledGradient.point2 = { (float)vuArea.getX() + 10.0f + usableWidth, (float)vuArea.getY() };
        ledGradient.isRadial = false;

        ledGradient.addColour(0.0, juce::Colour::fromRGBA(34, 214, 52, 255));
        ledGradient.addColour(0.55, juce::Colour::fromRGBA(242, 227, 46, 255));
        ledGradient.addColour(0.80, juce::Colour::fromRGBA(169, 39, 245, 255));

        g.setGradientFill(ledGradient);
        g.fillRect(ledBar);

        g.restoreState();

        g.drawImage(vuImage, vuArea.toFloat(), juce::RectanglePlacement::stretchToFit);
    }
}

void SILKAudioProcessorEditor::resized()
{
    saturationSlider.setBounds(166, 118, 170, 170);
    outGainSlider.setBounds(420, 409, 45, 45);

    bypassButton.setBounds(55, 126, 45, 45);
    phaseButton.setBounds(396, 126, 45, 45);

    // NUEVO TAMAÑO COMPACTO: Ancho 180, Alto 92. 
    vuArea.setBounds(160, 350, 180, 92);
}