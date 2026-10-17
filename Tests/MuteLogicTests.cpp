// MuteLogic.h の単体テスト（外部フレームワークに依存しない最小構成）
#include "MuteLogic.h"

#include <cstdio>

namespace
{
int failures = 0;

void check (bool condition, const char* description)
{
    if (! condition)
    {
        std::printf ("FAIL: %s\n", description);
        ++failures;
    }
}

void testShouldMute()
{
    using recmuter::MuteMode;
    using recmuter::shouldMute;

    // 再生または録音
    check (! shouldMute (MuteMode::playOrRecord, false, false), "playOrRecord: 停止中は通過");
    check (shouldMute (MuteMode::playOrRecord, true, false), "playOrRecord: 再生中はミュート");
    check (shouldMute (MuteMode::playOrRecord, true, true), "playOrRecord: 録音中はミュート");
    check (shouldMute (MuteMode::playOrRecord, false, true), "playOrRecord: 録音フラグのみでもミュート");

    // 録音中のみ
    check (! shouldMute (MuteMode::recordOnly, false, false), "recordOnly: 停止中は通過");
    check (! shouldMute (MuteMode::recordOnly, true, false), "recordOnly: 再生中は通過");
    check (shouldMute (MuteMode::recordOnly, true, true), "recordOnly: 録音中はミュート");

    // 再生中のみ
    check (! shouldMute (MuteMode::playOnly, false, false), "playOnly: 停止中は通過");
    check (shouldMute (MuteMode::playOnly, true, false), "playOnly: 再生中はミュート");
    check (! shouldMute (MuteMode::playOnly, true, true), "playOnly: 録音中は通過");
}

void testGainRampFadesOut()
{
    recmuter::GainRamp ramp;
    ramp.setFadeTime (1000.0, 10.0); // 10 サンプルでフェード
    ramp.setTarget (0.0f);

    float last = 1.0f;
    for (int i = 0; i < 10; ++i)
    {
        const auto gain = ramp.next();
        check (gain < last, "フェードアウト中はゲインが単調減少する");
        last = gain;
    }

    check (ramp.isSettled(), "フェード時間経過後は目標値に到達する");
    check (ramp.getCurrent() == 0.0f, "フェードアウト後のゲインは 0");
    check (ramp.next() == 0.0f, "到達後は 0 のまま");
}

void testGainRampFadesIn()
{
    recmuter::GainRamp ramp;
    ramp.setFadeTime (1000.0, 4.0);
    ramp.setTarget (0.0f);
    ramp.snapToTarget();
    check (ramp.getCurrent() == 0.0f, "snapToTarget で即座に目標値になる");

    ramp.setTarget (1.0f);
    for (int i = 0; i < 4; ++i)
        ramp.next();

    check (ramp.getCurrent() == 1.0f, "フェードイン後のゲインは 1");
}

void testZeroFadeIsImmediate()
{
    recmuter::GainRamp ramp;
    ramp.setFadeTime (48000.0, 0.0);
    ramp.setTarget (0.0f);
    check (ramp.next() == 0.0f, "フェード 0ms では即座にミュートされる");
}
} // namespace

int main()
{
    testShouldMute();
    testGainRampFadesOut();
    testGainRampFadesIn();
    testZeroFadeIsImmediate();

    if (failures == 0)
        std::printf ("All tests passed.\n");

    return failures == 0 ? 0 : 1;
}
