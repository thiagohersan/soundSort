#pragma once

#include "ofConstants.h"
#include "ofSoundBuffer.h"

#define OF_USING_LAD // libaudiodecoder

#include "audiodecoder.h"

/// reads a sound file into an ofSoundBuffer.
/// encoding support varies by platform.
/// Windows and OSX use libaudiodecoder for decoding, Linux uses libsndfile
bool ofLoadSound(ofSoundBuffer &buffer, string path);

/// writes an ofSoundBuffer as a 16-bit PCM WAV file.
bool ofSaveSound(const ofSoundBuffer &buffer, string path);

class ofSoundFile {
public:
	ofSoundFile();
	ofSoundFile(string path);

	virtual ~ofSoundFile();
	void close();

	/// opens a sound file for reading with readTo().
	/// encoding support varies by platform.
	/// Windows and OSX use libaudiodecoder for decoding, Linux uses libsndfile
	bool loadSound(string _path);

	/// writes an ofSoundBuffer as a 16-bit PCM WAV file
	bool saveSound(string _path, const ofSoundBuffer &buffer);

	/// reads a file into an ofSoundBuffer.
	/// by default, this will resize the buffer to fit the entire file.
	/// supplying a "samples" argument will read only the given number of samples
	bool readTo(ofSoundBuffer &buffer, unsigned int samples = 0);

	/// seek to the sample at the requested index
	bool seekTo(unsigned int sampleIndex);

	/// returns sound file duration in milliseconds
	unsigned long getDuration();
	int getNumChannels();
	int getSampleRate();
	unsigned long getNumSamples();
	bool isCompressed();
	bool isLoaded();
	string getPath();

private:

	bool sfReadFile(ofSoundBuffer & buffer);
	bool mpg123ReadFile(ofSoundBuffer & buffer);
	bool ladReadFile(ofSoundBuffer & buffer);

	bool sfOpen(string path);
	bool mpg123Open(string path);
	bool ladOpen(string path);

	void initDecodeLib();

	AudioDecoder* audioDecoder;

	// common info
	int channels;
	float duration; //in secs
	unsigned int samples;
	int samplerate;
	string path;
	bool bCompressed;
	bool bLoaded;
};
