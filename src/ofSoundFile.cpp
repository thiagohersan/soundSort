
#include "ofSoundFile.h"
#include "ofLog.h"
#include "ofUtils.h"

//--------------------------------------------------------------
bool ofLoadSound(ofSoundBuffer &buff, string path){
    ofSoundFile sf(path);
    if(sf.isLoaded()){
        sf.readTo(buff);
        return true;
    }else{
        return false;
    }
}

//--------------------------------------------------------------
// for now this only write 16 bit PCM WAV files.
// It can't really live in ofSoundFile yet because
// ofSoundFile doesn't hold a complete representation
// the sound file that can be written to disk. You'd
// need something that would let you stream the data to
// it via writeTo() or similar. Doesn't really fit wtih
// the current model.
bool ofSaveSound(const ofSoundBuffer &buff,  string path){
	ofSoundFile soundFile;
	return soundFile.saveSound(path, buff);
}

//--------------------------------------------------------------
ofSoundFile::ofSoundFile() {
    bCompressed = false;
    bLoaded = false;
	audioDecoder = NULL;
	close();
}

//--------------------------------------------------------------
ofSoundFile::ofSoundFile(string path) {
    ofSoundFile();
    loadSound(path);
}

//--------------------------------------------------------------
ofSoundFile::~ofSoundFile() {
	close();
}

//--------------------------------------------------------------
bool ofSoundFile::loadSound(string _path){
 	path = ofToDataPath(_path);
	bool result = false;
	if(ofFilePath::getFileExt(path)=="mp3"){
        bCompressed=true;
        result = ladOpen(path);
	}else{
        result = ladOpen(path);
	}
	if(result) { // prevent div by zero if file doesn't open.
		duration = float(samples/channels) / float(samplerate);
	}
	bLoaded = result;
	return result;
}

//--------------------------------------------------------------
bool ofSoundFile::saveSound(string path, const ofSoundBuffer &buff){
	// check that we're writing a wav and complain if the file extension is wrong.
	ofFile f(path);
	if(ofToLower(f.getExtension())!="wav") {
		path += ".wav";
		ofLogWarning() << "Can only write wav files - will save file as " << path;
	}

	fstream file(ofToDataPath(path).c_str(), ios::out | ios::binary);
	if(!file.is_open()) {
		ofLogError() << "Error opening sound file '" << path << "' for writing";
		return false;
	}

	// write a wav header
	short myFormat = 1; // for pcm
	int mySubChunk1Size = 16;
	int bitsPerSample = 16; // assume 16 bit pcm
	int myByteRate = buff.getSampleRate() * buff.getNumChannels() * bitsPerSample/8;
	short myBlockAlign = buff.getNumChannels() * bitsPerSample/8;
	int myChunkSize = 36 + buff.size()*bitsPerSample/8;
	int myDataSize = buff.size()*bitsPerSample/8;
	int channels = buff.getNumChannels();
	int samplerate = buff.getSampleRate();

	file.seekp (0, ios::beg);
	file.write ("RIFF", 4);
	file.write ((char*) &myChunkSize, 4);
	file.write ("WAVE", 4);
	file.write ("fmt ", 4);
	file.write ((char*) &mySubChunk1Size, 4);
	file.write ((char*) &myFormat, 2); // should be 1 for PCM
	file.write ((char*) &channels, 2); // # channels (1 or 2)
	file.write ((char*) &samplerate, 4); // 44100
	file.write ((char*) &myByteRate, 4); //
	file.write ((char*) &myBlockAlign, 2);
	file.write ((char*) &bitsPerSample, 2); //16
	file.write ("data", 4);
	file.write ((char*) &myDataSize, 4);

	// write the wav file per the wav file format, 4096 bytes of data at a time.
#define WRITE_BUFF_SIZE 4096

	short writeBuff[WRITE_BUFF_SIZE];
	int pos = 0;
	while(pos<buff.size()) {
		int len = MIN(WRITE_BUFF_SIZE, buff.size()-pos);
		for(int i = 0; i < len; i++) {
			writeBuff[i] = (int)(buff[pos]*32767.f);
			pos++;
		}
		file.write((char*)writeBuff, len*bitsPerSample/8);
	}

	file.close();
	return true;
}

//--------------------------------------------------------------

bool ofSoundFile::ladOpen(string path){
	audioDecoder = new AudioDecoder(ofToDataPath(path));
	int result = audioDecoder->open();

	samples = audioDecoder->numSamples();
	channels = audioDecoder->channels();
	samplerate = audioDecoder->sampleRate();
	duration = audioDecoder->duration();
	return result == AUDIODECODER_OK;
}


//--------------------------------------------------------------
void ofSoundFile::close(){
	if ( audioDecoder )
		delete audioDecoder;
	audioDecoder = NULL;
	channels = 1;
	duration = 0; //in secs
	samplerate = 0;
	samples = 0;
}

//--------------------------------------------------------------
bool ofSoundFile::isLoaded(){
    return bLoaded;
}

//--------------------------------------------------------------
int ofSoundFile::getNumChannels(){
	return channels;
}

//--------------------------------------------------------------
unsigned long ofSoundFile::getDuration(){
	return duration*1000;
}

//--------------------------------------------------------------
int ofSoundFile::getSampleRate(){
	return samplerate;
}

//--------------------------------------------------------------
unsigned long ofSoundFile::getNumSamples(){
    return samples;
}

//--------------------------------------------------------------
bool ofSoundFile::isCompressed(){
    return bCompressed;
}

//--------------------------------------------------------------
string ofSoundFile::getPath(){
    return path;
}

//--------------------------------------------------------------
bool ofSoundFile::readTo(ofSoundBuffer & buffer, unsigned int _samples){
	buffer.setNumChannels(channels);
	buffer.setSampleRate(samplerate);
	if(_samples!=0){
		// will read the requested number of samples
		// clamp to the number of samples we actually have
		_samples = min(_samples,samples);
		buffer.resize(_samples*channels);
	}
	else if (audioDecoder) {
		// will read entire file
		buffer.resize(samples);
	}

	if(audioDecoder) return ladReadFile(buffer);

	return false;
}
//--------------------------------------------------------------
bool ofSoundFile::seekTo(unsigned int sample){
	sample = min(samples,sample);
	if(audioDecoder) audioDecoder->seek(sample);
	else return false;

	return true; //TODO: check errors
}

bool ofSoundFile::ladReadFile(ofSoundBuffer &buffer){

	int samplesRead = audioDecoder->read( buffer.size(), &buffer[0] );
	return samplesRead;
}
