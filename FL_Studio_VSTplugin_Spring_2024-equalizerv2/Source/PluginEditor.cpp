/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

void LookAndFeel::drawRotarySlider(juce::Graphics& g,
                                   int x,
                                   int y, 
                                   int width,
                                   int height,
                                   float sliderPosProportional,
                                   float rotaryStartAngle,
                                   float rotaryEndAngle,
                                   juce::Slider& slider)
{
    using namespace juce;

    auto bounds = Rectangle<float>(x, y, width, height);

    //  Colour(97u, 18u, 167u)
    g.setColour(Colours::yellow);
    g.fillEllipse(bounds);

    g.setColour(Colours::red);
    g.drawEllipse(bounds, 1.f);

    if ( auto* rswl = dynamic_cast<RotarySliderWithLabels*>(&slider) ) {
        auto center = bounds.getCentre();

        Path p;

        Rectangle<float> r;
        r.setLeft(center.getX() - 2);
        r.setRight(center.getX() + 2);
        r.setTop(bounds.getY());
        r.setBottom(center.getY() - rswl->getTextHeight() * 1.5);

        p.addRoundedRectangle(r, 2.f);

        jassert(rotaryStartAngle < rotaryEndAngle);

        auto sliderAngRad = jmap(sliderPosProportional, 0.f, 1.f, rotaryStartAngle, rotaryEndAngle);

        p.applyTransform(AffineTransform().rotated(sliderAngRad, center.getX(), center.getY()));

        g.fillPath(p);

        g.setFont(rswl->getTextBoxHeight());
        auto text = rswl->getDisplayString();
        auto strWidth = g.getCurrentFont().getStringWidth(text) - 7;

        r.setSize(strWidth + 4, rswl->getTextHeight() + 2);
        r.setCentre(bounds.getCentre());

        g.setColour(Colours::black);
        g.fillRect(r);

        g.setColour(Colours::white);
        g.drawFittedText(text, r.toNearestInt(), juce::Justification::verticallyCentred, 1);
    }

}

void RotarySliderWithLabels::paint(juce::Graphics& g) {
    using namespace juce;

    auto startAng = degreesToRadians(180.f + 45.f);
    auto endAng = degreesToRadians(180.f - 45.f) + MathConstants<float>::twoPi;

    auto range = getRange();

    auto sliderBounds = getSliderBounds();

    /*
    g.setColour(Colours::black);
    g.drawRect(getLocalBounds());
    g.setColour(Colours::yellow);
    g.drawRect(sliderBounds);
    */

    getLookAndFeel().drawRotarySlider(g, 
                                      sliderBounds.getX(), 
                                      sliderBounds.getY(), 
                                      sliderBounds.getWidth(),
                                      sliderBounds.getHeight(), 
                                      jmap(getValue(), range.getStart(), range.getEnd(), 0.0, 1.0), 
                                      startAng, 
                                      endAng,
                                      *this);

    auto center = sliderBounds.toFloat().getCentre();
    auto radius = sliderBounds.getWidth() * 0.5f;

    g.setColour(Colours::white);
    g.setFont(getTextHeight());

    auto numChoices = labels.size();
    for (int i = 0; i < numChoices; ++i) {
        auto pos = labels[i].pos;
        jassert(0.f <= pos);
        jassert(pos <= 1.f);

        auto ang = jmap(pos, 0.f, 1.f, startAng, endAng);

        auto c = center.getPointOnCircumference(radius + getTextHeight() * 0.5f + 1, ang);

        Rectangle<float> r;

        auto str = labels[i].label;
        r.setSize(g.getCurrentFont().getStringWidthFloat(str), getTextHeight());
        r.setCentre(c);
        r.setY(r.getY() + getTextHeight());

        g.drawFittedText(str, r.toNearestInt(), juce::Justification::centred, 1);
    }
}

juce::Rectangle<int> RotarySliderWithLabels::getSliderBounds() const {
    //return getLocalBounds();
    auto bounds = getLocalBounds();

    auto size = juce::jmin(bounds.getWidth(), bounds.getHeight());

    size -= getTextHeight() * 2;
    juce::Rectangle<int> r;
    r.setSize(size, size);
    r.setCentre(bounds.getCentreX(), 0);
    r.setY(2);

    return r;
}

