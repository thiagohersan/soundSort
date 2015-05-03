#include "Song.h"

Song::~Song(){
    for(int i=0; i<numFrames; i++){
        delete oldOrder[i];
    }
    delete[] oldOrder;
    delete[] newOrder;
    int foo;
}

Song::Song(unsigned int nFrames, unsigned int nCchannels){
    // two arrays of pointers
    oldOrder = new Sample*[nFrames];
    newOrder = new Sample*[nFrames];
    numFrames = nFrames;
    numChannels = nCchannels;
}

// IMPORTANT : creates a sample obj!
void Song::initSampleAt(int i, short v){
    Sample *s = new Sample(v,i);
    oldOrder[i] = s;
    newOrder[i] = s;
}

// local helper function to compare values of samples
bool SampleComp(const Sample* s0, const Sample* s1){
    return (s0->getSample() < s1->getSample());
}


// order the array into newOrder
void Song::orderSamples(){
    // shove the array into a vector
    std::vector<Sample*> tempV(newOrder, newOrder+numFrames);
    sort(tempV.begin(), tempV.end(), SampleComp);

    for(int i=0; i<numFrames; i++){
        // set the new index value for each sample.
        // iterate through vector, get original index of sample at i,
        // and use it to index the original-ordered array
        (oldOrder[(tempV.at(i))->getOldIndex()])->setNewIndex(i);

        // also stick sample in the new array
        newOrder[i] = tempV.at(i);
    }

    // sanity check
    //std::cout << "value at old[26700]: " << (oldOrder[26700])->getSample() << std::endl;
    //std::cout << "value at new[old[26700].new]: " << (newOrder[(oldOrder[26700])->getNewIndex()])->getSample() << std::endl;
}

// grab sample from original ordered array
short Song::getOldSample(int i){
    return (oldOrder[i]->getSample());
}

// grab sample from new array
short Song::getNewSample(int i){
    return (newOrder[i]->getSample());
}

// set value of sample from original ordered array
void Song::setOldSample(int i, short v){
    (oldOrder[i]->setValue(v));
}

// set value of sample from new array
void Song::setNewSample(int i, short v){
    (newOrder[i]->setValue(v));
}

unsigned int Song::getNumFrames() const{
    return numFrames;
}
unsigned int Song::getNumChannels() const{
    return numChannels;
}
