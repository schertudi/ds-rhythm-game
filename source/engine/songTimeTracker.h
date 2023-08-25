#pragma once

#include "engineTypes.h"


class SongTimeTracker {
    private:
    int globalBeat;
    int subBeat;
    int bpm; //the rate at which globalBeat changes
    int subBpm; //this is n times faster than bpm and used for subbeat calculations
    int progress; //0-100 depending on how close to next beat we are
    int subProgress;
    int numSubBeats;
    int numBeatsInBar;

    public:
    void init(int b, int sub);

    int updateBeat(int frame);

    songPosition getSongPosition();

    private:

    int getGlobalBeat();

    int getLocalBeat(int _globalBeat);

    int getBar(int _globalBeat);

    int getProgress();
};


