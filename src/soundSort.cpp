#include "soundSort.h"

//--------------------------------------------------------------
void soundSort::setup() {
    ofSetVerticalSync(true);
    ofEnableAlphaBlending();

    fileNames.push_back("LZ.wav");
    fileNames.push_back("VU.wav");
    fileNames.push_back("MD.wav");

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
        drawSamples(mySongs.at(n), ofVec2f(0,mySongs.at(n)->getNumFrames()));
        myVizs.at(n).end();

        ofFbo bgnd = ofFbo();
        bgnd.allocate(appWidth, appHeight);
        bgnd.begin();
        ofBackground(255);
        ofSetColor(255);
        myVizs.at(n).draw(0,0);
        bgnd.end();

        string saveFboFileName = ofToString(mySongs.at(n)->getSongName());
        saveFboFileName = ofToString("___").append(saveFboFileName.append(".png"));
        saveFBO(bgnd, saveFboFileName);
    }

    // re-order pixels mashup
    reSampleSong(mySongs.at(2), mySongs.at(0));
    reSampleSong(mySongs.at(2), mySongs.at(1));
    mySongs.at(0)->saveToFile(ofToString("__").append(mySongs.at(0)->getSongName()).append(".wav"));
    mySongs.at(1)->saveToFile(ofToString("__").append(mySongs.at(1)->getSongName()).append(".wav"));

    for(int n=0; n<numOfSongs; n++) {
        // cleanup
        delete mySongs.at(n);
    }
}

// src remains unchanged
void soundSort::reSampleSong(Song *srcSong, Song *dstSong){
    for(unsigned int i=0; i<dstSong->getNumFrames(); i++){
        // destructive
        short srcVal = (i<srcSong->getNumFrames())?srcSong->getNewSample(i):0;
        dstSong->setNewSample(i, srcVal);
    }
}

void soundSort::saveFBO(ofFbo &fbo, string &filename){
    ofImage f;
    ofPixels *p = new ofPixels[appWidth*appHeight];
    fbo.readToPixels(*p);
    f.setFromPixels(*p);
    f.saveImage(filename);
    delete[] p;
}

//--------------------------------------------------------------
void soundSort::update() {}

//--------------------------------------------------------------
void soundSort::draw() {
    ofBackground(255);
    ofSetColor(255);
    myVizs.at(whichToDraw).draw(0,0);
}

// from and to are in frames...
void soundSort::drawSamples(Song *song, ofVec2f bounds) {
    int from = bounds.x;
    int to = bounds.y;
    int numFrames = (to-from);
	
    // if more frames than pixels... grab every other numFrames/width-th frame
    if(numFrames >= appWidth) {
        unsigned int framesPerPixel = numFrames/appWidth/2;
        for(int i=0; i<appWidth; i++) {
            unsigned int mIndex = 2*framesPerPixel*i+from;
            unsigned int mIndexMin = framesPerPixel*i+from;
            unsigned int mIndexMax = to-(framesPerPixel*(i+1));

            for(int j=0; j<framesPerPixel; j++){
                unsigned int mJndex = mIndex+2*j, mJndexMin = mIndexMin+j, mJndexMax = mIndexMax+j;

                // for oldOrder just draw two samples in a row
                float y0 = (float(song->getOldSample(mJndex))/float(MAX_SAMPLE_VAL)) *appHeight*0.25;
                float y1 = (float(song->getOldSample(mJndex+1))/float(MAX_SAMPLE_VAL)) *appHeight*0.25;
                ofSetColor(0,4);
                ofLine(i,appHeight*0.25,i,appHeight*0.25-y0);
                ofLine(i,appHeight*0.25,i,appHeight*0.25-y1);

                // for newOrder, draw a line between min and max, after folding the array
                float minY = (float(song->getNewSample(mJndexMin))/float(MAX_SAMPLE_VAL)) *appHeight*0.25;
                float maxY = (float(song->getNewSample(mJndexMax))/float(MAX_SAMPLE_VAL)) *appHeight*0.25;
                int alpha = ofMap(abs(minY)+abs(maxY), 0, appHeight*0.20, 0, 255, true);
                ofSetColor(0,alpha);
                ofLine(appWidth-i,appHeight*0.75-minY,appWidth-i,appHeight*0.75-maxY);
            }
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

void soundSort::mousePressed(int x, int y, int button) {
    whichToDraw = (whichToDraw+1)%numOfSongs;
}

//--------------------------------------------------------------
void soundSort::keyPressed(int key) {}
void soundSort::keyReleased(int key) {}
void soundSort::mouseMoved(int x, int y ) {}
void soundSort::mouseDragged(int x, int y, int button) {}
void soundSort::windowResized(int w, int h) {}
void soundSort::mouseReleased(int x, int y, int button) {}

