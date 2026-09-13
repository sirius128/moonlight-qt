#pragma once

#include "TPCircularBuffer.h"

#include <AudioUnit/AudioUnit.h>
#include <AudioToolbox/AudioToolbox.h>

#include <Limelight.h>

// Frames the spatial mixer may be asked for in one callback. The output buffer
// the mixer renders into (CoreAudioRenderer::m_SpatialBuffer) is sized to match.
constexpr uint32_t kSpatialMaxFramesPerSlice = 4096;
constexpr uint32_t kSpatialOutputChannels = 2;

class AUSpatialRenderer
{
public:
    AUSpatialRenderer();
    ~AUSpatialRenderer();

    double getAudioUnitLatency();
    void setRingBufferPtr(const TPCircularBuffer* __nonnull buffer);
    bool setup(AUSpatialMixerOutputType outputType, float sampleRate, int inChannelCount);
    OSStatus setStreamFormatAndACL(float inSampleRate, AudioChannelLayoutTag inLayoutTag, AudioUnitScope inScope, AudioUnitElement inElement);
    OSStatus setOutputType(AUSpatialMixerOutputType outputType);
    OSStatus process(AudioBufferList* __nullable outputABL, AudioUnitRenderActionFlags* __nonnull ioActionFlags, const AudioTimeStamp* __nullable inTimestamp, float inNumberFrames);

    friend OSStatus inputCallback(void * _Nonnull,
                    AudioUnitRenderActionFlags *_Nullable,
                    const AudioTimeStamp * _Nullable,
                    uint32_t, uint32_t,
                    AudioBufferList * _Nonnull);

    uint32_t m_HeadTracking;
    uint32_t m_PersonalizedHRTF;

private:
    void buildChannelMap(int inChannelCount);

    AudioUnit _Nonnull m_Mixer;
    const TPCircularBuffer* _Nonnull m_RingBufferPtr; // pointer to RingBuffer in the outer CoreAudioRenderer

    double m_AudioUnitLatency;

    // Maps a spatial mixer input channel to its index in the interleaved PCM we
    // receive from the host. Identity unless the host's channel order differs
    // from the CoreAudio layout tag we hand to the mixer.
    uint32_t m_ChannelMap[AUDIO_CONFIGURATION_MAX_CHANNEL_COUNT];
};
