#include "soundOrgApp.h"
#include "ofSoundFile.h"

//--------------------------------------------------------------
void soundOrgApp::setup() {
    ofSetVerticalSync(true);
    ofEnableAlphaBlending();

    fileNames.push_back("LZ.wav");
    fileNames.push_back("VU.wav");

    numOfSongs = fileNames.size();
    appHeight = ofGetHeight();
    appWidth  = ofGetWidth();

    whichToDraw = 0;

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
        mSoundBuffer.clear();
        mSoundFile.close();

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
        cout << "--max val: " << maxVal << endl;
		
        // don't need it anymore, delete!
        delete[] ft;
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
        cout << "--scaled samples in the arrays" << endl;

        // order the samples
        mySongs.at(n)->orderSamples();
        cout << "--ordered the samples" << endl << endl;
		
        // set which frames to show (initially show all frames)
        drawBoundaries.push_back(ofVec2f(0,numFrames));

        // draw an FBO with the graphs
        myVizs.push_back(ofFbo());
        myVizs.at(n).allocate(appWidth, appHeight);
        myVizs.at(n).begin();
        ofBackground(255);
        ofSetColor(0);
        drawSamples(mySongs.at(n), drawBoundaries.at(n));
        myVizs.at(n).end();
        saveFBO(myVizs.at(n), ofToString(fileNames.at(n)).append(".png"));

        // cleanup
        delete mySongs.at(n);
    }
}

void soundOrgApp::saveFBO(ofFbo &fbo, string &filename){
    ofImage f;
    ofPixels *p = new ofPixels[appWidth*appHeight];
    fbo.readToPixels(*p);
    f.setFromPixels(*p);
    f.saveImage(filename);
    delete[] p;
}

//--------------------------------------------------------------
void soundOrgApp::update() {}

//--------------------------------------------------------------
void soundOrgApp::draw() {
    ofBackground(255);
    ofSetColor(255);
    myVizs.at(whichToDraw).draw(0,0);
}

// from and to are in frames...
void soundOrgApp::drawSamples(Song *song, ofVec2f bounds) {
    int from = bounds.x;
    int to = bounds.y;
    int numFrames = (to-from);
	
    // if more frames than pixels... grab every other numFrames/width-th frame
    if(numFrames >= appWidth) {
        unsigned int framesPerPixel = numFrames/appWidth;
        for(int i=0; i<appWidth; i++) {
            unsigned int mIndex = framesPerPixel*i+from;

            float maxVal = 0, minVal = 0;
            for(int j=0; j<framesPerPixel; j++){
                unsigned int mJndex = mIndex+j;
                float thisVal = song->getOldSample(mJndex);
                if(thisVal > maxVal){maxVal = thisVal;}
                if(thisVal < minVal){minVal = thisVal;}
            }

            float maxY0 = (maxVal/float(MAX_SAMPLE_VAL)) *appHeight*0.25;
            float minY0 = (minVal/float(MAX_SAMPLE_VAL)) *appHeight*0.25;
            float y1 = (float(song->getNewSample(mIndex))/float(MAX_SAMPLE_VAL)) *appHeight*0.25;

            ofLine(i,appHeight*0.25,i,appHeight*0.25-minY0);
            ofLine(i,appHeight*0.25,i,appHeight*0.25-maxY0);
            ofLine(i,appHeight*0.75,i,appHeight*0.75-y1);
        }
    }
	
    // if more pixels than frames... draw every other width/frames-th pixel
    else {
        for(int i=0; i<numFrames; i++) {
            float f0 = float(appWidth)/float(numFrames);
            float x0 = f0*i;

            float y0 = (float(mySongs.at(whichToDraw)->getOldSample(int(x0+from)))/float(MAX_SAMPLE_VAL)) *appHeight*0.25;
            float y1 = (float(mySongs.at(whichToDraw)->getNewSample(int(x0+from)))/float(MAX_SAMPLE_VAL)) *appHeight*0.25;

            ofRect(x0,appHeight*0.25,f0*2/3,-y0);
            ofRect(x0,appHeight*0.75,f0*2/3,-y1);
        }
    }
}

void soundOrgApp::mousePressed(int x, int y, int button) {
    whichToDraw = (whichToDraw+1)%numOfSongs;
}

//--------------------------------------------------------------
void soundOrgApp::keyPressed(int key) {}
void soundOrgApp::keyReleased(int key) {}
void soundOrgApp::mouseMoved(int x, int y ) {}
void soundOrgApp::mouseDragged(int x, int y, int button) {}
void soundOrgApp::windowResized(int w, int h) {}
void soundOrgApp::mouseReleased(int x, int y, int button) {}

