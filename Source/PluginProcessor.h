#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include <atomic>

#include "MuteLogic.h"

// 再生中・録音中だけ音声をミュートするエフェクトプラグイン
class RecMuterAudioProcessor final : public juce::AudioProcessor
{
public:
    RecMuterAudioProcessor();
    ~RecMuterAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }

    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // エディタの状態表示用（オーディオスレッドから更新される）
    bool isCurrentlyMuted() const noexcept { return muted.load (std::memory_order_relaxed); }
    bool isHostPlaying() const noexcept { return hostPlaying.load (std::memory_order_relaxed); }
    bool isHostRecording() const noexcept { return hostRecording.load (std::memory_order_relaxed); }

    juce::AudioProcessorValueTreeState parameters;

    static constexpr auto modeParamId = "mode";
    static constexpr auto fadeParamId = "fadeMs";

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    std::atomic<float>* modeParam = nullptr;
    std::atomic<float>* fadeParam = nullptr;

    recmuter::GainRamp ramp;
    double currentSampleRate = 44100.0;

    std::atomic<bool> muted { false };
    std::atomic<bool> hostPlaying { false };
    std::atomic<bool> hostRecording { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RecMuterAudioProcessor)
};