juce::String RotarySliderWithLabels::getDisplayString() const {
    if ( auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(param) )
        return choiceParam->getCurrentChoiceName();

    juce::String str;
    bool addk = false;

    if ( auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(param) ) {
        float val = getValue();

        if ( val > 999.f ) {
            val /= 1000.f;
            addk = true;
        }

        str = juce::String(val, (addk ? 2 : 0));
    }
    else {
        jassertfalse;
    }

    if (suffix.isNotEmpty()) {
        str << " ";
        if (addk)
            str << "k";

        str << suffix;
    }

    return str;
}

ResponseCurveComponent::ResponseCurveComponent(GarethsEQAudioProcessor& p) : audioProcessor(p) {
    const auto& params = audioProcessor.getParameters();
    for (auto param : params) {
        param->addListener(this);
    }

    updateChain();

    startTimerHz(60);
}

ResponseCurveComponent::~ResponseCurveComponent() {
    const auto& params = audioProcessor.getParameters();
    for (auto param : params) {
        param->removeListener(this);
    }
}

void ResponseCurveComponent::parameterValueChanged(int parameterIndex, float newValue) {
    parametersChanged.set(true);
}

void ResponseCurveComponent::timerCallback() {
    if (parametersChanged.compareAndSetBool(false, true)) {
        updateChain();

        repaint();
    }
}

void ResponseCurveComponent::updateChain() {
    auto chainSettings = getChainSettings(audioProcessor.apvts);

    auto peakCoefficients1 = makePeakFilter1(chainSettings, audioProcessor.getSampleRate());
    updateCoefficients(monoChain.get<ChainPositions::Peak1>().coefficients, peakCoefficients1);

    auto peakCoefficients2 = makePeakFilter2(chainSettings, audioProcessor.getSampleRate());
    updateCoefficients(monoChain.get<ChainPositions::Peak2>().coefficients, peakCoefficients2);

    auto peakCoefficients3 = makePeakFilter3(chainSettings, audioProcessor.getSampleRate());
    updateCoefficients(monoChain.get<ChainPositions::Peak3>().coefficients, peakCoefficients3);

    auto lowCutCoefficients = makeLowCutFilter(chainSettings, audioProcessor.getSampleRate());
    auto highCutCoefficients = makeHighCutFilter(chainSettings, audioProcessor.getSampleRate());

    updateCutFilter(monoChain.get<ChainPositions::LowCut>(), lowCutCoefficients, chainSettings.lowCutSlope);
    updateCutFilter(monoChain.get<ChainPositions::HighCut>(), highCutCoefficients, chainSettings.highCutSlope);
}

