#include "streaming/audio/renderers/coreaudiochannelmap.h"

#include <QStringList>
#include <QTextStream>

using namespace CoreAudioChannelMap;

namespace {
bool require(bool condition, const QString& message, QTextStream& err)
{
    if (!condition) {
        err << "FAIL: " << message << '\n';
    }
    return condition;
}

QString format(const std::vector<int32_t>& map)
{
    QStringList parts;
    for (int32_t channel : map) {
        parts << QString::number(channel);
    }
    return parts.join(' ');
}

// Checks that the device layout maps to the expected per-output channels
bool expectMap(const QString& name, const std::vector<uint32_t>& device, int streamChannels,
               const std::vector<int32_t>& expected, QTextStream& err)
{
    std::vector<int32_t> map;
    bool built = build(device, streamLabels(streamChannels), map);
    return require(built && map == expected,
                   QStringLiteral("%1: got %2 (%3), expected %4")
                       .arg(name, built ? QStringLiteral("success") : QStringLiteral("failure"),
                            format(map), format(expected)),
                   err);
}

bool expectFailure(const QString& name, const std::vector<uint32_t>& device,
                   const std::vector<uint32_t>& stream, QTextStream& err)
{
    std::vector<int32_t> map;
    return require(!build(device, stream, map),
                   QStringLiteral("%1: mapped to %2 instead of failing").arg(name, format(map)),
                   err);
}
} // namespace

