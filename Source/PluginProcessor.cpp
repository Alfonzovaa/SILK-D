#include "PluginProcessor.h"
#include "PluginEditor.h"

// =======================================================
// CONSTRUCTOR (VERSIÓN SEGURA APVTS)
// =======================================================

SILKAudioProcessor::SILKAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
#endif
    apvts(*this, nullptr, "Parameters",
        {
            std::make_unique<juce::AudioParameterFloat>("saturation", "Saturation", 0.0f, 10.0f, 0.0f),
            std::make_unique<juce::AudioParameterFloat>("inputGain", "Input Gain", -24.0f, 12.0f, 0.0f),
            std::make_unique<juce::AudioParameterFloat>("outGain", "Output Gain", -24.0f, 12.0f, 0.0f),
            std::make_unique<juce::AudioParameterBool>("bypass", "Bypass", false),
            std::make_unique<juce::AudioParameterBool>("phase", "Mode Switch", false)
        })
{
}

SILKAudioProcessor::~SILKAudioProcessor() {}

// =======================================================
// BASIC INFO
// =======================================================

const juce::String SILKAudioProcessor::getName() const { return JucePlugin_Name; }
bool SILKAudioProcessor::acceptsMidi() const { return false; }
bool SILKAudioProcessor::producesMidi() const { return false; }
bool SILKAudioProcessor::isMidiEffect() const { return false; }
double SILKAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int SILKAudioProcessor::getNumPrograms() { return 1; }
int SILKAudioProcessor::getCurrentProgram() { return 0; }
void SILKAudioProcessor::setCurrentProgram(int) {}
const juce::String SILKAudioProcessor::getProgramName(int) { return {}; }
void SILKAudioProcessor::changeProgramName(int, const juce::String&) {}

// =======================================================
// SETUP
// =======================================================

void SILKAudioProcessor::prepareToPlay(double, int) {}
void SILKAudioProcessor::releaseResources() {}

bool SILKAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::mono()
        || layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

// =======================================================
// 🔥 SATURACIÓN FINAL (PRO ANALOG STYLE)
// =======================================================

static inline float analogSaturator(float x, float drive, bool hardMode)
{
    x *= drive;

    // SOFT MODE → tape warm / smooth glue
    if (!hardMode)
    {
        return std::tanh(x * 0.75f);
    }

    // HARD MODE → tube + asymmetry (más armónicos reales)
    float even = std::tanh(x * 1.2f);
    float odd = x / (1.0f + 0.25f * std::abs(x));

    return (even * 0.6f) + (odd * 0.4f);
}

// =======================================================
// PROCESS BLOCK
// =======================================================

void SILKAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    bool isBypassed = apvts.getRawParameterValue("bypass")->load() > 0.5f;
    bool isModeActive = apvts.getRawParameterValue("phase")->load() > 0.5f;

    float saturationValue = apvts.getRawParameterValue("saturation")->load();

    float inputGain = juce::Decibels::decibelsToGain(
        apvts.getRawParameterValue("inputGain")->load());

    float outGain = juce::Decibels::decibelsToGain(
        apvts.getRawParameterValue("outGain")->load());

    if (isBypassed)
        saturationValue = 0.0f;

    // 🔥 DRIVE EXPONENCIAL (nivel hardware real)
    float drive = std::pow(10.0f, saturationValue * 0.12f);

    if (isModeActive && !isBypassed)
        drive *= 2.0f;

    float gainComp = 1.0f / std::sqrt(drive);

    // ===================================================
    // DSP LOOP
    // ===================================================

    for (int ch = 0; ch < totalNumInputChannels; ++ch)
    {
        auto* data = buffer.getWritePointer(ch);

        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            float x = data[i] * inputGain;

            if (isBypassed)
            {
                data[i] = x * outGain;
            }
            else
            {
                float y = analogSaturator(x, drive, isModeActive);
                data[i] = y * gainComp * outGain;
            }
        }
    }

    // ===================================================
    // VU METER
    // ===================================================

    float maxPeak = 0.0f;

    for (int ch = 0; ch < totalNumInputChannels; ++ch)
    {
        float peak = buffer.getMagnitude(ch, 0, buffer.getNumSamples());
        if (peak > maxPeak)
            maxPeak = peak;
    }

    float peakDB = juce::Decibels::gainToDecibels(maxPeak);

    if (!isBypassed && saturationValue > 0.1f)
        peakDB += saturationValue * 1.2f;

    float vu = juce::jmap(peakDB, -36.0f, 3.0f, 0.0f, 1.0f);
    vu = juce::jlimit(0.0f, 1.0f, vu);

    float current = vuLevel.load();

    if (vu > current)
        vuLevel.store(vu);
    else
        vuLevel.store(current * 0.88f);
}

// =======================================================
// STATE SAVE / LOAD
// =======================================================

void SILKAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());

    if (xml)
        copyXmlToBinary(*xml, destData);
}

void SILKAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState && xmlState->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

// =======================================================

bool SILKAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* SILKAudioProcessor::createEditor()
{
    return new SILKAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SILKAudioProcessor();
}