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
    // Inicialización del árbol de parámetros vinculando el ID "saturation"
    apvts(*this, nullptr, "Parameters", {
        std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID("saturation", 1),
            "Saturation",
            0.0f,
            10.0f,
            0.0f
        )
        })
{}

SILKAudioProcessor::~SILKAudioProcessor()
{}

//==============================================================================
const juce::String SILKAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SILKAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool SILKAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool SILKAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double SILKAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SILKAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int SILKAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SILKAudioProcessor::setCurrentProgram(int index)
{}

const juce::String SILKAudioProcessor::getProgramName(int index)
{
    return {};
}

void SILKAudioProcessor::changeProgramName(int index, const juce::String& newName)
{}

//==============================================================================
void SILKAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void SILKAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SILKAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

void SILKAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // 1. Cargar el valor actual de la perilla desde el hilo de audio (Rango 0.0 a 10.0)
    auto saturationValue = apvts.getRawParameterValue("saturation")->load();

    // 2. Calcular el factor de empuje (drive). Si está en 0 no afecta (drive = 1).
    float drive = 1.0f + (saturationValue * 2.0f);

    // Compensación de ganancia de salida automática basada en la intensidad de saturación
    float gainCompensation = 1.0f / std::sqrt(drive);

    // 3. Bucle de procesamiento de muestras de audio
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            float inputSample = channelData[sample];

            // Escalamos la señal de entrada con el drive
            float drivenSample = inputSample * drive;

            // Aplicamos Soft Clipping matemático mediante tangente hiperbólica
            float saturatedSample = std::tanh(drivenSample);

            // Guardamos la muestra procesada con su volumen compensado
            channelData[sample] = saturatedSample * gainCompensation;
        }
    }
}

//==============================================================================
bool SILKAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* SILKAudioProcessor::createEditor()
{
    return new SILKAudioProcessorEditor(*this);
}

//==============================================================================
void SILKAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // Opcional: Aquí guardarías el estado del ValueTree a un XML para recordar la perilla al guardar el proyecto en el DAW
}

void SILKAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // Opcional: Aquí restaurarías el estado desde el DAW
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SILKAudioProcessor();
}