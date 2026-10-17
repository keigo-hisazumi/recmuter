#include "PluginEditor.h"

RecMuterAudioProcessorEditor::RecMuterAudioProcessorEditor (RecMuterAudioProcessor& p)
    : AudioProcessorEditor (&p),
      processor (p)
{
    // 選択肢はパラメータ定義から取得する
    if (auto* choice = dynamic_cast<juce::AudioParameterChoice*> (
            p.parameters.getParameter (RecMuterAudioProcessor::modeParamId)))
        modeBox.addItemList (choice->choices, 1);

    modeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        p.parameters, RecMuterAudioProcessor::modeParamId, modeBox);
    fadeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        p.parameters, RecMuterAudioProcessor::fadeParamId, fadeSlider);

    for (auto* label : { &modeLabel, &fadeLabel })
    {
        label->setJustificationType (juce::Justification::centredLeft);
        addAndMakeVisible (*label);
    }

    fadeSlider.setTextValueSuffix (" ms");
    addAndMakeVisible (modeBox);
    addAndMakeVisible (fadeSlider);

    setSize (360, 200);
    startTimerHz (30);
}

void RecMuterAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff1e1f24));

    // ミュート状態の表示
    const auto statusColour = lastMuted ? juce::Colour (0xffd9534f) : juce::Colour (0xff3fa36b);
    g.setColour (statusColour);
    g.fillRoundedRectangle (statusArea.toFloat(), 6.0f);

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (22.0f, juce::Font::bold));
    g.drawText (lastMuted ? "MUTED" : "PASS", statusArea, juce::Justification::centred);

    // トランスポート状態の表示
    juce::String transport = lastRecording ? "Recording" : (lastPlaying ? "Playing" : "Stopped");
    g.setFont (juce::FontOptions (13.0f));
    g.setColour (juce::Colours::lightgrey);
    g.drawText ("Transport: " + transport,
                statusArea.withY (statusArea.getBottom() + 4).withHeight (18),
                juce::Justification::centred);
}

void RecMuterAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (16);

    statusArea = area.removeFromTop (56);
    area.removeFromTop (30);

    auto modeRow = area.removeFromTop (28);
    modeLabel.setBounds (modeRow.removeFromLeft (90));
    modeBox.setBounds (modeRow);

    area.removeFromTop (12);

    auto fadeRow = area.removeFromTop (28);
    fadeLabel.setBounds (fadeRow.removeFromLeft (90));
    fadeSlider.setBounds (fadeRow);
}

void RecMuterAudioProcessorEditor::timerCallback()
{
    const auto mutedNow = processor.isCurrentlyMuted();
    const auto playingNow = processor.isHostPlaying();
    const auto recordingNow = processor.isHostRecording();

    if (mutedNow != lastMuted || playingNow != lastPlaying || recordingNow != lastRecording)
    {
        lastMuted = mutedNow;
        lastPlaying = playingNow;
        lastRecording = recordingNow;
        repaint();
    }
}
