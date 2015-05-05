#include "ofMain.h"
#include "soundSort.h"
#include "ofAppGlutWindow.h"

//========================================================================
int main( ){
    ofAppGlutWindow window;
	ofSetupOpenGL(&window, 4096/4,2160/4, OF_WINDOW);
	ofRunApp( new soundSort());
}
