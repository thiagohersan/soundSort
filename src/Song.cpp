#include "Song.h"
#include "ofSoundFile.h"

Song::~Song(){
    for(int i=0; i<numFrames; i++){
        delete oldOrder[i];
    }
    delete[] oldOrder;
    delete[] newOrder;
}

Song::Song(string filename){
    // open file, read samples into buffers
    ofSoundFile mSoundFile;
    ofSoundBuffer mSoundBuffer;

    mSoundFile.loadSound(filename);
    mSoundFile.readTo(mSoundBuffer);
    numChannels = mSoundFile.getNumChannels();
    numFrames = mSoundFile.getNumSamples()/mSoundFile.getNumChannels();
    sampleRate = mSoundFile.getSampleRate();
    oldOrder = new Sample*[numFrames];
    newOrder = new Sample*[numFrames];

    cout << "file has " << numFrames << " frames and " << numChannels << " channels" << endl;

    // read into short buffer
    float *ft = new float[numChannels*numFrames];
    mSoundBuffer.copyTo(ft, numFrames, numChannels, 0);
    short *tempBuffer = new short[numChannels*numFrames];
    for(unsigned int i=0; i<numChannels*numFrames; i++){
        tempBuffer[i] = (short) ofMap(ft[i], -1, 1, -MAX_SAMPLE_VAL, MAX_SAMPLE_VAL, true);
    }
    mSoundBuffer.clear();
    mSoundFile.close();

    // read audio values into data structs and find maxVal
    short maxVal = 0;
    for(unsigned int i=0; i<numFrames; i++) {
        // make mono
        int mv = (tempBuffer[numChannels*i+0] + tempBuffer[numChannels*i+1])/2;
        this->initSampleAt(i,mv);

        // find largest sample value
        if (abs(mv) > maxVal) {
            maxVal = abs(mv);
        }
    }
    cout << "--max val: " << maxVal << endl;

    // don't need these anymore! delete!
    delete[] ft;
    delete[] tempBuffer;

    // scale values, put them in the arrays
    for(int i=0; i<numFrames; i++) {
        // scale the fucker
        float t = (float)(this->getOldSample(i));
        t *= (float)(MAX_SAMPLE_VAL)/(float)(maxVal);

        // put back in the Sample
        this->setOldSample(i,(short)t);
    }
    cout << "--scaled samples in the arrays" << endl;
}

void Song::saveToFile(string filename){
    ofSoundFile mSoundFile;
    ofSoundBuffer mSoundBuffer;
    short *tempBuffer = new short[numChannels*numFrames];
    for(unsigned int i=0; i<numFrames; i++){
        tempBuffer[numChannels*i] = this->getOldSample(i);
        tempBuffer[numChannels*i+1] = this->getOldSample(i);
    }
    mSoundBuffer.copyFrom(tempBuffer, numFrames, 2, sampleRate);
    mSoundFile.saveSound(filename, mSoundBuffer);
    delete[] tempBuffer;
    mSoundBuffer.clear();
    mSoundFile.close();
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
    return oldOrder[i]->getSample();
}

// grab sample from new array
short Song::getNewSample(int i){
    return newOrder[i]->getSample();
}

// set value of sample from original ordered array
void Song::setOldSample(int i, short v){
    oldOrder[i]->setValue(v);
}

// set value of sample from new array
void Song::setNewSample(int i, short v){
    newOrder[i]->setValue(v);
}

unsigned int Song::getNumFrames() const{
    return numFrames;
}
unsigned int Song::getNumChannels() const{
    return numChannels;
}
