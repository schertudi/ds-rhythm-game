#include "songTimeTracker.h"


void SongTimeTracker::init(int b, int sub) {
    globalBeat = -3;
    bpm = b;
    subBpm = b * sub;
    progress = 0;
    numSubBeats = sub;
    subBeat = 0;
    subProgress = 0;
    numBeatsInBar = 4;
}

int SongTimeTracker::updateBeat(int frame) {
    //return 0 if no beat changes
    //return 1 if sub beat change
    //return 2 if global beat change + sub beat
    //return 3 if glob change but not sub (should NEVER happen!)
    //appears that subbeat is not always accurate, why?
    /*
    at 60fps we update every 1/60 = 0.0166 seconds
    if we have song at 120 bpm with 4 sub beats, we need granularity of 120 * 4 = 480 bpm
    480 bpm means 480 beats in 60 seconds
    so beat changes every 60 / 480 = 0.125 seconds
    so we would expect ~10 frames before each subbeat update
    */
    //NOTE!! will desync if bpm * subBeats > 1000
    int oldProg = progress;
    progress = ((frame * 10) % (60*60*10/bpm)) * 100 / (60*60*10/bpm);
    subProgress = ((frame * 10) % (60*60*10/subBpm)) * 100 / (60*60*10/subBpm);
    //if(frame % (60*60*10/bpm) == 0) {

    bool newGlobalBeat = oldProg != progress && progress == 0;
    
    if (newGlobalBeat) { //check if diff to avoid precision-related bugs
        globalBeat++;
    }

    if (subProgress == 0) {
        subBeat ++;
        if (subBeat == numSubBeats) {
            subBeat = 0;
            if (!newGlobalBeat) return 3;
        }

        if (newGlobalBeat) return 2;
        return 1;
    }

    return 0;
}

songPosition SongTimeTracker::getSongPosition() {
    songPosition pos = {globalBeat, getLocalBeat(globalBeat), subBeat, getBar(globalBeat), progress, subProgress, numSubBeats, numBeatsInBar};
    return pos;
}

int SongTimeTracker::getGlobalBeat() {
    return globalBeat;
}

int SongTimeTracker::getLocalBeat(int _globalBeat) {
    return (_globalBeat) % numBeatsInBar;
}

int SongTimeTracker::getBar(int _globalBeat) {
    return (globalBeat) / numBeatsInBar;
}

int SongTimeTracker::getProgress() {
    return progress;
}



