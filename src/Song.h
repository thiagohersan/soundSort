#pragma once

#include "sample.h"
#include <iostream>
#include <algorithm>
#include <vector>

class Song {
    Sample** oldOrder;
    Sample** newOrder;
    int size;

    public:
        Song(int s);
        ~Song();

        void initSampleAt(int i, short v);
        short getOldSample(int i);
        short getNewSample(int i);
        void  orderNew();
        void  setOldSample(int i, short v);
        void  setNewSample(int i, short v);
        int   getSize();
};
