#pragma once

#include "ofConstants.h"
#include "ofMath.h"

inline void ofStereoVolumes(float volume, float pan, float & left, float & right){
	pan = ofClamp(pan, -1, 1);
	// calculates left/right volumes from pan-value (constant panning law)
	// see: Curtis Roads: Computer Music Tutorial p 460
	// thanks to jasch
	float angle = pan * 0.7853981633974483f; // in radians from -45. to +45.
	float cosAngle = cos(angle);
	float sinAngle = sin(angle);
	left  = (cosAngle - sinAngle) * 0.7071067811865475 * volume; // multiplied by sqrt(2)/2
	right = (cosAngle + sinAngle) * 0.7071067811865475 * volume; // multiplied by sqrt(2)/2
}


/*!

 @brief Buffer for audio samples and associated metadata.

 ofSoundBuffer stores audio as an array of interleaved floating point samples, with a given sample rate for playback.

 #### How sound recording works

 Physically speaking, what we call _sound_ is simply changes in air pressure perceived by a listener. These changes in sound pressure are converted
 by a microphone into changes in voltage that can be recorded, making a _sound recording_. A sound recording is therefore a recording of changes in air
 pressure at a particular point in space (ie where the microphone was positioned). When it is played back through a speaker, the speaker reproduces the
 same pattern of changes in air pressure as were recorded by the microphone, but this time at a different point in space (ie where the speaker is positioned).

 In digital audio these changes in air pressure are recorded as a set of discrete numbers (_samples_), each number representing a snapshot of the air pressure
 at a particular point in time. For high quality audio there are typically 44,100 snapshots recorded every second. This is called the _sample rate_ and is
 expressed in _Hz_ (44100Hz) or _kHz_ (44.1kHz).

 Because humans have two ears, rather than one, sound is usually recorded in stereo. The simplest stereo sound recording is two _channels_ of sound
 recorded by two microphones at two different points in space. More channels can also be recorded (eg with 5.1 surround sound systems or Ambisonics).


 #### Frames, channels and samples

 Data in an ofSoundBuffer is stored _interleaved_ as an array of floats. Interleaved audio is analogous to how different color channels are stored side by side
 in an ofImage or ofPixels object.

 The functions and function arguments in ofSoundBuffer that deal with this interleaved data are based on 3 key terms:

 _channels_ refers to the number of channels or individual streams of interleaved audio samples in the buffer. A mono recording has 1 channel, a stereo
 recording has 2 channels.

 _samples_ refers to the actual raw data. One _sample_ is a single floating point number between -1 and 1, which represents a snapshot of sound pressure at
 a single moment in time. A 0.1 second long buffer at 44100Hz contains 4410 _samples_ if it has 1 channel, 8820 _samples_ if it has 2 channels, 13230
 _samples_ if it has 3 channels, and so on.

 _frames_ refers to the number of multi-channel sets of interleaved sample data there are in the buffer. A 0.1 second long buffer at 44100Hz always has
 4410 _frames_, regardless of how many channels it has. To get the number of _samples_ in a buffer you multiply the number of _channels_ by the number of _frames_.


 If I have an ofSoundBuffer with 8 _frames_ of mono (1 _channel_) audio, the underlying array contains 8 _samples_ (ie it is 8 floats long),
 and the samples are arranged like this:

 L L L L L L L L

 where `L` represents a single sample.


 If I have an ofSoundBuffer with 8 _frames_ of stereo (2 _channel_) audio, then the underlying array contains 16 _samples_ ((getNumFrames()*getNumChannels(),
 ie 8*2) arranged in an _interleaved_ pattern:

 L R L R L R L R L R L R L R L R

 where `L` represents a single sample for the left channel, and `R` represents a single sample for the right channel. Grouping the frames together for clarity:

 LR LR LR LR LR LR LR LR


 If I have an ofSoundBuffer with 8 frames of 5.1 surround (6 _channel_) audio, then the underlying array of floats contains 48 _samples_ (getNumFrames()*getNumChannels(),
 ie 8*6) and is usually arranged in an _interleaved_ pattern like this:

 L C R Ls Rs Lfe L C R Ls Rs Lfe L C R Ls Rs Lfe L C R Ls Rs Lfe L C R Ls Rs Lfe L C R Ls Rs Lfe L C R Ls Rs Lfe L C R Ls Rs Lfe

 where `L` represents a single sample for the left channel, `C` for centre, `R` for right, `Ls` for left surround, `Rs` for right surround and `Lfe` for the subwoofer.
 Grouping the frames together for clarity:

 LCRLsRsLfe LCRLsRsLfe LCRLsRsLfe LCRLsRsLfe LCRLsRsLfe LCRLsRsLfe LCRLsRsLfe LCRLsRsLfe

 */


