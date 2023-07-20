#ifndef CONSTANTS_H
#define CONSTANTS_H


#define HALF_WIDTH (SCREEN_WIDTH/2)
#define HALF_HEIGHT (SCREEN_HEIGHT/2)
#define BRAD_PI (1 << 14)

struct Colour {
    int r;
    int g;
    int b;
};

struct Vec2d {
    int x;
    int y;
};

struct songPosition {
    int globalBeat;
    int localBeat;
    int subBeat;
    int bar;
    int globalBeatProgress; //used for animations
    int subBeatProgress; //used for calculating timings
    int numSubBeats;
    int numBeatsInBar;
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

enum class direction { TOP, BOTTOM, LEFT, RIGHT };

#define NUM_SUBLAYERS 10
#define ANIMATION_BG_LAYER 1 * NUM_SUBLAYERS
#define ANIMATION_MG_LAYER 2 * NUM_SUBLAYERS
#define ANIMATION_FG_LAYER 3 * NUM_SUBLAYERS
#define BEATPATH_LAYER 4 * NUM_SUBLAYERS



#endif