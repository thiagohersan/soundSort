/*
 * libaudiodecoder - Native Portable Audio Decoder Library
 * libaudiodecoder API Header File
 * Latest version available at: http://www.oscillicious.com/libaudiodecoder
 *
 * Copyright (c) 2010-2012 Albert Santoni, Bill Good, RJ Ryan  
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * The text above constitutes the entire libaudiodecoder license; however, 
 * the Oscillicious community also makes the following non-binding requests:
 *
 * Any person wishing to distribute modifications to the Software is
 * requested to send the modifications to the original developer so that
 * they can be incorporated into the canonical version. It is also 
 * requested that these non-binding requests be included along with the 
 * license above.
 */

#include <string>
#include <iostream>
#include "audiodecodercoreaudio.h"
#include <TargetConditionals.h>

#if TARGET_OS_IPHONE
typedef short READ_SAMPLE;
#else
typedef float READ_SAMPLE;
#endif


std::string getErrorString( OSStatus errorCode ) {
	std::string errorString = "no error";
	if(noErr != errorCode)
	{
		
		switch(errorCode)
		{
	#ifdef __IPHONE_3_1
	#if __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_3_1
			case kExtAudioFileError_CodecUnavailableInputConsumed:
				errorString = "Write function interrupted - last buffer written";
				break;
			case kExtAudioFileError_CodecUnavailableInputNotConsumed:
				errorString = "Write function interrupted - last buffer not written";
				break;
	#endif /* __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_3_1 */
	#endif /* __IPHONE_3_1 */
			case kExtAudioFileError_InvalidProperty:
				errorString = "Invalid property";
				break;
			case kExtAudioFileError_InvalidPropertySize:
				errorString = "Invalid property size";
				break;
			case kExtAudioFileError_NonPCMClientFormat:
				errorString = "Non-PCM client format";
				break;
			case kExtAudioFileError_InvalidChannelMap:
				errorString = "Wrong number of channels for format";
				break;
			case kExtAudioFileError_InvalidOperationOrder:
				errorString = "Invalid operation order";
				break;
			case kExtAudioFileError_InvalidDataFormat:
				errorString = "Invalid data format";
				break;
			case kExtAudioFileError_MaxPacketSizeUnknown:
				errorString = "Max packet size unknown";
				break;
			case kExtAudioFileError_InvalidSeek:
				errorString = "Seek offset out of bounds";
				break;
			case kExtAudioFileError_AsyncWriteTooLarge:
				errorString = "Async write too large";
				break;
			case kExtAudioFileError_AsyncWriteBufferOverflow:
				errorString = "Async write could not be completed in time";
				break;
			case kAudioConverterErr_InvalidOutputSize:
				errorString = "Invalid output size";
				break;
			default:
				errorString = "Unknown ext audio error";
		}
		
	}
	
	return errorString;
}


AudioDecoderCoreAudio::AudioDecoderCoreAudio(const std::string filename)
: AudioDecoderBase(filename)
, m_headerFrames(0)
{
    m_filename = filename;
}

AudioDecoderCoreAudio::~AudioDecoderCoreAudio() {
	ExtAudioFileDispose(m_audioFile);

}