class ofSoundBuffer {
public:
	ofSoundBuffer();
	/// initialise by converting samples from a signed short (16-bit) buffer containing numFrames frames of numChannels channels of audio data.
	ofSoundBuffer(short * shortBuffer, unsigned int numFrames, int numChannels, int sampleRate);

	enum InterpolationAlgorithm{
		Linear,
		Hermite
	};
	static InterpolationAlgorithm defaultAlgorithm;  //defaults to Linear for mobile, Hermite for desktop

	/// sample rate of the audio in this buffer
	int getSampleRate() const { return samplerate; }
	void setSampleRate(int rate);
	/// resample by changing the playback speed, keeping the same sampleRate
	void resample(float speed, InterpolationAlgorithm algorithm=defaultAlgorithm);
	/// the number of channels per frame
	int getNumChannels() const { return channels; }
	/// set the number of channels. does not change the underlying data, ie causes getNumFrames() to return a different result.
	void setNumChannels(int channels);
	/// the number of frames, ie the number of sets of (getNumChannels()) samples
	unsigned long getNumFrames() const { return size()/getNumChannels(); }

	/// return the tickCount that was assigned by ofSoundStream (if this buffer originated from an ofSoundStream).
	unsigned long long getTickCount() const { return tickCount; }

	/// return the duration of audio in this buffer in milliseconds (==(getNumFrames()/getSampleRate())*1000)
	unsigned long getDurationMS() const;

	/// access the sample at the given position in the buffer.
	/// to retrieve the sample for channel channelIndex of frame frameIndex, do the following:
	/// ofSoundBuffer myBuffer;
	/// ...
	/// float sample = myBuffer[(frameIndex*myBuffer.getNumChannels()) + channelIndex];
	float & operator[](unsigned int samplePos);
	const float & operator[](unsigned int samplePos) const;

	/// return a new buffer containing the contents of this buffer multiplied by value.
	ofSoundBuffer operator*(float value);
	/// multiply everything in this buffer by value, in-place.
	ofSoundBuffer & operator*=(float value);

	/// assuming a 2-channel buffer, apply a stereo pan by multiplying channel 0 by left and channel 1 by right.
	void stereoPan(float left, float right);

	/// copy length samples from shortBuffer and interpret as interleaved with the given number of channels at the given samplerate
	void copyFrom(short * shortBuffer, unsigned int length, int channels, int samplerate);

	/// resize ofSoundBuffer to outNumFrames with outNumChannels, and then copy outNumFrames of data from us to soundBuffer.
	/// fromFrame is a frame offset. if we don't have enough source data, loop with fromFrame=0 until we have filled outBuffer.
	/// if outBuffer has fewer channels than our buffer, just copy the first outNumChannels of our data and skip the rest.
	/// if outBuffer has more channels than our buffer, loop through our channels repeatedly until done.
	void copyTo(ofSoundBuffer & outBuffer, unsigned int outNumFrames, unsigned int outNumChannels, unsigned int fromFrame,bool loop=false) const;
	/// as copyTo but mixes source audio with audio in `outBuffer` by adding samples together (+), instead of overwriting.
	void addTo(ofSoundBuffer & outBuffer, unsigned int outNumFrames, unsigned int outNumChannels, unsigned int fromFrame,bool loop=false) const;

