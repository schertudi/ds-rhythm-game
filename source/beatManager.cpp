#include "rhythmPath.cpp"

struct songPosition {
    int globalBeat;
    int mainBeat;
    int subBeat;
    int bar;
    int mainBeatProgress; //used for animations
    int subBeatProgress; //used for calculating timings
};

class BeatManager {
    private:
    int beat;
    int bpm;
    int progress; //0-100 depending on how close to next beat we are
    int numSubBeats;

    /*
    there are three types of beats
    global beat: constantly increments
    main beat: beat in relation to bar, ranging from 0 to beatsInBar
    sub beat: more granular version of main beat allowing for notes on half or quarter beats. ranges from 0 to numSubBeats * beatsInBar

    global beat should be used for keeping track of when things happen relative to start of song
    main beat should be used for metronome, guide for animations, reference for how long note sounds should be played for
    sub beat is used for keeping track of when notes should be played
    */

    public:
    BeatManager(int b, int sub) {
        beat = -1;
        bpm = b * sub;
        progress = 0;
        numSubBeats = sub;
    }

    bool updateBeat(int frame) { //return true if switched to next beat
        //NOTE!! will desync if bpm * subBeats > 1000
        progress = ((frame * 10) % (60*60*10/bpm)) * 100 / (60*60*10/bpm);
        //if(frame % (60*60*10/bpm) == 0) {
        if (progress == 0) {
            beat++;
            return true;
        }
        return false;
    }

    int getGlobalBeat() {
        return beat;
    }

    int getLocalBeat() {
        return (beat) % (4 * numSubBeats) + 1;
    }

    int getBar() {
        return (beat) / (4 * numSubBeats);
    }

    int getProgress() {
        return progress;
    }
};

