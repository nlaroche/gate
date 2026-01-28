#include "PluginEditor.h"
#include "ParameterIDs.h"

GateEditor::GateEditor(GateProcessor& p)
    : AudioProcessorEditor(&p), processor_(p)
{
    setSize(550, 450);
    setResizable(false, false);

    // Create relays
    patternRelay_ = std::make_unique<juce::WebSliderRelay>(ParameterIDs::pattern);
    stepsRelay_ = std::make_unique<juce::WebSliderRelay>(ParameterIDs::steps);
    rateRelay_ = std::make_unique<juce::WebSliderRelay>(ParameterIDs::rate);
    stepDataRelay_ = std::make_unique<juce::WebSliderRelay>(ParameterIDs::stepData);
    attackRelay_ = std::make_unique<juce::WebSliderRelay>(ParameterIDs::attack);
    holdRelay_ = std::make_unique<juce::WebSliderRelay>(ParameterIDs::hold);
    releaseRelay_ = std::make_unique<juce::WebSliderRelay>(ParameterIDs::release);
    curveRelay_ = std::make_unique<juce::WebSliderRelay>(ParameterIDs::curve);
    swingRelay_ = std::make_unique<juce::WebSliderRelay>(ParameterIDs::swing);
    humanizeRelay_ = std::make_unique<juce::WebSliderRelay>(ParameterIDs::humanize);
    velocityRelay_ = std::make_unique<juce::WebSliderRelay>(ParameterIDs::velocity);
    depthRelay_ = std::make_unique<juce::WebSliderRelay>(ParameterIDs::depth);
    mixRelay_ = std::make_unique<juce::WebSliderRelay>(ParameterIDs::mix);
    outputRelay_ = std::make_unique<juce::WebSliderRelay>(ParameterIDs::output);
    bypassRelay_ = std::make_unique<juce::WebToggleButtonRelay>(ParameterIDs::bypass);

    setupWebView();
    startTimerHz(30);
}

GateEditor::~GateEditor()
{
    stopTimer();
}

void GateEditor::setupWebView()
{
    auto options = juce::WebBrowserComponent::Options{}
        .withOptionsFrom(*patternRelay_)
        .withOptionsFrom(*stepsRelay_)
        .withOptionsFrom(*rateRelay_)
        .withOptionsFrom(*stepDataRelay_)
        .withOptionsFrom(*attackRelay_)
        .withOptionsFrom(*holdRelay_)
        .withOptionsFrom(*releaseRelay_)
        .withOptionsFrom(*curveRelay_)
        .withOptionsFrom(*swingRelay_)
        .withOptionsFrom(*humanizeRelay_)
        .withOptionsFrom(*velocityRelay_)
        .withOptionsFrom(*depthRelay_)
        .withOptionsFrom(*mixRelay_)
        .withOptionsFrom(*outputRelay_)
        .withOptionsFrom(*bypassRelay_)
        .withBackend(juce::WebBrowserComponent::Options::Backend::webview2)
        .withWinWebView2Options(
            juce::WebBrowserComponent::Options::WinWebView2{}
                .withBackgroundColour(juce::Colour(0xFF0a0a14))
        );

    webView_ = std::make_unique<juce::WebBrowserComponent>(options);
    addAndMakeVisible(*webView_);

    // Create attachments
    auto& apvts = processor_.getAPVTS();

    patternAttachment_ = std::make_unique<juce::WebSliderParameterAttachment>(
        *apvts.getParameter(ParameterIDs::pattern), *patternRelay_, nullptr);
    stepsAttachment_ = std::make_unique<juce::WebSliderParameterAttachment>(
        *apvts.getParameter(ParameterIDs::steps), *stepsRelay_, nullptr);
    rateAttachment_ = std::make_unique<juce::WebSliderParameterAttachment>(
        *apvts.getParameter(ParameterIDs::rate), *rateRelay_, nullptr);
    stepDataAttachment_ = std::make_unique<juce::WebSliderParameterAttachment>(
        *apvts.getParameter(ParameterIDs::stepData), *stepDataRelay_, nullptr);
    attackAttachment_ = std::make_unique<juce::WebSliderParameterAttachment>(
        *apvts.getParameter(ParameterIDs::attack), *attackRelay_, nullptr);
    holdAttachment_ = std::make_unique<juce::WebSliderParameterAttachment>(
        *apvts.getParameter(ParameterIDs::hold), *holdRelay_, nullptr);
    releaseAttachment_ = std::make_unique<juce::WebSliderParameterAttachment>(
        *apvts.getParameter(ParameterIDs::release), *releaseRelay_, nullptr);
    curveAttachment_ = std::make_unique<juce::WebSliderParameterAttachment>(
        *apvts.getParameter(ParameterIDs::curve), *curveRelay_, nullptr);
    swingAttachment_ = std::make_unique<juce::WebSliderParameterAttachment>(
        *apvts.getParameter(ParameterIDs::swing), *swingRelay_, nullptr);
    humanizeAttachment_ = std::make_unique<juce::WebSliderParameterAttachment>(
        *apvts.getParameter(ParameterIDs::humanize), *humanizeRelay_, nullptr);
    velocityAttachment_ = std::make_unique<juce::WebSliderParameterAttachment>(
        *apvts.getParameter(ParameterIDs::velocity), *velocityRelay_, nullptr);
    depthAttachment_ = std::make_unique<juce::WebSliderParameterAttachment>(
        *apvts.getParameter(ParameterIDs::depth), *depthRelay_, nullptr);
    mixAttachment_ = std::make_unique<juce::WebSliderParameterAttachment>(
        *apvts.getParameter(ParameterIDs::mix), *mixRelay_, nullptr);
    outputAttachment_ = std::make_unique<juce::WebSliderParameterAttachment>(
        *apvts.getParameter(ParameterIDs::output), *outputRelay_, nullptr);
    bypassAttachment_ = std::make_unique<juce::WebToggleButtonParameterAttachment>(
        *apvts.getParameter(ParameterIDs::bypass), *bypassRelay_, nullptr);

#if GATE_DEV_MODE
    webView_->goToURL("http://localhost:5173");
#else
    auto webUIPath = juce::File::getSpecialLocation(juce::File::currentExecutableFile)
                         .getParentDirectory().getChildFile("WebUI").getChildFile("index.html");
    webView_->goToURL(juce::URL(webUIPath));
#endif
}

void GateEditor::timerCallback()
{
    juce::DynamicObject::Ptr data = new juce::DynamicObject();
    data->setProperty("currentStep", processor_.currentStep.load());
    data->setProperty("gateLevel", processor_.gateLevel.load());
    data->setProperty("outputLevel", processor_.outputLevel.load());
    data->setProperty("stepPattern", processor_.stepPattern.load());

    webView_->emitEventIfBrowserIsVisible("visualizerData", juce::var(data.get()));
}

void GateEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xFF0a0a14));
}

void GateEditor::resized()
{
    if (webView_)
        webView_->setBounds(getLocalBounds());
}
