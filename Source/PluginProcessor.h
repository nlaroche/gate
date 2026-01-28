#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <array>
#include <random>

class GateProcessor : public juce::AudioProcessor
{
public:
    GateProcessor();
    ~GateProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.5; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts_; }

    // Visualizer data
    std::atomic<int> currentStep{ 0 };
    std::atomic<float> gateLevel{ 0.0f };
    std::atomic<float> outputLevel{ 0.0f };
    std::atomic<int> stepPattern{ 0xFFFF };  // 16 bits for 16 steps

private:
    juce::AudioProcessorValueTreeState apvts_;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    static constexpr int kStateVersion = 1;
    static constexpr int kMaxSteps = 16;

    // Preset patterns
    static constexpr std::array<uint16_t, 8> kPresetPatterns = {{
        0xFFFF,  // All on
        0xAAAA,  // Alternating
        0x8888,  // Quarter notes
        0xF0F0,  // Half notes
        0xEEEE,  // Trance gate
        0xFAFA,  // Sidechain style
        0xB6B6,  // Syncopated
        0xF8F8,  // Stutter
    }};

    // Gate state
    double sampleRate_ = 44100.0;
    double samplesPerBeat_ = 22050.0;
    double stepPosition_ = 0.0;
    int lastStep_ = -1;

    // Envelope state
    float envelopeValue_ = 0.0f;
    enum class EnvelopeStage { Attack, Hold, Release, Off };
    EnvelopeStage envelopeStage_ = EnvelopeStage::Off;
    int holdSamplesRemaining_ = 0;

    // Smoothed parameters
    juce::SmoothedValue<float> smoothDepth_;
    juce::SmoothedValue<float> smoothMix_;
    juce::SmoothedValue<float> smoothOutput_;

    // Random for humanize
    std::mt19937 rng_;
    std::uniform_real_distribution<float> dist_;

    // Get step state from pattern
    bool isStepOn(int step, int pattern, int stepData) const;
    float calculateEnvelope(float attackMs, float releaseMs, float curve, float holdPct,
                           int stepLengthSamples);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GateProcessor)
};
