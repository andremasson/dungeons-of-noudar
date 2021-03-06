//
// Created by monty on 10/10/16.
//
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include <stdio.h>

#ifdef __APPLE__
#if TARGET_IOS
#import <OpenAl/al.h>
#import <OpenAl/alc.h>
#include <AudioToolbox/AudioToolbox.h>
#else
#import <OpenAl/al.h>
#import <OpenAl/alc.h>
#include <AudioToolbox/AudioToolbox.h>

#endif
#else

#include <AL/al.h>
#include <AL/alc.h>

#endif


#include "glm/glm.hpp"

#include "SoundListener.h"

ALCdevice* device;
ALCcontext* context;

namespace odb {
    SoundListener::SoundListener() {
        printf("Default device: %s\n", alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER));

        device = alcOpenDevice(NULL);
        context = alcCreateContext(device, NULL);
        alcMakeContextCurrent(context);

        printf("OpenAL version: %s\n", alGetString(AL_VERSION));
        printf("OpenAL vendor: %s\n", alGetString(AL_VENDOR));
        printf("OpenAL renderer: %s\n", alGetString(AL_RENDERER));

    }
}
