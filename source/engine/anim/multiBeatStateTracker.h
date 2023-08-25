#pragma once

#include <vector>
#include <stddef.h>
#include "../engineTypes.h"

class MultiBeatStateTracker {
    public:
    int lastHitBeat;
    int relativeBeat;
    bool started;
    bool finished;
    bool killed;

    MultiBeatStateTracker() {

    }

    void init(int _startBeat, int _numBeats, int _beatGap);
 
    bool update(int currentBeat, std::vector<playableBeatStatus> beatStates);

    int findRelativeBeat(int beat);

    private:
    int startBeat;
    int numBeats;
    int beatGap;
    int endBeat;

    //DUPLICATED CODE, TODO: FIX
    int getBeatIndex(int beat, std::vector<playableBeatStatus> beatStates);

    bool certainBeatHit(int beat, int checkBeat, std::vector<playableBeatStatus> beatStates);

    bool reachedLastBeat(int currentBeat);

    int nextBeatToHit(int currentBeat);

    bool beatInSequenceMissed(std::vector<playableBeatStatus> beatStates);

};