int main()
{
    QTextStream err(stderr);
    bool ok = true;

    // The host's WAVE order, with the 5.1 back pair played as surround
    ok &= require(streamLabels(6) == std::vector<uint32_t>{ Left, Right, Center, LFEScreen,
                                                            LeftSurround, RightSurround },
                  QStringLiteral("5.1 stream labels changed"), err);
    ok &= require(streamLabels(8) == std::vector<uint32_t>{ Left, Right, Center, LFEScreen,
                                                            RearSurroundLeft, RearSurroundRight,
                                                            LeftSurround, RightSurround },
                  QStringLiteral("7.1 stream labels changed"), err);
    ok &= require(streamLabels(12).size() == 12 && streamLabels(12)[8] == VerticalHeightLeft &&
                      streamLabels(12)[11] == RightTopRear,
                  QStringLiteral("7.1.4 stream labels changed"), err);
    ok &= require(streamLabels(2).empty() && streamLabels(4).empty(),
                  QStringLiteral("unsupported channel counts got stream labels"), err);

    // Standard layouts, as Audio MIDI Setup writes them
    ok &= expectMap(QStringLiteral("5.1"),
                    { Left, Right, Center, LFEScreen, LeftSurround, RightSurround }, 6,
                    { 0, 1, 2, 3, 4, 5 }, err);
    ok &= expectMap(QStringLiteral("7.1"),
                    { Left, Right, Center, LFEScreen, LeftSurround, RightSurround, RearSurroundLeft,
                      RearSurroundRight },
                    8, { 0, 1, 2, 3, 6, 7, 4, 5 }, err);
    ok &= expectMap(QStringLiteral("7.1.4"),
                    { Left, Right, Center, LFEScreen, LeftSurround, RightSurround, RearSurroundLeft,
                      RearSurroundRight, VerticalHeightLeft, VerticalHeightRight, LeftTopRear,
                      RightTopRear },
                    12, { 0, 1, 2, 3, 6, 7, 4, 5, 8, 9, 10, 11 }, err);

    // HDMI receivers put the LFE before the center
    ok &= expectMap(QStringLiteral("5.1 with the LFE before the center"),
                    { Left, Right, LFEScreen, Center, LeftSurround, RightSurround }, 6,
                    { 0, 1, 3, 2, 4, 5 }, err);
    ok &= expectMap(QStringLiteral("7.1 with the LFE before the center"),
                    { Left, Right, LFEScreen, Center, LeftSurround, RightSurround, RearSurroundLeft,
                      RearSurroundRight },
                    8, { 0, 1, 3, 2, 6, 7, 4, 5 }, err);

    // An audio interface with the speakers on outputs 3-8
    ok &= expectMap(QStringLiteral("5.1 on outputs 3-8"),
                    { Unused, Unused, Left, Right, Center, LFEScreen, LeftSurround, RightSurround,
                      Unused, Unused },
                    6, { -1, -1, 0, 1, 2, 3, 4, 5, -1, -1 }, err);
    ok &=
        expectMap(QStringLiteral("unlabeled outputs left over"),
                  { Unknown, Unknown, Left, Right, Center, LFEScreen, LeftSurround, RightSurround },
                  6, { -1, -1, 0, 1, 2, 3, 4, 5 }, err);

    // A 5.1 stream plays its surround pair on the side speakers of 7.1
    ok &= expectMap(QStringLiteral("5.1 stream on 7.1 speakers"),
                    { Left, Right, Center, LFEScreen, LeftSurround, RightSurround, RearSurroundLeft,
                      RearSurroundRight },
                    6, { 0, 1, 2, 3, 4, 5, -1, -1 }, err);

    // Other names for the same speakers
    ok &= expectMap(QStringLiteral("5.1 labeled rear surround"),
                    { Left, Right, Center, LFEScreen, RearSurroundLeft, RearSurroundRight }, 6,
                    { 0, 1, 2, 3, 4, 5 }, err);
    ok &= expectMap(QStringLiteral("5.1 labeled surround direct"),
                    { Left, Right, Center, LFEScreen, LeftSurroundDirect, RightSurroundDirect }, 6,
                    { 0, 1, 2, 3, 4, 5 }, err);
    ok &= expectMap(QStringLiteral("7.1 labeled surround direct for the sides"),
                    { Left, Right, Center, LFEScreen, LeftSurroundDirect, RightSurroundDirect,
                      RearSurroundLeft, RearSurroundRight },
                    8, { 0, 1, 2, 3, 6, 7, 4, 5 }, err);
    ok &= expectMap(QStringLiteral("7.1.4 labeled top middle and top back"),
                    { Left, Right, Center, LFEScreen, LeftSurround, RightSurround, RearSurroundLeft,
                      RearSurroundRight, LeftTopMiddle, RightTopMiddle, TopBackLeft, TopBackRight },
                    12, { 0, 1, 2, 3, 6, 7, 4, 5, 8, 9, 10, 11 }, err);

    // Speakers that can't be matched send the stream back to SDL
    ok &= expectFailure(QStringLiteral("7.1.4 without height speakers"),
                        { Left, Right, Center, LFEScreen, LeftSurround, RightSurround,
                          RearSurroundLeft, RearSurroundRight, 1 << 16, (1 << 16) + 1,
                          (1 << 16) + 2, (1 << 16) + 3 },
                        streamLabels(12), err);
    ok &= expectFailure(QStringLiteral("5.1 on quadraphonic and center channels"),
                        { Left, Right, Center, 7 /* LeftCenter */, LeftSurround, RightSurround },
                        streamLabels(6), err);
    ok &= expectFailure(QStringLiteral("unlabeled device"),
                        { Unknown, Unknown, Unknown, Unknown, Unknown, Unknown }, streamLabels(6),
                        err);
    ok &= expectFailure(QStringLiteral("7.1 on 5.1 speakers"),
                        { Left, Right, Center, LFEScreen, LeftSurround, RightSurround },
                        streamLabels(8), err);
    ok &= expectFailure(QStringLiteral("fewer outputs than stream channels"),
                        { Left, Right, Center, LFEScreen }, streamLabels(6), err);
    ok &= expectFailure(QStringLiteral("no stream channels"), { Left, Right }, {}, err);

    // An equivalent label isn't taken twice
    ok &= expectFailure(
        QStringLiteral("7.1 on one surround pair"),
        { Left, Right, Center, LFEScreen, LeftSurround, RightSurround, Unused, Unused },
        streamLabels(8), err);

    if (ok) {
        QTextStream(stdout) << "coreaudio_channel_map: all checks passed\n";
    }
    return ok ? 0 : 1;
}