void ResponseCurveComponent::paint(juce::Graphics& g)
{
    using namespace juce;
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(Colours::black);
    
    g.drawImage(background, getLocalBounds().toFloat());

    //auto responseArea = getLocalBounds();
    auto responseArea = getAnalysisArea();//getRenderArea();
   

    auto w = responseArea.getWidth();

    auto& lowcut = monoChain.get<ChainPositions::LowCut>();
    auto& peak1 = monoChain.get<ChainPositions::Peak1>();
    auto& peak2 = monoChain.get<ChainPositions::Peak2>();
    auto& peak3 = monoChain.get<ChainPositions::Peak3>();
    auto& highcut = monoChain.get<ChainPositions::HighCut>();

    auto sampleRate = audioProcessor.getSampleRate();

    std::vector<double> mags;

    mags.resize(w);

    for (int i = 0; i < w; ++i) {
        double mag = 1.f;
        auto freq = mapToLog10(double(i) / double(w), 20.0, 20000.0);

        if (!monoChain.isBypassed<ChainPositions::Peak1>())
            mag *= peak1.coefficients->getMagnitudeForFrequency(freq, sampleRate);

        if (!monoChain.isBypassed<ChainPositions::Peak2>())
            mag *= peak2.coefficients->getMagnitudeForFrequency(freq, sampleRate);

        if (!monoChain.isBypassed<ChainPositions::Peak3>())
            mag *= peak3.coefficients->getMagnitudeForFrequency(freq, sampleRate);


        if (!lowcut.isBypassed<0>())
            mag *= lowcut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!lowcut.isBypassed<1>())
            mag *= lowcut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!lowcut.isBypassed<2>())
            mag *= lowcut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!lowcut.isBypassed<3>())
            mag *= lowcut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);

        if (!highcut.isBypassed<0>())
            mag *= highcut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!highcut.isBypassed<1>())
            mag *= highcut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!highcut.isBypassed<2>())
            mag *= highcut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!highcut.isBypassed<3>())
            mag *= highcut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);

        mags[i] = Decibels::gainToDecibels(mag);
    }

    Path responseCurve;

    const double outputMin = responseArea.getBottom();
    const double outputMax = responseArea.getY();
    auto map = [outputMin, outputMax](double input) {
        return jmap(input, -24.0, 24.0, outputMin, outputMax);
        };

    responseCurve.startNewSubPath(responseArea.getX(), map(mags.front()));

    for (size_t i = 1; i < mags.size(); ++i) {
        responseCurve.lineTo(responseArea.getX() + i, map(mags[i]));
    }

    g.setColour(Colours::orange);
    g.drawRoundedRectangle(getRenderArea().toFloat(), 4.f, 1.f);

    g.setColour(Colours::white);
    g.strokePath(responseCurve, PathStrokeType(2.f));

}

void ResponseCurveComponent::resized() {
    using namespace juce;
    background = Image(Image::PixelFormat::RGB, getWidth(), getHeight(), true);

    Graphics g(background);

    Array<float> freqs{
        20, /*30, 40,*/ 50, 100,
        200, /*300, 400,*/ 500, 1000,
        2000, /*3000, 4000,*/ 5000, 10000,
        20000
    };
    
    auto renderArea = getAnalysisArea();
    auto left = renderArea.getX();
    auto right = renderArea.getRight();
    auto top = renderArea.getY();
    auto bottom = renderArea.getBottom();
    auto width = renderArea.getWidth();

    Array<float> xs;
    for (auto f : freqs) {
        auto normX = mapFromLog10(f, 20.f, 20000.f);
        xs.add(left + width * normX);
    }

    g.setColour(Colours::dimgrey);
    for (auto x : xs) {
        //auto normX = mapFromLog10(f, 20.f, 2000.f);

        //g.drawVerticalLine(getWidth() * normX, 0.f, getHeight());
        g.drawVerticalLine(x, top, bottom);
    }

    Array<float> gain{
        -24, -12, 0, 12, 24
    };

    for (auto gDb : gain) {
        auto y = jmap(gDb, -24.f, 24.f, float(bottom), float(top));
        //g.drawHorizontalLine(y, 0, getWidth());
        g.setColour(gDb == 0.f ? Colours::green : Colours::darkgrey);
        g.drawHorizontalLine(y, left, right);
    }

    //g.drawRect(getAnalysisArea());

    g.setColour(Colours::lightgrey);
    const int fontHeight = 10;
    g.setFont(fontHeight);

    for (int i = 0; i < freqs.size(); ++i) {
        auto f = freqs[i];
        auto x = xs[i];

        bool addK = false;
        String str;
        if (f > 999.f) {
            addK = true;
            f /= 1000.f;
        }

        str << f;
        if (addK)
            str << "k";
        str << "Hz";

        auto textWidth = g.getCurrentFont().getStringWidth(str);

        Rectangle<int> r;
        r.setSize(textWidth, fontHeight);
        r.setCentre(x, 0);
        r.setY(1);

        g.drawFittedText(str, r, juce::Justification::centred, 1);
    }

    for (auto gDb : gain) {
        auto y = jmap(gDb, -24.f, 24.f, float(bottom), float(top));

        String str;
        if (gDb > 0) {
            str << "+";
        }
        str << gDb;

        auto textWidth = g.getCurrentFont().getStringWidth(str);

        Rectangle<int> r;
        r.setSize(textWidth, fontHeight);
        r.setX(getWidth() - textWidth);
        r.setCentre(r.getCentreX(), y);

        g.setColour(gDb == 0.f ? Colours::green : Colours::lightgrey);

        g.drawFittedText(str, r, juce::Justification::centred, 1);

        str.clear();
        str << (gDb - 24.f);

        r.setX(1);
        textWidth = g.getCurrentFont().getStringWidth(str);
        r.setSize(textWidth, fontHeight);
        g.setColour(Colours::lightgrey);
        g.drawFittedText(str, r, juce::Justification::centred, 1);
    }

}

