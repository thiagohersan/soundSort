#pragma once

#include "ofMain.h"
#include "song.h"

#define MAX_SAMPLE_VAL 32767
#define NUM_SAMPLE_VAL 65535

class testApp : public ofBaseApp{
public:
	vector<string> fileNames;
    unsigned int numOfSongs;

	short **sBuffer;
	Song **song;
	unsigned int **stats;
	unsigned int **binStats;
    unsigned int *numFrames;
    unsigned int *numChannels;

	
	int *maxStat;
	int *maxBinStat;
	
	int mpX;
	int cF0, cF1;
	
	int appHeight, appWidth;
	
	void drawSamples (int from, int to);
	void drawStats   (int from, int to);
	void drawStatsBin(int from, int to);
	bool needsDrawed;
	bool statsNeedsDrawed;
	
	unsigned int whichToDraw;
	
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