int AudioDecoderCoreAudio::open() {
//    std::cerr << "AudioDecoderCoreAudio::open()" << std::endl;

    //Open the audio file.
    OSStatus err;

    /** This code blocks works with OS X 10.5+ only. DO NOT DELETE IT for now. */
    /*CFStringRef urlStr = CFStringCreateWithCharacters(0,
   				reinterpret_cast<const UniChar *>(
                //qurlStr.unicode()), qurlStr.size());
                m_filename.data()), m_filename.size());
                */
    CFStringRef urlStr = CFStringCreateWithCString(kCFAllocatorDefault, 
                                                   m_filename.c_str(),
                                                   CFStringGetSystemEncoding());

    CFURLRef urlRef = CFURLCreateWithFileSystemPath(NULL, urlStr, kCFURLPOSIXPathStyle, false);
    err = ExtAudioFileOpenURL(urlRef, &m_audioFile);
    CFRelease(urlStr);
    CFRelease(urlRef);

    /** TODO: Use FSRef for compatibility with 10.4 Tiger. 
        Note that ExtAudioFileOpen() is deprecated above Tiger, so we must maintain
        both code paths if someone finishes this part of the code.
    FSRef fsRef;
    CFURLGetFSRef(reinterpret_cast<CFURLRef>(url.get()), &fsRef);
    err = ExtAudioFileOpen(&fsRef, &m_audioFile);
    */

	if (err != noErr)
	{
        std::cerr << "AudioDecoderCoreAudio: Error opening file." << std::endl;
		return AUDIODECODER_ERROR;
	}

    // get the input file format
    CAStreamBasicDescription inputFormat;
    UInt32 size = sizeof(inputFormat);
    err = ExtAudioFileGetProperty(m_audioFile, kExtAudioFileProperty_FileDataFormat, &size, &inputFormat);
	if (err != noErr)
	{
        std::cerr << "AudioDecoderCoreAudio: Error getting file format." << std::endl;
		return AUDIODECODER_ERROR;
	}    
    m_inputFormat = inputFormat;
    
	// create the output format
	CAStreamBasicDescription outputFormat;
/*	FillOutASBDForLPCM( outputFormat,
					   inputFormat.mSampleRate,
					   inputFormat.mChannelsPerFrame,
					   16, // bits per channel
					   16, // bits per channel
					   false, // isFloat
					   false // isBigEndian
					   );*/
    bzero(&outputFormat, sizeof(AudioStreamBasicDescription));
	outputFormat.mFormatID = kAudioFormatLinearPCM;
	outputFormat.mSampleRate = inputFormat.mSampleRate;
	outputFormat.mChannelsPerFrame = 2;
    outputFormat.mFormatFlags = kAudioFormatFlagsCanonical;
    //kAudioFormatFlagsCanonical means Native endian, float, packed on Mac OS X,
    //but signed int for iOS instead.
    //Note iPhone/iOS only supports signed integers supposedly:
    //outputFormat.mFormatFlags = kAudioFormatFlagIsSignedInteger;
	
    //Debugging:
    //printf ("Source File format: "); inputFormat.Print();
    //printf ("Dest File format: "); outputFormat.Print();


	/*
	switch(inputFormat.mBitsPerChannel) {
		case 16:
			outputFormat.mFormatFlags =  kAppleLosslessFormatFlag_16BitSourceData;
			break;
		case 20:
			outputFormat.mFormatFlags =  kAppleLosslessFormatFlag_20BitSourceData;
			break;
		case 24:
			outputFormat.mFormatFlags =  kAppleLosslessFormatFlag_24BitSourceData;
			break;
		case 32:
			outputFormat.mFormatFlags =  kAppleLosslessFormatFlag_32BitSourceData;
			break;
	}*/

    // get and set the client format - it should be lpcm
    CAStreamBasicDescription clientFormat = outputFormat; //We're always telling the OS to do the conversion to floats for us now
	clientFormat.mChannelsPerFrame = 2;
	clientFormat.mBytesPerFrame = sizeof(READ_SAMPLE)*clientFormat.mChannelsPerFrame;
	clientFormat.mBitsPerChannel = sizeof(READ_SAMPLE)*8; //16 for signed int, 32 for float;
	clientFormat.mFramesPerPacket = 1;
	clientFormat.mBytesPerPacket = clientFormat.mBytesPerFrame*clientFormat.mFramesPerPacket;
	clientFormat.mReserved = 0;
	m_clientFormat = clientFormat;
    size = sizeof(clientFormat);
    
    err = ExtAudioFileSetProperty(m_audioFile, kExtAudioFileProperty_ClientDataFormat, size, &clientFormat);
	if (err != noErr)
	{
		//qDebug() << "SSCA: Error setting file property";
        std::cerr << "AudioDecoderCoreAudio: Error setting file property." << std::endl;
		return AUDIODECODER_ERROR;
	}
	
	//Set m_iChannels and m_iNumSamples;
	m_iChannels = clientFormat.NumberChannels();

	//get the total length in frames of the audio file - copypasta: http://discussions.apple.com/thread.jspa?threadID=2364583&tstart=47
	UInt32		dataSize;
	SInt64		totalFrameCount;		
	dataSize	= sizeof(totalFrameCount); //XXX: This looks sketchy to me - Albert
	err			= ExtAudioFileGetProperty(m_audioFile, kExtAudioFileProperty_FileLengthFrames, &dataSize, &totalFrameCount);
	if (err != noErr)
	{
        std::cerr << "AudioDecoderCoreAudio: Error getting number of frames." << std::endl;
		return AUDIODECODER_ERROR;
	}

      //
      // WORKAROUND for bug in ExtFileAudio
      //
      
      AudioConverterRef acRef;
      UInt32 acrsize=sizeof(AudioConverterRef);
      err = ExtAudioFileGetProperty(m_audioFile, kExtAudioFileProperty_AudioConverter, &acrsize, &acRef);
      //_ThrowExceptionIfErr(@"kExtAudioFileProperty_AudioConverter", err);

      AudioConverterPrimeInfo primeInfo;
      UInt32 piSize=sizeof(AudioConverterPrimeInfo);
      memset(&primeInfo, 0, piSize);
      err = AudioConverterGetProperty(acRef, kAudioConverterPrimeInfo, &piSize, &primeInfo);
      if(err != kAudioConverterErr_PropertyNotSupported) // Only if decompressing
      {
         //_ThrowExceptionIfErr(@"kAudioConverterPrimeInfo", err);
         
         m_headerFrames=primeInfo.leadingFrames;
      }
	
	m_iNumSamples = (totalFrameCount/*-m_headerFrames*/)*m_iChannels;
	m_iSampleRate = inputFormat.mSampleRate;
	m_fDuration = m_iNumSamples / static_cast<float>(m_iSampleRate * m_iChannels);
	
    //Convert mono files into stereo
    if (inputFormat.NumberChannels() == 1)
    {
        SInt32 channelMap[2] = {0, 0}; // array size should match the number of output channels
        AudioConverterSetProperty(acRef, kAudioConverterChannelMap, 
                                    sizeof(channelMap), channelMap);
    }

	//Seek to position 0, which forces us to skip over all the header frames.
	//This makes sure we're ready to just let the Analyser rip and it'll
	//get the number of samples it expects (ie. no header frames).
	seek(0);

    return AUDIODECODER_OK;
}

