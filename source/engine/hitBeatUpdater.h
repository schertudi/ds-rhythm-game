#pragma once

#include <vector>
#include "constants.h"
#include "hitBeat.h"

//manages hit beats throughout their lifespan by deciding when to activate/deactivate. gives external objects a simple overview of current hit beat states.
//a bit of a god class but that's ok :)
//beat status manager (really we just care about what states of currently spawned beats are)

class HitBeatUpdater {

    private:
    int beatLookAhead;
    int margin; // if we are 10% close to beat or 10% past this is still correct
    std::vector<HitBeatAbstract*> beatInteractObjs;
    std::vector<HitBeatAbstract*> spawnedBeats;

    public:
    HitBeatUpdater(int _lookahead, int _margin, std::vector<HitBeatAbstract*> _beatInteractObjs);
   
    void onBeat(songPosition pos);

    std::vector<playableBeatStatus> getBeatStates(songPosition pos, int touchX, int touchY, bool isAutomatedPlay);

    void renderBeats(songPosition pos);

    void deactivateBeat(int beatStart, int currBeat);

    void playSound(int beatStart, AudioPlayer man);

    private:

    void activateCurrentBeat(songPosition pos);

    void killInactiveBeats(int currBeat, int timeout);


};