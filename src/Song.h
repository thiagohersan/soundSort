#pragma once

#include "sample.h"
#include <iostream>
#include <algorithm>
#include <vector>
using namespace std;

#define MAX_SAMPLE_VAL 32767

class Song {
    Sample** oldOrder;
    Sample** newOrder;
    unsigned int numFrames;
    unsigned int numChannels;

    public:
        Song(string filename);
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
