#include "coreaudiochannelmap.h"

namespace CoreAudioChannelMap {

namespace {

// Labels that name the same speaker, in order of preference. Devices and
// Audio MIDI Setup don't agree on which ones to use for surround and height
// speakers, and the WAVE layouts call the 5.1 surround pair rear surround.
const std::vector<uint32_t> Equivalents[] = {
    { LeftSurround, RearSurroundLeft, LeftSurroundDirect },
    { RightSurround, RearSurroundRight, RightSurroundDirect },
    { VerticalHeightLeft, LeftTopMiddle },
    { VerticalHeightRight, RightTopMiddle },
    { LeftTopRear, TopBackLeft },
    { RightTopRear, TopBackRight },
};

bool assign(const std::vector<uint32_t>& deviceLabels, uint32_t label, int32_t channel,
            std::vector<int32_t>& map)
{
    for (size_t i = 0; i < deviceLabels.size(); i++) {
        if (deviceLabels[i] == label && map[i] < 0) {
            map[i] = channel;
            return true;
        }
    }
    return false;
}

} // namespace

std::vector<uint32_t> streamLabels(int channelCount)
{
    switch (channelCount) {
    case 6:
        return { Left, Right, Center, LFEScreen, LeftSurround, RightSurround };
    case 8:
        return { Left,         Right,        Center, LFEScreen, RearSurroundLeft, RearSurroundRight,
                 LeftSurround, RightSurround };
    case 12:
        return { Left,
                 Right,
                 Center,
                 LFEScreen,
                 RearSurroundLeft,
                 RearSurroundRight,
                 LeftSurround,
                 RightSurround,
                 VerticalHeightLeft,
                 VerticalHeightRight,
                 LeftTopRear,
                 RightTopRear };
    default:
        return {};
    }
}

bool build(const std::vector<uint32_t>& deviceLabels, const std::vector<uint32_t>& streamLabels,
           std::vector<int32_t>& map)
{
    map.assign(deviceLabels.size(), -1);
    if (streamLabels.empty()) {
        return false;
    }

    std::vector<bool> placed(streamLabels.size(), false);

    // Exact matches go first, so a 7.1 stream's side and back pairs each find
    // their own speakers before either falls back to the other's
    for (size_t i = 0; i < streamLabels.size(); i++) {
        if (streamLabels[i] != Unused && streamLabels[i] != Unknown) {
            placed[i] = assign(deviceLabels, streamLabels[i], (int32_t)i, map);
        }
    }

    for (size_t i = 0; i < streamLabels.size(); i++) {
        if (placed[i]) {
            continue;
        }

        for (const std::vector<uint32_t>& group : Equivalents) {
            bool inGroup = false;
            for (uint32_t label : group) {
                inGroup |= label == streamLabels[i];
            }
            if (!inGroup) {
                continue;
            }

            for (auto label = group.begin(); label != group.end() && !placed[i]; ++label) {
                placed[i] = assign(deviceLabels, *label, (int32_t)i, map);
            }
            break;
        }

        if (!placed[i]) {
            return false;
        }
    }
    return true;
}

} // namespace CoreAudioChannelMap
