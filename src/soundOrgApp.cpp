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

    short *maxVal = new short[numOfSongs];
    int *maxAt    = new int[numOfSongs];
    int *maxBinAt = new int[numOfSongs];

    stats      = new unsigned int*[numOfSongs];
    binStats   = new unsigned int*[numOfSongs];
    numFrames  = new unsigned int[numOfSongs];
    numChannels= new unsigned int[numOfSongs];
    sBuffer    = new short*[numOfSongs];
    song       = new Song*[numOfSongs];
    maxStat    = new int[numOfSongs];
    maxBinStat = new int[numOfSongs];
    drawBoundary = new ofVec2f[numOfSongs];

    for(int n=0; n<numOfSongs; n++) {
        maxVal[n] = 0;
        maxAt[n] = -1;
        maxBinAt[n] = -1;
		
        maxStat[n]    = 0;
        maxBinStat[n] = 0;
		
        stats[n] = new unsigned int[NUM_SAMPLE_VAL+1];
        binStats[n] = new unsigned int[appWidth+1];

        // clear stats arrays
        for(int i=0; i<(NUM_SAMPLE_VAL+1); i++) {
            stats[n][i] = 0;
        }
        for(int i=0; i<(appWidth+1); i++) {
            binStats[n][i] = 0;
        }

        // open file, read samples into buffer
        ofSoundFile mSoundFile;
        ofSoundBuffer mSoundBuffer;

        mSoundFile.loadSound(fileNames.at(n));
        mSoundFile.readTo(mSoundBuffer);
        numChannels[n] = mSoundFile.getNumChannels();
        numFrames[n] = mSoundFile.getNumSamples()/mSoundFile.getNumChannels(); // ??

        cout << "file has " << numFrames[n] << " frames and " << numChannels[n] << " channels" << endl;

        // read into short buffer
        float *ft = new float[numChannels[n]*numFrames[n]];
        mSoundBuffer.copyTo(ft, numFrames[n], numChannels[n], 0);
        sBuffer[n] = new short[numChannels[n]*numFrames[n]];
        for(unsigned int i=0; i<numChannels[n]*numFrames[n]; i++){
            sBuffer[n][i] = (short) ofMap(ft[i], -1, 1, -MAX_SAMPLE_VAL, MAX_SAMPLE_VAL, true);
        }

        song[n] = new Song(numFrames[n]);

        // read audio values into data structs
        for(unsigned int i=0; i<numFrames[n]; i++) {
            // make mono
            int mv = (sBuffer[n][i+0] + sBuffer[n][i+1])/2;
            song[n]->initSampleAt(i,mv);
			
            // find largest sample value
            if (abs(mv) > maxVal[n]) {
                maxVal[n] = abs(mv);
            }
        }
		
        printf("max val[%d] : %d\n", n, maxVal[n]);
		
        // don't need it anymore, delete!
        delete[] sBuffer[n];
		
        // scale values, put them in the arrays
        // then update counts
        int binFact = int((NUM_SAMPLE_VAL+1)/appWidth);
        for(int i=0; i<numFrames[n]; i++) {
            // scale the fucker
            float t = (float)(song[n]->getOldSample(i));
            t /= (float)(maxVal[n]);
            t *= (float)(MAX_SAMPLE_VAL);
			
            // put back in the arrays
            song[n]->setOldSample(i,(short)t);
            song[n]->setNewSample(i,(short)t);
			
            // update stats
            // shift indexes due to negative numbers (-32768 -> index0)
            int sI = int(short(t)+MAX_SAMPLE_VAL+1);
            int bsI = sI/binFact;
            stats[n][sI] += 1;
            binStats[n][bsI] += 1;
        }
        printf("scaled samples in the arrays\n");
		
        // ignoring some values because they happen too often
        stats[n][MAX_SAMPLE_VAL+1] = stats[n][MAX_SAMPLE_VAL+2];
        stats[n][MAX_SAMPLE_VAL] = stats[n][MAX_SAMPLE_VAL-1];

        // get maxstat
        for(int i=0; i<(NUM_SAMPLE_VAL+1); i++) {
            int t = stats[n][i];
			
            if(t>maxStat[n]) {
                maxStat[n] = t;
                maxAt[n] = i;
            }
        }
        printf("found maxStat[%d] at: %d\n", n, maxAt[n]);
		
        // get maxbinstat
        for(int i=0; i<(appWidth+1); i++) {
            int t = binStats[n][i];
			
            if(t>maxBinStat[n]) {
                maxBinStat[n] = t;
                maxBinAt[n] = i;
            }
        }
        printf("found maxBinStat[%d] at: %d\n", n,maxBinAt[n]);
		
        // order the samples
        song[n]->orderNew();
        printf("ordered the samples\n");
		
        // set which frames to show (initially show all frames)
        drawBoundary[n] = ofVec2f(0,numFrames[n]);
    }
	
    whichToDraw = 0;
    statsNeedsDrawed = 1;
    needsDrawed = 1;
}

//--------------------------------------------------------------
void soundOrgApp::update() {}

//--------------------------------------------------------------
void soundOrgApp::draw() {
    if(needsDrawed == 1) {
        ofBackground(220,220,220);
        if(statsNeedsDrawed == 1) {
            //drawStatsBin(drawBoundary[whichToDraw]);
            drawStats(drawBoundary[whichToDraw]);
            //drawSamples(drawBoundary[whichToDraw]);
            statsNeedsDrawed = 0;
        }
        else {
            drawSamples(drawBoundary[whichToDraw]);
        }
        needsDrawed = 0;
    }
}

