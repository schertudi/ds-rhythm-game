#ifndef CONSTANTS_H
#define CONSTANTS_H


#define HALF_WIDTH (SCREEN_WIDTH/2)
#define HALF_HEIGHT (SCREEN_HEIGHT/2)
#define BRAD_PI (1 << 14)

struct songPosition {
    int globalBeat;
    int localBeat;
    int subBeat;
    int bar;
    int globalBeatProgress; //used for animations
    int subBeatProgress; //used for calculating timings
};

#endif