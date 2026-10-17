#pragma once

#include <algorithm>
#include <cmath>

// JUCE に依存しないミュート判定ロジック。
// 単体テストからも利用できるよう、ヘッダーのみで完結させている。
namespace recmuter
{

// どのトランスポート状態でミュートするか
enum class MuteMode
{
    playOrRecord = 0, // 再生中または録音中にミュート
    recordOnly,       // 録音中のみミュート
    playOnly          // 再生中（録音していない時）のみミュート
};

// トランスポート状態からミュートすべきかを判定する
constexpr bool shouldMute (MuteMode mode, bool isPlaying, bool isRecording) noexcept
{
    switch (mode)
    {
        case MuteMode::playOrRecord: return isPlaying || isRecording;
        case MuteMode::recordOnly:   return isRecording;
        case MuteMode::playOnly:     return isPlaying && ! isRecording;
    }

    return false;
}

// ミュート切り替え時のクリックノイズを防ぐための線形ゲインランプ
class GainRamp
{
public:
    // サンプルレートとフェード時間（ミリ秒）を設定する
    void setFadeTime (double sampleRate, double fadeMs) noexcept
    {
        const auto samples = sampleRate * fadeMs / 1000.0;
        step = samples >= 1.0 ? static_cast<float> (1.0 / samples) : 1.0f;
    }

    void setTarget (float newTarget) noexcept { target = newTarget; }

    // 現在値を即座に目標値へ合わせる（再生開始前の初期化用）
    void snapToTarget() noexcept { current = target; }

    // 1 サンプル分進めて、そのサンプルに掛けるゲインを返す
    float next() noexcept
    {
        if (current < target)
            current = std::min (current + step, target);
        else if (current > target)
            current = std::max (current - step, target);

        return current;
    }

    // next() は目標値でクランプするため、到達後は厳密に一致する
    bool isSettled() const noexcept { return ! (current < target) && ! (current > target); }
    float getCurrent() const noexcept { return current; }
    float getTarget() const noexcept { return target; }

private:
    float current = 1.0f;
    float target = 1.0f;
    float step = 1.0f;
};

} // namespace recmuter