int AudioDecoderCoreAudio::seek(int sampleIdx) {
    OSStatus err = noErr;
    SInt64 segmentStart = sampleIdx / 2;

    err = ExtAudioFileSeek(m_audioFile, (SInt64)segmentStart+m_headerFrames);
    //_ThrowExceptionIfErr(@"ExtAudioFileSeek", err);
	//qDebug() << "SSCA: Seeking to" << segmentStart;

	//err = ExtAudioFileSeek(m_audioFile, sampleIdx / 2);		
	if (err != noErr)
	{
        std::cerr << "AudioDecoderCoreAudio: Error " << err << " seeking to sample " << sampleIdx << std::endl;
	}

    m_iPositionInSamples = sampleIdx;

    return AUDIODECODER_OK;
}
/*
int AudioDecoderCoreAudio::readShort(int size, const short *destination) {
	OSStatus err;
    short *destBuffer(const_cast<short*>(destination));
    unsigned int samplesWritten = 0;
    unsigned int i = 0;
    UInt32 numFrames = 0;
    unsigned int totalFramesToRead = size/m_clientFormat.NumberChannels();
    unsigned int numFramesRead = 0;
    unsigned int numFramesToRead = totalFramesToRead;

	
    while (numFramesRead < totalFramesToRead) {
    	numFramesToRead = totalFramesToRead - numFramesRead;
    	
		AudioBufferList fillBufList;
		fillBufList.mNumberBuffers = 1; //Decode a single track
        //See CoreAudioTypes.h for definitins of these variables:
		fillBufList.mBuffers[0].mNumberChannels = m_clientFormat.NumberChannels();
		fillBufList.mBuffers[0].mDataByteSize = numFramesToRead*m_clientFormat.NumberChannels() * sizeof(short int);
		fillBufList.mBuffers[0].mData = (void*)(&destBuffer[numFramesRead*2]);
		std::cout << "frames to read: " << numFramesToRead << "/" << totalFramesToRead << " size "<< fillBufList.mBuffers[0].mDataByteSize << " (" << numFramesToRead*m_clientFormat.NumberChannels()<<" samples)" << std::endl;
		
        // client format is always linear PCM - so here we determine how many frames of lpcm
        // we can read/write given our buffer size
		numFrames = numFramesToRead; //This silly variable acts as both a parameter and return value.
		err = ExtAudioFileRead (m_audioFile, &numFrames, &fillBufList);\
		std::cout << "actually read " <<numFrames <<std::endl;
		//The actual number of frames read also comes back in numFrames.
		//(It's both a parameter to a function and a return value. wat apple?)
		//XThrowIfError (err, "ExtAudioFileRead");
        if (err != noErr)
        {
            std::cerr << "Error '" << getErrorString(err) << "' reading samples from file" << std::endl;
            return 0;
        }
		
		if (!numFrames) {
			// this is our termination condition
			break;
		}
		numFramesRead += numFrames;
    }
    
    m_iPositionInSamples += numFramesRead*m_iChannels;
	
	std::cout << "read " << numFramesRead << " frames, " << m_iChannels << " channels";
	
    return numFramesRead*m_iChannels;
}*/


