#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ParameterIDs.h"

GateProcessor::GateProcessor()
    : AudioProcessor(BusesProperties()
                     .withInput("Input", juce::AudioChannelSet::stereo(), true)
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts_(*this, nullptr, "Parameters", createParameterLayout()),
      rng_(std::random_device{}()),
      dist_(-1.0f, 1.0f)
{
}

GateProcessor::~GateProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout GateProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Pattern Parameters
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID(ParameterIDs::pattern, 1), "Pattern",
        juce::StringArray{ "All", "Alternate", "Quarter", "Half", "Trance", "Sidechain", "Syncopated", "Stutter" },
        4));  // Default to Trance

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParameterIDs::steps, 1), "Steps",
        juce::NormalisableRange<float>(4.0f, 16.0f, 1.0f), 16.0f));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID(ParameterIDs::rate, 1), "Rate",
        juce::StringArray{ "1/1", "1/2", "1/4", "1/8", "1/16", "1/32" },
        3));  // Default to 1/8

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParameterIDs::stepData, 1), "Step Data",
        juce::NormalisableRange<float>(0.0f, 65535.0f, 1.0f), 65535.0f));

    // Envelope Parameters
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParameterIDs::attack, 1), "Attack",
        juce::NormalisableRange<float>(0.1f, 100.0f, 0.1f, 0.5f), 5.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParameterIDs::hold, 1), "Hold",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 50.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParameterIDs::release, 1), "Release",
        juce::NormalisableRange<float>(0.1f, 500.0f, 0.1f, 0.5f), 50.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParameterIDs::curve, 1), "Curve",
        juce::NormalisableRange<float>(-100.0f, 100.0f, 0.1f), 0.0f));

    // Modulation Parameters
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParameterIDs::swing, 1), "Swing",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 0.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParameterIDs::humanize, 1), "Humanize",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 0.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParameterIDs::velocity, 1), "Velocity",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 0.0f));

    // Mix Parameters
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParameterIDs::depth, 1), "Depth",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 100.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParameterIDs::mix, 1), "Mix",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 100.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParameterIDs::output, 1), "Output",
        juce::NormalisableRange<float>(-24.0f, 12.0f, 0.1f), 0.0f));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID(ParameterIDs::bypass, 1), "Bypass", false));

    return { params.begin(), params.end() };
}

void GateProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    sampleRate_ = sampleRate;
    samplesPerBeat_ = sampleRate * 60.0 / 120.0;  // Default 120 BPM

    smoothDepth_.reset(sampleRate, 0.02);
    smoothMix_.reset(sampleRate, 0.02);
    smoothOutput_.reset(sampleRate, 0.02);

    stepPosition_ = 0.0;
    lastStep_ = -1;
    envelopeValue_ = 0.0f;
    envelopeStage_ = EnvelopeStage::Off;
}

void GateProcessor::releaseResources() {}

bool GateProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    return true;
}

bool GateProcessor::isStepOn(int step, int pattern, int stepData) const
{
    // Use preset pattern or custom step data
    uint16_t patternBits = (pattern >= 0 && pattern < 8) ? kPresetPatterns[pattern] : static_cast<uint16_t>(stepData);
    return (patternBits >> (15 - step)) & 1;
}

float GateProcessor::calculateEnvelope(float attackMs, float releaseMs, float curve, float holdPct,
                                        int stepLengthSamples)
{
    const float attackSamples = (attackMs / 1000.0f) * static_cast<float>(sampleRate_);
    const float releaseSamples = (releaseMs / 1000.0f) * static_cast<float>(sampleRate_);
    const float holdSamples = holdPct / 100.0f * static_cast<float>(stepLengthSamples);

    // Apply curve to envelope
    auto applyCurve = [curve](float linear) {
        if (std::abs(curve) < 1.0f) return linear;
        if (curve > 0) {
            // Exponential
            return std::pow(linear, 1.0f + curve / 50.0f);
        } else {
            // Logarithmic
            return 1.0f - std::pow(1.0f - linear, 1.0f - curve / 50.0f);
        }
    };

    switch (envelopeStage_)
    {
        case EnvelopeStage::Attack:
            envelopeValue_ += 1.0f / attackSamples;
            if (envelopeValue_ >= 1.0f)
            {
                envelopeValue_ = 1.0f;
                envelopeStage_ = EnvelopeStage::Hold;
                holdSamplesRemaining_ = static_cast<int>(holdSamples);
            }
            return applyCurve(envelopeValue_);

        case EnvelopeStage::Hold:
            holdSamplesRemaining_--;
            if (holdSamplesRemaining_ <= 0)
            {
                envelopeStage_ = EnvelopeStage::Release;
            }
            return 1.0f;

        case EnvelopeStage::Release:
            envelopeValue_ -= 1.0f / releaseSamples;
            if (envelopeValue_ <= 0.0f)
            {
                envelopeValue_ = 0.0f;
                envelopeStage_ = EnvelopeStage::Off;
            }
            return applyCurve(envelopeValue_);

        case EnvelopeStage::Off:
        default:
            return 0.0f;
    }
}

void GateProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    auto* leftChannel = buffer.getWritePointer(0);
    auto* rightChannel = buffer.getWritePointer(1);

    // Get parameters
    const int patternIdx = static_cast<int>(apvts_.getRawParameterValue(ParameterIDs::pattern)->load());
    const int numSteps = static_cast<int>(apvts_.getRawParameterValue(ParameterIDs::steps)->load());
    const int rateIdx = static_cast<int>(apvts_.getRawParameterValue(ParameterIDs::rate)->load());
    const int customStepData = static_cast<int>(apvts_.getRawParameterValue(ParameterIDs::stepData)->load());
    const float attackMs = apvts_.getRawParameterValue(ParameterIDs::attack)->load();
    const float holdPct = apvts_.getRawParameterValue(ParameterIDs::hold)->load();
    const float releaseMs = apvts_.getRawParameterValue(ParameterIDs::release)->load();
    const float curve = apvts_.getRawParameterValue(ParameterIDs::curve)->load();
    const float swing = apvts_.getRawParameterValue(ParameterIDs::swing)->load() / 100.0f;
    const float humanize = apvts_.getRawParameterValue(ParameterIDs::humanize)->load() / 100.0f;
    const float velocityAmt = apvts_.getRawParameterValue(ParameterIDs::velocity)->load() / 100.0f;
    const float depthParam = apvts_.getRawParameterValue(ParameterIDs::depth)->load() / 100.0f;
    const float mixParam = apvts_.getRawParameterValue(ParameterIDs::mix)->load() / 100.0f;
    const float outputDb = apvts_.getRawParameterValue(ParameterIDs::output)->load();
    const bool bypassed = apvts_.getRawParameterValue(ParameterIDs::bypass)->load() > 0.5f;

    if (bypassed)
    {
        gateLevel.store(1.0f);
        return;
    }

    // Update smoothed values
    smoothDepth_.setTargetValue(depthParam);
    smoothMix_.setTargetValue(mixParam);
    smoothOutput_.setTargetValue(juce::Decibels::decibelsToGain(outputDb));

    // Get tempo from host
    if (auto* playHead = getPlayHead())
    {
        if (auto posInfo = playHead->getPosition())
        {
            if (posInfo->getBpm())
            {
                samplesPerBeat_ = sampleRate_ * 60.0 / *posInfo->getBpm();
            }
            if (posInfo->getPpqPosition())
            {
                // Sync to host position
                const double ppq = *posInfo->getPpqPosition();
                // Rate: 1/1=1, 1/2=2, 1/4=4, 1/8=8, 1/16=16, 1/32=32
                const double stepsPerBeat = std::pow(2.0, rateIdx);
                stepPosition_ = std::fmod(ppq * stepsPerBeat, static_cast<double>(numSteps));
            }
        }
    }

    // Calculate step length
    const double stepsPerBeat = std::pow(2.0, rateIdx);
    const double samplesPerStep = samplesPerBeat_ / stepsPerBeat;

    // Update visualizer
    stepPattern.store(patternIdx >= 0 ? kPresetPatterns[patternIdx] : customStepData);

    float peakLevel = 0.0f;
    float avgGateLevel = 0.0f;

    for (int i = 0; i < numSamples; ++i)
    {
        const float currentDepth = smoothDepth_.getNextValue();
        const float currentMix = smoothMix_.getNextValue();
        const float currentOutput = smoothOutput_.getNextValue();

        // Calculate current step with swing
        int currentStepInt = static_cast<int>(stepPosition_) % numSteps;

        // Apply swing to odd steps
        double swingOffset = 0.0;
        if (currentStepInt % 2 == 1)
        {
            swingOffset = swing * samplesPerStep * 0.5;
        }

        // Check for step change
        if (currentStepInt != lastStep_)
        {
            lastStep_ = currentStepInt;

            // Apply humanize (timing jitter)
            if (humanize > 0.0f)
            {
                swingOffset += dist_(rng_) * humanize * samplesPerStep * 0.1;
            }

            // Check if this step is on
            if (isStepOn(currentStepInt, patternIdx, customStepData))
            {
                envelopeStage_ = EnvelopeStage::Attack;
                envelopeValue_ = 0.0f;
            }
        }

        // Calculate gate envelope
        float gateEnvelope = calculateEnvelope(attackMs, releaseMs, curve, holdPct,
                                                static_cast<int>(samplesPerStep));

        // Apply velocity variation
        if (velocityAmt > 0.0f)
        {
            const float velocityMod = 1.0f - velocityAmt * 0.5f + dist_(rng_) * velocityAmt * 0.5f;
            gateEnvelope *= velocityMod;
        }

        // Apply depth (how much the gate affects signal)
        const float gateGain = 1.0f - (1.0f - gateEnvelope) * currentDepth;

        // Process audio
        const float dryL = leftChannel[i];
        const float dryR = rightChannel[i];

        const float wetL = dryL * gateGain;
        const float wetR = dryR * gateGain;

        // Mix dry/wet
        leftChannel[i] = (dryL * (1.0f - currentMix) + wetL * currentMix) * currentOutput;
        rightChannel[i] = (dryR * (1.0f - currentMix) + wetR * currentMix) * currentOutput;

        // Advance step position
        stepPosition_ += 1.0 / samplesPerStep;
        if (stepPosition_ >= numSteps)
            stepPosition_ -= numSteps;

        peakLevel = std::max(peakLevel, std::abs(leftChannel[i]));
        peakLevel = std::max(peakLevel, std::abs(rightChannel[i]));
        avgGateLevel += gateEnvelope;
    }

    // Update visualizer
    currentStep.store(static_cast<int>(stepPosition_) % numSteps);
    gateLevel.store(avgGateLevel / numSamples);
    outputLevel.store(peakLevel);
}

juce::AudioProcessorEditor* GateProcessor::createEditor()
{
    return new GateEditor(*this);
}

void GateProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts_.copyState();
    state.setProperty("stateVersion", kStateVersion, nullptr);
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void GateProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml && xml->hasTagName(apvts_.state.getType()))
    {
        auto state = juce::ValueTree::fromXml(*xml);
        apvts_.replaceState(state);
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GateProcessor();
}
