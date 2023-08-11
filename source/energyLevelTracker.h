#pragma once

#include <tuple>
#include "constants.h"
#include "levelParser.h"

class EnergyLevelTracker {
    const int NUM_POWERUP_BARS = 1;
    const int WARNING_LENGTH = 8; //measured in subbeats
    const int COOLDOWN_LENGTH = 4; //measured in beats

    int powerupCombo = -1;
    bool missedSection = false;
    int powerupSectionStart = -1;
    powerupInfo p;
    levelData level;
    int currEnergyLevel;


    std::tuple<int, int> getNextLevelChange(int currBeat, songPosition songPos);
 
    public:
    EnergyLevelTracker(levelData _levelData);

    powerupInfo getCurrPowerupInfo () {
        return p;
    }

    void newBeat(songPosition songPos);

    //i wonder if we should do an event system for on beat hit/miss? a lot of classes may be intersted in this
    //is that overkill?
    void beatHit(int registerBeat, songPosition songPos);

    void beatEarly(int beat);

    void beatMiss(int beat);

    int getEnergyLevel();

};