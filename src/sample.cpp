#include "sample.h"

Sample::~Sample(){
}

Sample::Sample(){
    theSample = -1;
    oldIndex = -1;
    newIndex = -1;
}

Sample::Sample(short s){
    theSample = s;
    oldIndex = -1;
    newIndex = -1;
}

Sample::Sample(short s, int oI){
    theSample = s;
    oldIndex = oI;
    newIndex = -1;
}

Sample::Sample(short s, int oI, int nI){
    theSample = s;
    oldIndex = oI;
    newIndex = nI;
}

short Sample::getSample() const {
    return theSample;
}

int   Sample::getOldIndex(){
    return oldIndex;
}

int   Sample::getNewIndex(){
    return newIndex;
}

void  Sample::setNewIndex(int i){
    newIndex = i;
}

void  Sample::setValue(short v){
    theSample = v;
}
