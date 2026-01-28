#pragma once

#include "PluginProcessor.h"
#include <juce_gui_extra/juce_gui_extra.h>

class GateEditor : public juce::AudioProcessorEditor,
                   private juce::Timer
{
public:
    explicit GateEditor(GateProcessor&);
    ~GateEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void setupWebView();

    GateProcessor& processor_;

    // Relays
    std::unique_ptr<juce::WebSliderRelay> patternRelay_;
    std::unique_ptr<juce::WebSliderRelay> stepsRelay_;
    std::unique_ptr<juce::WebSliderRelay> rateRelay_;
    std::unique_ptr<juce::WebSliderRelay> stepDataRelay_;
    std::unique_ptr<juce::WebSliderRelay> attackRelay_;
    std::unique_ptr<juce::WebSliderRelay> holdRelay_;
    std::unique_ptr<juce::WebSliderRelay> releaseRelay_;
    std::unique_ptr<juce::WebSliderRelay> curveRelay_;
    std::unique_ptr<juce::WebSliderRelay> swingRelay_;
    std::unique_ptr<juce::WebSliderRelay> humanizeRelay_;
    std::unique_ptr<juce::WebSliderRelay> velocityRelay_;
    std::unique_ptr<juce::WebSliderRelay> depthRelay_;
    std::unique_ptr<juce::WebSliderRelay> mixRelay_;
    std::unique_ptr<juce::WebSliderRelay> outputRelay_;
    std::unique_ptr<juce::WebToggleButtonRelay> bypassRelay_;

    std::unique_ptr<juce::WebBrowserComponent> webView_;

    // Attachments
    std::unique_ptr<juce::WebSliderParameterAttachment> patternAttachment_;
    std::unique_ptr<juce::WebSliderParameterAttachment> stepsAttachment_;
    std::unique_ptr<juce::WebSliderParameterAttachment> rateAttachment_;
    std::unique_ptr<juce::WebSliderParameterAttachment> stepDataAttachment_;
    std::unique_ptr<juce::WebSliderParameterAttachment> attackAttachment_;
    std::unique_ptr<juce::WebSliderParameterAttachment> holdAttachment_;
    std::unique_ptr<juce::WebSliderParameterAttachment> releaseAttachment_;
    std::unique_ptr<juce::WebSliderParameterAttachment> curveAttachment_;
    std::unique_ptr<juce::WebSliderParameterAttachment> swingAttachment_;
    std::unique_ptr<juce::WebSliderParameterAttachment> humanizeAttachment_;
    std::unique_ptr<juce::WebSliderParameterAttachment> velocityAttachment_;
    std::unique_ptr<juce::WebSliderParameterAttachment> depthAttachment_;
    std::unique_ptr<juce::WebSliderParameterAttachment> mixAttachment_;
    std::unique_ptr<juce::WebSliderParameterAttachment> outputAttachment_;
    std::unique_ptr<juce::WebToggleButtonParameterAttachment> bypassAttachment_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GateEditor)
};
