/*
  ==============================================================================
    This file contains the basic framework code for a JUCE plugin processor.
  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
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
    apvts(*this, nullptr, "Parameters", {
        std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("saturation", 1), "Saturation", 0.0f, 10.0f, 0.0f),
        std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("inputGain", 1), "Input Gain", -24.0f, 12.0f, 0.0f), // Nueva perilla
        std::make_unique<juce::AudioParameterBool>(juce::ParameterID("bypass", 1), "Bypass", false),
        std::make_unique<juce::AudioParameterBool>(juce::ParameterID("phase", 1), "Phase Boost", false)
        })
{}

SILKAudioProcessor::~SILKAudioProcessor() {}

//==============================================================================
const juce::String SILKAudioProcessor::getName() const { return JucePlugin_Name; }
bool SILKAudioProcessor::acceptsMidi() const { return false; }
bool SILKAudioProcessor::producesMidi() const { return false; }
bool SILKAudioProcessor::isMidiEffect() const { return false; }
double SILKAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int SILKAudioProcessor::getNumPrograms() { return 1; }
int SILKAudioProcessor::getCurrentProgram() { return 0; }
void SILKAudioProcessor::setCurrentProgram(int index) {}
const juce::String SILKAudioProcessor::getProgramName(int index) { return {}; }
void SILKAudioProcessor::changeProgramName(int index, const juce::String& newName) {}

//==============================================================================
void SILKAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {}
void SILKAudioProcessor::releaseResources() {}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SILKAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    return true;
}
#endif

void SILKAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // 1. Cargar parámetros del APVTS
    bool isBypassed = apvts.getRawParameterValue("bypass")->load() > 0.5f;
    bool isPhaseBoostActive = apvts.getRawParameterValue("phase")->load() > 0.5f;
    auto saturationValue = apvts.getRawParameterValue("saturation")->load();

    // Convertir los decibelios de la nueva perilla OUT a ganancia lineal
    auto inputGainDB = apvts.getRawParameterValue("inputGain")->load();
    float inputGainLinear = juce::Decibels::decibelsToGain(inputGainDB);

    if (isBypassed)
    {
        saturationValue = 0.0f;
    }

    float drive = 1.0f + (saturationValue * 2.0f);

    if (isPhaseBoostActive && !isBypassed)
    {
        drive *= 2.5f;
    }

    float gainCompensation = 1.0f / std::sqrt(drive);

    // 2. Bucle de procesamiento de audio
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            // Aplicar el control de volumen OUT a la entrada
            float inputSample = channelData[sample] * inputGainLinear;

            if (isBypassed)
            {
                channelData[sample] = inputSample;
            }
            else
            {
                float drivenSample = inputSample * drive;
                float saturatedSample = std::tanh(drivenSample);
                channelData[sample] = saturatedSample * gainCompensation;
            }
        }
    }

    // ---- CÁLCULO DEL VU METER LOGARÍTMICO (MÁS SENSIBLE) ----
    float maxPeak = 0.0f;
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        float peak = buffer.getMagnitude(channel, 0, buffer.getNumSamples());
        if (peak > maxPeak)
            maxPeak = peak;
    }

    // Convertimos a Decibelios para que sea mucho más sensible a sonidos bajos
    float peakDB = juce::Decibels::gainToDecibels(maxPeak);

    // Mapeamos un rango útil de -40 dB (vacio) hasta 0 dB (lleno total) a una escala de 0.0 a 1.0
    float normalizedVU = juce::jmap(peakDB, -40.0f, 0.0f, 0.0f, 1.0f);
    normalizedVU = juce::jlimit(0.0f, 1.0f, normalizedVU);

    // Aplicar decaimiento suave (Ballistics)
    float currentVU = vuLevel.load();
    if (normalizedVU > currentVU)
    {
        vuLevel.store(normalizedVU);
    }
    else
    {
        vuLevel.store(currentVU * 0.88f); // Decaimiento fluido
    }
}

//==============================================================================
bool SILKAudioProcessor::hasEditor() const { return true; }
juce::AudioProcessorEditor* SILKAudioProcessor::createEditor() { return new SILKAudioProcessorEditor(*this); }
void SILKAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {}
void SILKAudioProcessor::setStateInformation(const void* data, int sizeInBytes) {}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new SILKAudioProcessor(); }