// helper function for setting line color based on some maths
void setColor(float y, float maxY, int c) {
    y = fabs(y);
    unsigned short rgb[] = {0,0,0};
    rgb[(c+2)%3] = (1-(y/maxY))*100 + 127;
    rgb[(c+1)%3] = (1-(y/maxY))*100 + 40;
    rgb[(c+0)%3] = (1-(y/maxY))*100 + 0;
    int a = (y/maxY)*200+55;
    a = 255;
    ofSetColor(rgb[0],rgb[1],rgb[2],a);
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
			 */

            float y0 = (float(song[whichToDraw]->getOldSample(numFrames/appWidth*i+from))/float(MAX_SAMPLE_VAL)) * float(appHeight)*0.25;
            float y1 = (float(song[whichToDraw]->getNewSample(numFrames/appWidth*i+from))/float(MAX_SAMPLE_VAL)) * float(appHeight)*0.25;
			
            //int cc0 = int((fabs(y0)/(ofGetHeight()*0.25)) * (155.0) + 100.0);
            //int cc1 = int((fabs(y1)/(ofGetHeight()*0.25)) * (155.0) + 100.0);
			
            setColor(y0, (appWidth*0.25), whichToDraw%3);
            // minus y0 and minus y1 because OF has (0,0) on top left corner
            ofLine(i,appHeight*0.25,i,appHeight*0.25-y0);
            setColor(y1, (appWidth*0.25), whichToDraw%3);
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
			
            float y0 = (float(song[whichToDraw]->getOldSample(int(x0+from)))/float(32767)) * appHeight*0.25;
            float y1 = (float(song[whichToDraw]->getNewSample(int(x0+from)))/float(32767)) * appHeight*0.25;
			
            setColor(y0,(appHeight*0.25), whichToDraw%3);
            ofRect(x0,appHeight*0.25,f0*2/3,-y0);
            setColor(y1,(appHeight*0.25), whichToDraw%3);
            ofRect(x0,appHeight*0.75,f0*2/3,-y1);
        }
    }
}

////////////////////////////////////////////////
// from and to are in frames... or sample locations on screen
void soundOrgApp::drawStats(ofVec2f bounds) {
    int from = bounds.x;
    int to = bounds.y;
    int numFrames = (to-from);

    // if more frames than pixels... grab every other numFrames/width-th frame
    if(numFrames >= appWidth) {
        for(int i=0; i<appWidth; i++) {

            float y0 = (float(stats[whichToDraw][numFrames/appWidth*i+from])/float(maxStat[whichToDraw])) * float(appHeight)*0.5;
            //float y1 = (float(song0->getNewSample(numFrames/ofGetWidth()*i+from))/float(MAX_SAMPLE_VAL)) * float(ofGetHeight())*0.25;
			
            // minus y0 and minus y1 because OF has (0,0) on top left corner
            setColor(y0, appHeight*0.5, whichToDraw%3);
            ofLine(i,appHeight*0.5,i,appHeight*0.5-y0);
            //ofSetColor(100,100,255);
            //ofLine(i,ofGetHeight()*0.75,i,ofGetHeight()*0.75-y1);
        }
    }
	
    // if more pixels than frames... draw every other width/frames-th pixel
    else {
        for(int i=0; i<numFrames; i++) {
            float f0 = float(appWidth)/float(numFrames);
            float x0 = f0*i;
			
            float y0 = (float(stats[whichToDraw][int(x0+from)])/float(maxStat[whichToDraw])) * float(appHeight)*0.5;
            //float y1 = (float(song0->getNewSample(int(x0+from)))/float(32767)) * ofGetHeight()*0.25;
			
            setColor(y0, appHeight*0.5, whichToDraw%3);
            ofRect(x0,appHeight*0.5,f0*2/3,-y0);
            //ofSetColor(100,100,255);
            //ofRect(x0,ofGetHeight()*0.75,f0*2/3,-y1);
        }
    }
	
    // always the same graph...
    for(int i=0; i<appWidth; i++) {
        //float y0 = (float(stats[numFrames/ofGetWidth()*i+from])/float(maxStat)) * float(ofGetHeight())*0.5;
        float y1 = (float(binStats[whichToDraw][i])/float(maxBinStat[whichToDraw])) * float(appHeight)*0.5;
		
        // minus y0 and minus y1 because OF has (0,0) on top left corner
        //ofSetColor(255,100,100);
        //ofLine(i,ofGetHeight()*0.5,i,ofGetHeight()*0.5-y0);
        setColor(y1, appHeight*0.5, whichToDraw%3);
        ofLine(i,appHeight,i,float(appHeight)-y1);
    }
	
}

// only draw the binned version
void soundOrgApp::drawStatsBin(ofVec2f bounds) {
    int from = bounds.x;
    int to = bounds.y;

    //for(int n=0; n<numOfSongs; n++) {
	// always the same graph...
	for(int i=0; i<appWidth; i++) {
		float y1 = (float(binStats[whichToDraw][i])/float(maxBinStat[whichToDraw])) * float(appHeight)/numOfSongs;
		
		// minus y0 because OF has (0,0) on top left corner
		setColor(y1, float(appHeight)/numOfSongs, whichToDraw%3);
		int n = 0;
		ofLine(i,appHeight-n*float(appHeight)/numOfSongs,i,(appHeight-n*float(appHeight)/numOfSongs)-y1);
	}
    //}
}


//--------------------------------------------------------------
void soundOrgApp::keyPressed(int key) {}
void soundOrgApp::keyReleased(int key) {}
void soundOrgApp::mouseMoved(int x, int y ) {}
void soundOrgApp::mouseDragged(int x, int y, int button) {}
void soundOrgApp::windowResized(int w, int h) {}
void soundOrgApp::mousePressed(int x, int y, int button) {}
void soundOrgApp::mouseReleased(int x, int y, int button) {}

