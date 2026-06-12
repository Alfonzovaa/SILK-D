/*
  ==============================================================================
    This file contains the basic framework code for a JUCE plugin processor.
  ==============================================================================
*/

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
    // Método de acceso al APVTS para los SliderAttachments y ButtonAttachments del Editor
    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }

    // Función que lee el nivel del VU de manera atómica e hilo-segura desde el Editor
    float getVULevel() const { return vuLevel.load(); }

private:
    //==============================================================================
    juce::AudioProcessorValueTreeState apvts;

    // Variable atómica para almacenar el nivel suavizado del VÚmetro logarítmico (0.0f a 1.0f)
    std::atomic<float> vuLevel{ 0.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SILKAudioProcessor)
};