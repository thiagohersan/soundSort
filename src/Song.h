#pragma once

#include "sample.h"
#include <iostream>
#include <algorithm>
#include <vector>

class Song {
    Sample** oldOrder;
    Sample** newOrder;
    int foo;
    unsigned int numFrames;
    unsigned int numChannels;

    public:
        Song(unsigned int nFrames, unsigned int nCchannels);
        ~Song();

        void initSampleAt(int i, short v);
        short getOldSample(int i);
        short getNewSample(int i);
        void orderSamples();
        void setOldSample(int i, short v);
        void setNewSample(int i, short v);
        unsigned int getNumFrames() const;
        unsigned int getNumChannels() const;
};
