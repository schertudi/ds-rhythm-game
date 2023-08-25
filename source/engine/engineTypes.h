#pragma once

#include <vector>
#include "../genericTypes.h"

typedef int beatTime_t;
typedef int globalBeatProgress_t;
typedef int subBeatProgress_t;

//possible TODO is to typedef some of these i'm getting a little sick of the confusion, also look at streamlining it a bit
struct songPosition {
    int globalBeat; //20 refs
    int localBeat; //barely used
    int subBeat; //7 refs, only used for more granular global beat really
    int bar;
    int globalBeatProgress; //used for animations......???? why is this not used lol
    int subBeatProgress; //used for calculating timings.... 5 refs
    int numSubBeats; //10 refs, can't really simplify meaningfully
    int numBeatsInBar; //3 refs, just used to calculating bar, not worth simplification
    int time; //unimplemented but should be
};

enum playerStatus { IDLE, EARLY_HIT, EARLY_LIFT, READY_TO_HIT, CORRECT_HIT, CORRECT_LIFT, SLIDER_HIT, SLIDER_EARLY_LIFT, SLIDER_END, MISS }; 

struct playableBeatStatus {
    int beatStart;
    int beatEnd;
    int timingProgress; //gives info about current timing for beat (eg if hitting it now would be too early/just on time/too late)
    playerStatus playerState; //what is player actually doing?
    playerStatus oldPlayerState; //what was player doing last frame?
    bool isHit; //pen is both down and in the right place
    bool isSlider;
    bool isActive;
    Vec2d startPos;
    Vec2d endPos;
};

enum class direction { NONE=0, TOP, BOTTOM, LEFT, RIGHT };

enum class powerupStates { IDLE, BEFORE, JUST_BEFORE, GET_COMBO, WIN, LOSE };

struct powerupInfo {
    powerupStates currState;
    int timeAtStateChange;
    int numBeatsHit;
    int numBeatsInSection;
};

