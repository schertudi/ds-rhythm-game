#include "multiBeatStateTracker.h"



void MultiBeatStateTracker::init(int _startBeat, int _numBeats, int _beatGap) {
    startBeat = _startBeat;
    numBeats = _numBeats;
    beatGap = _beatGap;
    endBeat = startBeat + numBeats * beatGap;
    started = false;
    killed = false;
    finished = false;
    lastHitBeat = _startBeat - _beatGap;
    relativeBeat = findRelativeBeat(lastHitBeat);
}

bool MultiBeatStateTracker::update(int currentBeat, std::vector<playableBeatStatus> beatStates) { //return true if progressed
    if (reachedLastBeat(currentBeat)) {
        finished = true;
        return false;
    }
    if (killed) { return false; }

    if (currentBeat >= startBeat - 1) { 
        started = true; 
    } else {
        return false;
    }

    if (beatInSequenceMissed(beatStates)) {
        killed = true;
        lastHitBeat = -1;
        relativeBeat = -1;
        return false;
    }

    int nextHitBeat = nextBeatToHit(currentBeat);
    bool hit = certainBeatHit(currentBeat, nextHitBeat, beatStates);
    if (hit) {
        lastHitBeat = nextHitBeat;
        relativeBeat = findRelativeBeat(lastHitBeat);
        return true;
    } 
    return false; 
}

int MultiBeatStateTracker::findRelativeBeat(int beat) {
    //relative beat will be 0 if we are at start, and at n if we have completed sequence of n beats
    if (lastHitBeat < startBeat) { return -1; }
    int rel = beat;
    rel -= startBeat;
    rel /= beatGap;
    return rel;
}

//DUPLICATED CODE, TODO: FIX
int MultiBeatStateTracker::getBeatIndex(int beat, std::vector<playableBeatStatus> beatStates) {
    for (size_t i = 0; i < beatStates.size(); i++) {
        if (beatStates[i].beatStart == beat) {
            return i;
        }
    }
    return -1;
}

bool MultiBeatStateTracker::certainBeatHit(int beat, int checkBeat, std::vector<playableBeatStatus> beatStates) {
    int index = getBeatIndex(checkBeat, beatStates);
    if (index == -1) return false;

    playerStatus state = beatStates[index].playerState;
    if (beat == checkBeat - 1 || beat == checkBeat) { //if just before, exactly at, or just after window for beat
        if (state == CORRECT_HIT) {
            return true;
        }
    }

    return false;
}

bool MultiBeatStateTracker::reachedLastBeat(int currentBeat) {
    return currentBeat >= startBeat + numBeats * beatGap;
}

int MultiBeatStateTracker::nextBeatToHit(int currentBeat) {
    int next = lastHitBeat + beatGap;
    return next;
}



bool MultiBeatStateTracker::beatInSequenceMissed(std::vector<playableBeatStatus> beatStates) {
    //check ALL beats from start to end of sequence
    for (int checkBeat = startBeat; checkBeat <= endBeat; checkBeat += beatGap) {
        int index = getBeatIndex(checkBeat, beatStates);
        if (index == -1) { continue; }
        playerStatus state = beatStates[index].playerState;

        if (state == EARLY_HIT || state == MISS) {
            return true;
        }
    }

    return false;
}
