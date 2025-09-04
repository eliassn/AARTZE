#include "AudioSystem.hpp"
#ifdef HAVE_OPENAL
#include <AL/al.h>
#include <AL/alc.h>

bool AudioSystem::Initialize()
{
    ALCdevice* device = alcOpenDevice(nullptr);
    if (!device) return false;
    ALCcontext* ctx = alcCreateContext(device, nullptr);
    if (!ctx) { alcCloseDevice(device); return false; }
    alcMakeContextCurrent(ctx);
    return true;
}

void AudioSystem::Shutdown()
{
    ALCcontext* ctx = alcGetCurrentContext();
    ALCdevice* device = ctx ? alcGetContextsDevice(ctx) : nullptr;
    if (ctx) { alcMakeContextCurrent(nullptr); alcDestroyContext(ctx); }
    if (device) { alcCloseDevice(device); }
}
#else
bool AudioSystem::Initialize(){ return true; }
void AudioSystem::Shutdown(){}
#endif