juce::Rectangle<int> ResponseCurveComponent::getRenderArea() {
    auto bounds = getLocalBounds();

    //bounds.reduce(10, 
    //    14);

    bounds.removeFromTop(12);
    bounds.removeFromBottom(2);
    bounds.removeFromLeft(20);
    bounds.removeFromRight(20);

    return bounds;
}

juce::Rectangle<int> ResponseCurveComponent::getAnalysisArea() {
    auto bounds = getRenderArea();
    bounds.removeFromTop(4);
    bounds.removeFromBottom(4);
    return bounds;
}

//==============================================================================
GarethsEQAudioProcessorEditor::GarethsEQAudioProcessorEditor (GarethsEQAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),

    peakFreqSlider1(*audioProcessor.apvts.getParameter("Peak Freq 1"), "Hz"),
    peakGainSlider1(*audioProcessor.apvts.getParameter("Peak Gain 1"), "dB"),
    peakQualitySlider1(*audioProcessor.apvts.getParameter("Peak Quality 1"), ""),
    peakFreqSlider2(*audioProcessor.apvts.getParameter("Peak Freq 2"), "Hz"),
    peakGainSlider2(*audioProcessor.apvts.getParameter("Peak Gain 2"), "dB"),
    peakQualitySlider2(*audioProcessor.apvts.getParameter("Peak Quality 2"), ""),
    peakFreqSlider3(*audioProcessor.apvts.getParameter("Peak Freq 3"), "Hz"),
    peakGainSlider3(*audioProcessor.apvts.getParameter("Peak Gain 3"), "dB"),
    peakQualitySlider3(*audioProcessor.apvts.getParameter("Peak Quality 3"), ""),
    lowCutFreqSlider(*audioProcessor.apvts.getParameter("LowCut Freq"), "Hz"),
    highCutFreqSlider(*audioProcessor.apvts.getParameter("HighCut Freq"), "Hz"),
    lowCutSlopeSlider(*audioProcessor.apvts.getParameter("LowCut Slope"), "dB/Oct"),
    highCutSlopeSlider(*audioProcessor.apvts.getParameter("HighCut Slope"), "dB/Oct"),


    responseCurveComponent(audioProcessor),
    peakFreqSlider1Attatchment(audioProcessor.apvts, "Peak Freq 1", peakFreqSlider1),
    peakGainSlider1Attatchment(audioProcessor.apvts, "Peak Gain 1", peakGainSlider1),
    peakQualitySlider1Attatchment(audioProcessor.apvts, "Peak Quality 1", peakQualitySlider1),
    peakFreqSlider2Attatchment(audioProcessor.apvts, "Peak Freq 2", peakFreqSlider2),
    peakGainSlider2Attatchment(audioProcessor.apvts, "Peak Gain 2", peakGainSlider2),
    peakQualitySlider2Attatchment(audioProcessor.apvts, "Peak Quality 2", peakQualitySlider2),
    peakFreqSlider3Attatchment(audioProcessor.apvts, "Peak Freq 3", peakFreqSlider3),
    peakGainSlider3Attatchment(audioProcessor.apvts, "Peak Gain 3", peakGainSlider3),
    peakQualitySlider3Attatchment(audioProcessor.apvts, "Peak Quality 3", peakQualitySlider3),
    lowCutFreqSliderAttatchment(audioProcessor.apvts, "LowCut Freq", lowCutFreqSlider),
    highCutFreqSliderAttatchment(audioProcessor.apvts, "HighCut Freq", highCutFreqSlider),
    lowCutSlopeSliderAttatchment(audioProcessor.apvts, "LowCut Slope", lowCutSlopeSlider),
    highCutSlopeSliderAttatchment(audioProcessor.apvts, "HighCut Slope", highCutSlopeSlider)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    peakFreqSlider1.labels.add({ 0.f, "20Hz" });
    peakFreqSlider1.labels.add({ 1.f, "20kHz" });
    peakGainSlider1.labels.add({ 0.f, "-24dB" });
    peakGainSlider1.labels.add({ 1.f, "-24dB" });
    peakQualitySlider1.labels.add({ 0.f, "0.1" });
    peakQualitySlider1.labels.add({ 1.f, "10.0" });

    peakFreqSlider2.labels.add({ 0.f, "20Hz" });
    peakFreqSlider2.labels.add({ 1.f, "20kHz" });
    peakGainSlider2.labels.add({ 0.f, "-24dB" });
    peakGainSlider2.labels.add({ 1.f, "-24dB" });
    peakQualitySlider2.labels.add({ 0.f, "0.1" });
    peakQualitySlider2.labels.add({ 1.f, "10.0" });

    peakFreqSlider3.labels.add({ 0.f, "20Hz" });
    peakFreqSlider3.labels.add({ 1.f, "20kHz" });
    peakGainSlider3.labels.add({ 0.f, "-24dB" });
    peakGainSlider3.labels.add({ 1.f, "-24dB" });
    peakQualitySlider3.labels.add({ 0.f, "0.1" });
    peakQualitySlider3.labels.add({ 1.f, "10.0" });

    lowCutFreqSlider.labels.add({ 0.f, "20Hz" });
    lowCutFreqSlider.labels.add({ 1.f, "20kHz" });

    highCutFreqSlider.labels.add({ 0.f, "20Hz" });
    highCutFreqSlider.labels.add({ 1.f, "20kHz" });

    lowCutSlopeSlider.labels.add({ 0.f, "12" });
    lowCutSlopeSlider.labels.add({ 1.f, "48" });

    highCutSlopeSlider.labels.add({ 0.f, "12" });
    highCutSlopeSlider.labels.add({ 1.f, "48" });


    for (auto* comp : getComps()) {
        addAndMakeVisible(comp);
    }
    

    setSize (680, 480);
}

