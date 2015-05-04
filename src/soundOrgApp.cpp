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
        mySongs.push_back(new Song(fileNames.at(n)));
        mySongs.at(n)->orderSamples();
        cout << "--ordered the samples" << endl << endl;

        // draw an FBO with the graphs
        myVizs.push_back(ofFbo());
        myVizs.at(n).allocate(appWidth, appHeight);
        myVizs.at(n).begin();
        ofBackground(255);
        ofSetColor(0);
        drawSamples(mySongs.at(n), ofVec2f(0,mySongs.at(n)->getNumFrames()));
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

