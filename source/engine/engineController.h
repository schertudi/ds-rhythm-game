#pragma once

#include "../sysWrappers/audioPlayer.h"
#include "songTimeTracker.h"
#include "anim/animationCommandManager.h"
#include "hitBeatUpdater.h"
#include "energyLevelTracker.h"
#include "energyLevelDisplay.h"
#include "../genericTypes.h"
#include "../sceneObject.h"


class EngineController : public SceneObject {

    public:

    void load (AudioPlayer* _audioPlayer) override;

    sceneStates update() override;

    void unload() override;

    private:
    int frame;
    int combo;
    bool isAutomatedPlay;
  
    int bpm = 120;
    int numSubBeats = 2;
    int numBeatsInBar = 4;

    AudioPlayer* audioPlayer;
    SongTimeTracker timeTracker;
    AnimationCommandManager animationCommandManager;
    HitBeatUpdater path;
    EnergyLevelTracker energyLevelTracker; //illegal
    EnergyLevelDisplay energyLevelDisplay;

    void playMetronome(int beatStatus);

    void updateHitBeats(std::vector<playableBeatStatus> beatStates, songPosition songPos);



};