int AudioDecoderCoreAudio::read(int size, const SAMPLE *destination) {
    OSStatus err;

	READ_SAMPLE *destBuffer;
	
#if TARGET_OS_IPHONE
	// 16-bit on iphone
	destBuffer = (READ_SAMPLE*)malloc(sizeof(READ_SAMPLE)*size );
#else
	destBuffer = const_cast<READ_SAMPLE*>(destination);
#endif
		
    unsigned int samplesWritten = 0;
    unsigned int i = 0;
    UInt32 numFrames = 0;
    unsigned int totalFramesToRead = size/m_iChannels;
    unsigned int numFramesRead = 0;
    unsigned int numFramesToRead = totalFramesToRead;

/*	Boolean writeable;
	UInt32 byteSize;
	unsigned char data[128];
	ExtAudioFileGetPropertyInfo( m_audioFile, kExtAudioFileProperty_ClientDataFormat, &byteSize, &writeable );
	assert(byteSize<128);
	ExtAudioFileGetProperty( m_audioFile, kExtAudioFileProperty_ClientDataFormat, byteSize, (void*)data );*/
	
    while (numFramesRead < totalFramesToRead) { 
    	numFramesToRead = totalFramesToRead - numFramesRead;
    	
		AudioBufferList fillBufList;
		fillBufList.mNumberBuffers = 1; //Decode a single track
        //See CoreAudioTypes.h for definitins of these variables:
		fillBufList.mBuffers[0].mNumberChannels = m_clientFormat.NumberChannels();
		fillBufList.mBuffers[0].mDataByteSize = numFramesToRead*m_iChannels * sizeof(READ_SAMPLE);
		fillBufList.mBuffers[0].mData = (void*)(&destBuffer[numFramesRead*2]);
			
        // client format is always linear PCM - so here we determine how many frames of lpcm
        // we can read/write given our buffer size
		numFrames = numFramesToRead; //This silly variable acts as both a parameter and return value.
		err = ExtAudioFileRead (m_audioFile, &numFrames, &fillBufList);
		//The actual number of frames read also comes back in numFrames.
		//(It's both a parameter to a function and a return value. wat apple?)
		//XThrowIfError (err, "ExtAudioFileRead");	
        if (err != noErr)
        {
            std::cerr << "Error '" << getErrorString(err) << "' reading samples from file" << std::endl;
			
            return 0;
        }

		if (!numFrames) {
				// this is our termination condition
			break;
		}
		numFramesRead += numFrames;
    }
    
    m_iPositionInSamples += numFramesRead*m_iChannels;
	

#if TARGET_OS_IPHONE
	// convert to float 0..1
	for ( int i=0; i<size; i++ ){
		SAMPLE* dest = const_cast<SAMPLE*>(destination);
		dest[i] = destBuffer[i]/32768.0f;
	}
#endif

	

    return numFramesRead*m_iChannels;
}


// static
std::vector<std::string> AudioDecoderCoreAudio::supportedFileExtensions() {
    std::vector<std::string> list;
    list.push_back(std::string("m4a"));
    list.push_back(std::string("mp3"));
    list.push_back(std::string("mp2"));
	list.push_back(std::string("aiff"));
	list.push_back(std::string("aif"));
    list.push_back(std::string("caf"));
    list.push_back(std::string("sd2"));
    list.push_back(std::string("ac3"));
    list.push_back(std::string("3gp"));
    //Can add mp3, mp2, ac3, and others here if you want.
    //See:
    //  http://developer.apple.com/library/mac/documentation/MusicAudio/Reference/AudioFileConvertRef/Reference/reference.html#//apple_ref/doc/c_ref/AudioFileTypeID
    
    return list;
}


