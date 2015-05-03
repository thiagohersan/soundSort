#include "soundOrgApp.h"
#include "ofSoundFile.h"

//--------------------------------------------------------------
void soundOrgApp::setup() {
    ofSetFrameRate(30);
    ofSetBackgroundAuto(false);
    ofEnableAlphaBlending();

    fileNames.push_back("LZ.wav");
    fileNames.push_back("VU.wav");

    numOfSongs = fileNames.size();
    appHeight = ofGetHeight();
    appWidth  = ofGetWidth();

    drawBoundary = new ofVec2f[numOfSongs];

    for(int n=0; n<numOfSongs; n++) {
        unsigned int numChannels, numFrames;
        short* tempBuffer;
        short maxVal = 0;

        // open file, read samples into buffer
        ofSoundFile mSoundFile;
        ofSoundBuffer mSoundBuffer;

        mSoundFile.loadSound(fileNames.at(n));
        mSoundFile.readTo(mSoundBuffer);
        numChannels = mSoundFile.getNumChannels();
        numFrames = mSoundFile.getNumSamples()/mSoundFile.getNumChannels(); // ??

        cout << "file has " << numFrames << " frames and " << numChannels << " channels" << endl;

        // read into short buffer
        float *ft = new float[numChannels*numFrames];
        mSoundBuffer.copyTo(ft, numFrames, numChannels, 0);
        tempBuffer = new short[numChannels*numFrames];
        for(unsigned int i=0; i<numChannels*numFrames; i++){
            tempBuffer[i] = (short) ofMap(ft[i], -1, 1, -MAX_SAMPLE_VAL, MAX_SAMPLE_VAL, true);
        }

        mySongs.push_back(new Song(numFrames, numChannels));

        // read audio values into data structs
        for(unsigned int i=0; i<numFrames; i++) {
            // make mono
            int mv = (tempBuffer[i+0] + tempBuffer[i+1])/2;
            mySongs.at(n)->initSampleAt(i,mv);
			
            // find largest sample value
            if (abs(mv) > maxVal) {
                maxVal = abs(mv);
            }
        }
        printf("max val[%d] : %d\n", n, maxVal);
		
        // don't need it anymore, delete!
        delete[] tempBuffer;
		
        // scale values, put them in the arrays
        for(int i=0; i<numFrames; i++) {
            // scale the fucker
            float t = (float)(mySongs.at(n)->getOldSample(i));
            t /= (float)(maxVal);
            t *= (float)(MAX_SAMPLE_VAL);
			
            // put back in the arrays
            mySongs.at(n)->setOldSample(i,(short)t);
            mySongs.at(n)->setNewSample(i,(short)t);
        }
        printf("scaled samples in the arrays\n");

        // order the samples
        mySongs.at(n)->orderSamples();
        printf("ordered the samples\n");
		
        // set which frames to show (initially show all frames)
        drawBoundary[n] = ofVec2f(0,numFrames);
    }

    whichToDraw = 0;
    needsDrawed = 1;
}

//--------------------------------------------------------------
void soundOrgApp::update() {}

//--------------------------------------------------------------
void soundOrgApp::draw() {
    if(needsDrawed == 1) {
        ofBackground(220,220,220);
        drawSamples(drawBoundary[whichToDraw]);
        needsDrawed = 0;
    }
}

// from and to are in frames...
void soundOrgApp::drawSamples(ofVec2f bounds) {
    int from = bounds.x;
    int to = bounds.y;
    int numFrames = (to-from);
	
    // if more frames than pixels... grab every other numFrames/width-th frame
    if(numFrames >= appWidth) {
        for(int i=0; i<appWidth; i++) {
            /*
			 float y0 = (float(sBuffer[numFrames/ofGetWidth()*i+from+0])/float(32767)) * ofGetHeight()*0.25;
			 float y1 = (float(sBuffer[numFrames/ofGetWidth()*i+from+1])/float(32767)) * ofGetHeight()*0.25;
             int cc0 = int((fabs(y0)/(ofGetHeight()*0.25)) * (155.0) + 100.0);
             int cc1 = int((fabs(y1)/(ofGetHeight()*0.25)) * (155.0) + 100.0);
			 */

            float y0 = (float(mySongs.at(whichToDraw)->getOldSample(numFrames/appWidth*i+from))/float(MAX_SAMPLE_VAL)) * float(appHeight)*0.25;
            float y1 = (float(mySongs.at(whichToDraw)->getNewSample(numFrames/appWidth*i+from))/float(MAX_SAMPLE_VAL)) * float(appHeight)*0.25;

            ofSetColor(0);
            // minus y0 and minus y1 because OF has (0,0) on top left corner
            ofLine(i,appHeight*0.25,i,appHeight*0.25-y0);
            ofLine(i,appHeight*0.75,i,appHeight*0.75-y1);
        }
    }
	
    // if more pixels than frames... draw every other width/frames-th pixel
    else {
        for(int i=0; i<numFrames; i++) {
            float f0 = float(appWidth)/float(numFrames);
            float x0 = f0*i;

            /*
			 float y0 = (float(sBuffer[int(x0+from+0)])/float(32767)) * ofGetHeight()*0.25;
			 float y1 = (float(sBuffer[int(x0+from+1)])/float(32767)) * ofGetHeight()*0.25;
			 */

            float y0 = (float(mySongs.at(whichToDraw)->getOldSample(int(x0+from)))/float(32767)) * appHeight*0.25;
            float y1 = (float(mySongs.at(whichToDraw)->getNewSample(int(x0+from)))/float(32767)) * appHeight*0.25;

            ofSetColor(0);
            ofRect(x0,appHeight*0.25,f0*2/3,-y0);
            ofRect(x0,appHeight*0.75,f0*2/3,-y1);
        }
    }
}


//--------------------------------------------------------------
void soundOrgApp::keyPressed(int key) {}
void soundOrgApp::keyReleased(int key) {}
void soundOrgApp::mouseMoved(int x, int y ) {}
void soundOrgApp::mouseDragged(int x, int y, int button) {}
void soundOrgApp::windowResized(int w, int h) {}
void soundOrgApp::mousePressed(int x, int y, int button) {}
void soundOrgApp::mouseReleased(int x, int y, int button) {}

