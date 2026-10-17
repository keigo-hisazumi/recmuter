#include "PluginProcessor.h"
#include "PluginEditor.h"

RecMuterAudioProcessor::RecMuterAudioProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput ("Input", juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "RecMuterState", createParameterLayout())
{
    modeParam = parameters.getRawParameterValue (modeParamId);
    fadeParam = parameters.getRawParameterValue (fadeParamId);
}

juce::AudioProcessorValueTreeState::ParameterLayout RecMuterAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // 選択肢の並びは recmuter::MuteMode の定義順と一致させること
    layout.add (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { modeParamId, 1 },
        "Mute When",
        juce::StringArray { "Play or Record", "Record Only", "Play Only" },
        0));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { fadeParamId, 1 },
        "Fade",
        juce::NormalisableRange<float> (0.0f, 200.0f, 0.1f),
        10.0f,
        juce::AudioParameterFloatAttributes().withLabel ("ms")));

    return layout;
}

void RecMuterAudioProcessor::prepareToPlay (double sampleRate, int)
{
    currentSampleRate = sampleRate;
    ramp.setFadeTime (currentSampleRate, fadeParam->load());
    ramp.snapToTarget();
}

bool RecMuterAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& mainIn = layouts.getMainInputChannelSet();
    const auto& mainOut = layouts.getMainOutputChannelSet();

    // 入出力が同じチャンネル構成であれば、モノラル・ステレオ・サラウンドいずれも受け付ける
    return ! mainOut.isDisabled() && mainIn == mainOut;
}

void RecMuterAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    // 入力より出力チャンネルが多い場合、余った出力チャンネルを無音にする
    for (auto ch = getTotalNumInputChannels(); ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear (ch, 0, buffer.getNumSamples());

    // ホストのトランスポート状態を取得する（取得できない場合は停止中として扱う）
    bool playing = false;
    bool recording = false;

    if (auto* hostPlayHead = getPlayHead())
    {
        if (const auto position = hostPlayHead->getPosition())
        {
            playing = position->getIsPlaying();
            recording = position->getIsRecording();
        }
    }

    hostPlaying.store (playing, std::memory_order_relaxed);
    hostRecording.store (recording, std::memory_order_relaxed);

    const auto mode = static_cast<recmuter::MuteMode> (juce::roundToInt (modeParam->load()));
    const auto shouldMute = recmuter::shouldMute (mode, playing, recording);
    muted.store (shouldMute, std::memory_order_relaxed);

    ramp.setFadeTime (currentSampleRate, fadeParam->load());
    ramp.setTarget (shouldMute ? 0.0f : 1.0f);

    const auto numChannels = buffer.getNumChannels();
    const auto numSamples = buffer.getNumSamples();

    // ゲインが確定している場合はブロック単位で処理する
    if (ramp.isSettled())
    {
        if (ramp.getCurrent() <= 0.0f)
            buffer.clear();

        return;
    }

    // フェード中はサンプルごとにゲインを適用する
    auto* const* channels = buffer.getArrayOfWritePointers();

    for (int i = 0; i < numSamples; ++i)
    {
        const auto gain = ramp.next();

        for (int ch = 0; ch < numChannels; ++ch)
            channels[ch][i] *= gain;
    }
}

juce::AudioProcessorEditor* RecMuterAudioProcessor::createEditor()
{
    return new RecMuterAudioProcessorEditor (*this);
}

void RecMuterAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = parameters.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void RecMuterAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xml));
}

// JUCE がプラグインのインスタンスを生成するためのエントリーポイント
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RecMuterAudioProcessor();
}
