#ifndef BEATMANAGER_H
#define BEATMANAGER_H

#include "rhythmPath.cpp"
#include "constants.h"



class BeatManager {
    private:
    int globalBeat;
    int subBeat;
    int bpm; //the rate at which globalBeat changes
    int subBpm; //this is n times faster than bpm and used for subbeat calculations
    int progress; //0-100 depending on how close to next beat we are
    int subProgress;
    int numSubBeats;

    /*
    there are three types of beats
    global beat: constantly increments
    bar beat: beat in relation to bar, ranging from 0 to beatsInBar
    sub beat: more granular version of main beat allowing for notes on half or quarter beats. ranges from 0 to numSubBeats * beatsInBar

    global beat should be used for keeping track of when things happen relative to start of song
    main beat should be used for metronome, guide for animations, reference for how long note sounds should be played for
    sub beat is used for keeping track of when notes should be played
    */

    public:
    BeatManager(int b, int sub) {
        globalBeat = -2;
        bpm = b;
        subBpm = b * sub;
        progress = 0;
        numSubBeats = sub;
        subBeat = 0;
        subProgress = 0;
    }

    int updateBeat(int frame) {
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

    songPosition getSongPosition() {
        songPosition pos = {globalBeat, getLocalBeat(), subBeat, getBar(), progress, subProgress, numSubBeats};
        return pos;
    }

    private:

    int getGlobalBeat() {
        return globalBeat;
    }

    int getLocalBeat() {
        return (globalBeat) % 4;
    }

    int getBar() {
        return (globalBeat) / 4;
    }

    int getProgress() {
        return progress;
    }
};


#endif