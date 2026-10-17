#pragma once

#include <juce_audio_utils/juce_audio_utils.h>

#include "PluginProcessor.h"

// 動作モードとフェード時間の設定、および現在のミュート状態を表示するエディタ
class RecMuterAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                           private juce::Timer
{
public:
    explicit RecMuterAudioProcessorEditor (RecMuterAudioProcessor&);
    ~RecMuterAudioProcessorEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    RecMuterAudioProcessor& processor;

    juce::Label modeLabel { {}, "Mute When" };
    juce::ComboBox modeBox;
    juce::Label fadeLabel { {}, "Fade" };
    juce::Slider fadeSlider { juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight };

    // ComboBox は項目追加後にアタッチする必要があるため、コンストラクタ本体で生成する
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> fadeAttachment;

    // 前回描画時の状態（変化があった時だけ再描画する）
    bool lastMuted = false;
    bool lastPlaying = false;
    bool lastRecording = false;

    juce::Rectangle<int> statusArea;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RecMuterAudioProcessorEditor)
};
