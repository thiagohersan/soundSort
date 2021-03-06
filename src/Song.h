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
    string songName;
    unsigned int numFrames;
    unsigned int numChannels;
    unsigned int sampleRate;
    void initSampleAt(int i, short v);

    public:
        Song(string filename);
        ~Song();
        void saveToFile(string filename);

        short getOldSample(int i);
        short getNewSample(int i);
        void orderSamples();
        void setOldSample(int i, short v);
        void setNewSample(int i, short v);
        unsigned int getNumFrames() const;
        unsigned int getNumChannels() const;
        const string& getSongName() const;
};
