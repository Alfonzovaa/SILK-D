#pragma once

#include <JuceHeader.h>

//==============================================================================
class SILKAudioProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    SILKAudioProcessor();
    ~SILKAudioProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    //==============================================================================
    // Método público necesario para que el Editor se conecte a la perilla de saturación
    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }

private:
    //==============================================================================
    // Árbol que gestionará de forma segura el parámetro de saturación
    juce::AudioProcessorValueTreeState apvts;

    // Macro de JUCE corregida con el nombre exacto de tu clase actual
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SILKAudioProcessor)
};