# CoreAudio channel map

This standalone Qt test compiles the real
`app/streaming/audio/renderers/coreaudiochannelmap.cpp`, which decides which
output of a surround device plays each channel of the stream when the macOS
CoreAudio renderer passes surround through. It checks the standard 5.1, 7.1
and 7.1.4 layouts, HDMI receivers that put the LFE before the center, speakers
on outputs 3-8 of an audio interface, a 5.1 stream on 7.1 speakers, other names
for the same speakers, and the layouts that must fall back to the SDL renderer.

The mapping code has no CoreAudio dependency, so the test builds on any
platform. From an empty build directory, run:

```sh
qmake /path/to/moonlight-qt/tests/coreaudio_channel_map/coreaudio_channel_map.pro
make && ./coreaudio_channel_map
```
