#pragma once

class Sample {
    short theSample;
    int oldIndex, newIndex;

    public:
        ~Sample(){}

        Sample(short s, int oI){
            theSample = s;
            oldIndex = oI;
            newIndex = -1;
        }
        short getSample() const {return theSample;}
        int getOldIndex() const{return oldIndex;}
        int getNewIndex() const{return newIndex;}
        void setNewIndex(int i){newIndex = i;}
        void setValue(short v){theSample = v;}
};
