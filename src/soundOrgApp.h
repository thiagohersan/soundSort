#pragma once

#include "ofMain.h"
#include "Song.h"

#define MAX_SAMPLE_VAL 32767
#define NUM_SAMPLE_VAL 65535

class soundOrgApp : public ofBaseApp{
public:
	vector<string> fileNames;
    unsigned int numOfSongs;
	int appHeight, appWidth;

	short **sBuffer;
	Song **song;
    unsigned int *numFrames;
    unsigned int *numChannels;

    ofVec2f *drawBoundary;
    bool needsDrawed;
	unsigned int whichToDraw;
    void drawSamples(ofVec2f bounds);

	void setup();
	void update();
	void draw();

	void keyPressed  (int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);
	
};
