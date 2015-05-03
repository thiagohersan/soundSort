#pragma once

class Sample {

    short theSample;
    int oldIndex, newIndex;

    public:
        Sample();
        Sample(short s);
        Sample(short s, int oI);
        Sample(short s, int oI, int nI);
        ~Sample();

        short getSample() const;
        int   getOldIndex();
        int   getNewIndex();
        void  setNewIndex(int i);
        void  setValue(short v);

};
