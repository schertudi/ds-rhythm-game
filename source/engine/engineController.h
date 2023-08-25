#pragma once

#include "../sysWrappers/audioPlayer.h"
#include "songTimeTracker.h"
#include "anim/animationCommandManager.h"
#include "hitBeatUpdater.h"
#include "energyLevelTracker.h"
#include "energyLevelDisplay.h"
#include "../genericTypes.h"

class EngineController {

    public:

    void init ();

    void update();

    private:
    int frame;
    int combo;
    bool isAutomatedPlay;
  
    int bpm = 120;
    int numSubBeats = 2;
    int numBeatsInBar = 4;

    AudioPlayer audioPlayer;
    SongTimeTracker timeTracker;
    AnimationCommandManager animationCommandManager;
    HitBeatUpdater path;
    EnergyLevelTracker energyLevelTracker; //illegal
    EnergyLevelDisplay energyLevelDisplay;

    void playMetronome(int beatStatus);

    void updateHitBeats(std::vector<playableBeatStatus> beatStates, songPosition songPos);



};