GarethsEQAudioProcessorEditor::~GarethsEQAudioProcessorEditor()
{
   
}

//==============================================================================
void GarethsEQAudioProcessorEditor::paint(juce::Graphics& g)
{
    using namespace juce;
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(Colours::black);
    /*
    Path curve;

    auto bounds = getLocalBounds();
    auto center = bounds.getCentre();

    g.setFont(Font("Iosevka Term Slab", 30, 0));

    String title{ "Gareth's EQ" };
    g.setFont(30);
    auto titleWidth = g.getCurrentFont().getStringWidth(title);

    curve.startNewSubPath(center.x, 32);
    curve.lineTo(center.x - titleWidth * 0.45f, 32);

    auto cornerSize = 20;
    auto curvePos = curve.getCurrentPosition();
    curve.quadraticTo(curvePos.getX() - cornerSize, curvePos.getY(),
                      curvePos.getX() - cornerSize, curvePos.getY() - 16);
    curvePos = curve.getCurrentPosition();
    curve.quadraticTo(curvePos.getX(), 2,
                      curvePos.getX() - cornerSize, 2);

    curve.lineTo({ 0.f, 2.f });
    curve.lineTo(0.f, 0.f);
    curve.lineTo(center.x, 0.f);
    curve.closeSubPath();

    g.setColour(Colour(97u, 18u, 167u));
    g.fillPath(curve);

    curve.applyTransform(AffineTransform().scaled(-1, 1));
    curve.applyTransform(AffineTransform().translated(getWidth(), 0));
    g.fillPath(curve);


    g.setColour(Colour(255u, 154u, 1u));
    g.drawFittedText(title, bounds, juce::Justification::centredTop, 1);

  
    g.setColour(Colours::grey);
    g.setFont(14);
    g.drawFittedText("LowCut", lowCutSlopeSlider.getBounds(), juce::Justification::centredBottom, 1);
    g.drawFittedText("Peak 1", peakQualitySlider1.getBounds(), juce::Justification::centredBottom, 1);
    g.drawFittedText("Peak 2", peakQualitySlider2.getBounds(), juce::Justification::centredBottom, 1);
    g.drawFittedText("Peak 2", peakQualitySlider3.getBounds(), juce::Justification::centredBottom, 1);
    g.drawFittedText("HighCut", highCutSlopeSlider.getBounds(), juce::Justification::centredBottom, 1);

    auto buildDate = Time::getCompilationDate().toString(true, false);
    auto buildTime = Time::getCompilationDate().toString(false, true);
    g.drawFittedText("Build: " + buildDate + "\n" + buildTime, highCutSlopeSlider.getBounds().withY(6), Justification::topRight, 2);
    */
}

void GarethsEQAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    auto bounds = getLocalBounds();
    float hRatio = 36.f / 100.f;
    auto responseArea = bounds.removeFromTop(bounds.getHeight() * hRatio);

    responseCurveComponent.setBounds(responseArea);

    bounds.removeFromTop(8);

    auto lowCutArea = bounds.removeFromLeft(bounds.getWidth() * 0.25);
    auto highCutArea = bounds.removeFromRight(bounds.getRight() * 0.25);
    //auto highCutArea = bounds.removeFromRight(bounds.getRight() * 0.25);

    lowCutSlopeSlider.setBounds(lowCutArea.removeFromTop(lowCutArea.getHeight() * 0.5));
    lowCutFreqSlider.setBounds(lowCutArea);
    //lowCutFreqSlider.setBounds(lowCutArea.removeFromTop(lowCutArea.getHeight() * 0.5));
    //lowCutSlopeSlider.setBounds(lowCutArea);

    highCutSlopeSlider.setBounds(highCutArea.removeFromTop(highCutArea.getHeight() * 0.5));
    highCutFreqSlider.setBounds(highCutArea);
    //highCutFreqSlider.setBounds(highCutArea.removeFromTop(highCutArea.getHeight() * 0.5));
    //highCutSlopeSlider.setBounds(highCutArea);

    
    //  NODE #1
    peakFreqSlider1.setCentrePosition(165, 175);
    peakFreqSlider1.setSize(85, 85);

    peakGainSlider1.setCentrePosition(300, 175);
    peakGainSlider1.setSize(85, 85);

    peakQualitySlider1.setCentrePosition(435, 175);
    peakQualitySlider1.setSize(85, 85);

    //  NODE #2
    peakFreqSlider2.setCentrePosition(165, 275);
    peakFreqSlider2.setSize(85, 85);

    peakGainSlider2.setCentrePosition(300, 275);
    peakGainSlider2.setSize(85, 85);

    peakQualitySlider2.setCentrePosition(435, 275);
    peakQualitySlider2.setSize(85, 85);

    //  NODE #3
    peakFreqSlider3.setCentrePosition(165, 375);
    peakFreqSlider3.setSize(85, 85);

    peakGainSlider3.setCentrePosition(300, 375);
    peakGainSlider3.setSize(85, 85);

    peakQualitySlider3.setCentrePosition(435, 375);
    peakQualitySlider3.setSize(85, 85);
    
}


std::vector<juce::Component*> GarethsEQAudioProcessorEditor::getComps() {
    return {
        &peakFreqSlider1,
        &peakGainSlider1,
        &peakQualitySlider1,
        &peakFreqSlider2,
        &peakGainSlider2,
        &peakQualitySlider2,
        &peakFreqSlider3,
        &peakGainSlider3,
        &peakQualitySlider3,
        &lowCutFreqSlider,
        &highCutFreqSlider,
        &lowCutSlopeSlider,
        &highCutSlopeSlider,
        &responseCurveComponent
    };
}