	/// as copyTo above but reads outNumFrames and outNumChannels from outBuffer
	void copyTo(ofSoundBuffer & outBuffer, unsigned int frameFrame = 0, bool loop=false) const;
	/// as addTo above but reads outNumFrames and outNumChannels from outBuffer
	void addTo(ofSoundBuffer & outBuffer, unsigned int fromFrame = 0, bool loop=false) const;

	/// copy sample data to out, where out is already allocated to match outNumFrames and outNumChannels (ie outNumFrames*outNumChannels samples).
	/// fromFrame is a frame offset. if we don't have enough source data, loop with fromFrame=0 until we have filled the out buffer.
	/// if out has fewer channels than our buffer, just copy the first outNumChannels of our data and skip the rest.
	/// if out has more channels than our buffer, loop through our channels repeatedly until done.
	void copyTo(float * out, unsigned int outNumFrames, unsigned int outNumChannels,unsigned int fromFrame,bool loop=false) const;
	/// as copyTo but mixes source audio with audio in `out` by adding samples together (+), instead of overwriting
	void addTo(float * out, unsigned int outNumFrames, unsigned int outNumChannels,unsigned int fromFrame,bool loop=false) const;

	/// resample our data to outBuffer at the given target speed, starting at fromFrame and copying numFrames of data. resize outBuffer to fit.
	/// speed is relative to current speed (ie 1.0f == no change). lower speeds will give a larger outBuffer, higher speeds a smaller outBuffer.
	void resampleTo(ofSoundBuffer & outBuffer, unsigned int fromFrame, unsigned int numFrames, float speed, bool loop=false, InterpolationAlgorithm algorithm=defaultAlgorithm);

	/// copy the requested channel of our data to outBuffer. resize outBuffer to fit.
	void getChannel(ofSoundBuffer & outBuffer, int sourceChannel);
	/// copy data from inBuffer to the given channel. resize ourselves to match inBuffer's getNumFrames().
	void setChannel(const ofSoundBuffer & inBuffer, int channel);

	float getRMSAmplitude();
	float getRMSAmplitudeChannel(unsigned int channel);

	void linearResampleTo(ofSoundBuffer & buffer, unsigned int fromFrame, unsigned int numFrames, float speed, bool loop);
	void hermiteResampleTo(ofSoundBuffer & buffer, unsigned int fromFrame, unsigned int numFrames, float speed, bool loop);

	/// fills the buffer with random noise between -amplitude and amplitude. useful for debugging.
	void fillWithNoise(float amplitude = 1);

	/// fills the buffer with a sine wave. useful for debugging.
	float fillWithTone(float pitchHz = 440., float phase = 0);

	/// amplifies samples so that the maximum amplitude is equal to 'level'
	void normalize(float level = 1);

	/// removes initial / ending silence from the buffer
	bool trimSilence(float threshold = 0.0001, bool trimStart = true, bool trimEnd = true);

	/// return the total number of samples in this buffer (==getNumFrames()*getNumChannels())
	unsigned long size() const { return buffer.size(); }
	/// resize this buffer to exactly this many samples. it's up to you make sure samples matches the channel count.
	void resize(unsigned int numSamples, float val=float());
	/// remove all samples, preserving channel count and sample rate.
	void clear();
	/// swap the contents of this buffer with otherBuffer
	void swap(ofSoundBuffer & otherBuffer);
	/// set everything in this buffer to value, preserving size, channel count and sample rate.
	void set(float value);
	/// copy data from source, interpreting it as nFrames frames of nChannels channels. resize ourself to fit the incoming data.
	void set(float * source, unsigned int nFrames, unsigned int nChannels);

	/// return the underlying buffer. careful!
	vector<float> & getBuffer();

	friend class ofBaseSoundStream;
protected:

	// checks that size() and number of channels are consistent, logs a warning if not. returns consistency check result.
	bool checkSizeAndChannelsConsistency( string function="" );

	vector<float> buffer;
	int channels;
	int samplerate;

	long unsigned long tickCount;
	int soundStreamDeviceID;
};