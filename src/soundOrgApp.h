#pragma once

#include "ofMain.h"
#include "Song.h"

#define MAX_SAMPLE_VAL 32767
#define NUM_SAMPLE_VAL 65535

class soundOrgApp : public ofBaseApp{
public:
    int appHeight, appWidth;
    vector<string> fileNames;
    unsigned int numOfSongs;

    vector<Song*> mySongs;
    vector<ofFbo> myVizs;

    vector<ofVec2f> drawBoundaries;
    unsigned int whichToDraw;
    void drawSamples(Song *song, ofVec2f bounds);

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
