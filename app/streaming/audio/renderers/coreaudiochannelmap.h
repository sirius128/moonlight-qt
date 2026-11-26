#pragma once

#include <cstdint>
#include <vector>

// Routing of the stream's channels to the outputs of a surround device, kept
// free of CoreAudio so it can be tested on any platform. Labels are
// AudioChannelLabel values, which coreaudio.cpp checks against the SDK.
namespace CoreAudioChannelMap {

enum Label : uint32_t
{
    Unknown = 0xFFFFFFFF,
    Unused = 0,
    Left = 1,
    Right = 2,
    Center = 3,
    LFEScreen = 4,
    LeftSurround = 5,
    RightSurround = 6,
    LeftSurroundDirect = 10,
    RightSurroundDirect = 11,
    VerticalHeightLeft = 13,
    VerticalHeightRight = 15,
    TopBackLeft = 16,
    TopBackRight = 18,
    RearSurroundLeft = 33,
    RearSurroundRight = 34,
    LeftTopMiddle = 49,
    RightTopMiddle = 51,
    LeftTopRear = 52,
    RightTopRear = 54,
};

// The speaker each channel of a surround stream is meant for, in the host's
// WAVE order: FL FR FC LFE BL BR SL SR (TFL TFR TBL TBR). The 5.1 back pair is
// labeled surround, which is where the SDL renderer has always played it on
// 5.1 and 7.1 speakers. Empty for channel counts the host doesn't send.
std::vector<uint32_t> streamLabels(int channelCount);

// Builds a kAudioOutputUnitProperty_ChannelMap: one entry per device output
// holding the stream channel it plays, or -1 for silence. Each stream channel
// goes to the output with the same label, or failing that to one with a label
// that means the same speaker. Returns false if there are no stream channels
// or any of them is left without an output.
bool build(const std::vector<uint32_t>& deviceLabels, const std::vector<uint32_t>& streamLabels,
           std::vector<int32_t>& map);

} // namespace CoreAudioChannelMap
