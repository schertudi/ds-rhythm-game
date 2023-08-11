

#include "energyLevelTracker.h"
#include "debugTools.h"



std::tuple<int, int> EnergyLevelTracker::getNextLevelChange(int currBeat, songPosition songPos) {
        /*
    to actually find - we need to know at what bars does level change, know how many beats inside a bar (include sub beats)
    start from current bar and iter thru, keep track of old + current energy level. first time old != current we get that time and LEAVE.
    but we actually dont leave because we want to find what time the last note of seq hits at. probably also count how many in this bar (seq) for funsies xd
    and dont assume powerupLength = 4 or 8 or anything, can be whatever i want it to be (though defining on a "how many bars" makes sense)
    will need to count backwards to get the beat that changes things then
    ignore changes where level goes down, only care about it going up eg level 1 to level 2
    */

    //find index of next bar that changes levels. say bar 0 has level 1 and bar 1 has level 3, we will set changeBar to 1. 
    int lastLevel = level.perBarEnergyLevel[songPos.bar];
    int changeBar = -1;
    for (size_t i = songPos.bar; i < level.perBarEnergyLevel.size(); i++) {
        int currLevel = level.perBarEnergyLevel[i];
        if (lastLevel < currLevel) {
            changeBar = i;
            break;
        }
    }

    //find the last beat player hits before this level change, hitting this after a combo will trigger the change.
    //check changeBar *= 0 (not that it ever should), and dont do this if no further change found (reached end)
    int powerupStartBeat = -1;
    int powerupEndBeat = -1;

    for (size_t i = 0; i < level.beatInteracts.size(); i++) { //some wasteful iteration, might be a TODO to optimise (will see)
        HitBeatAbstract* beatToHit = level.beatInteracts[i];
        int thisBeat = beatToHit->getStartBeat();
        if (beatToHit->isSlider()) { thisBeat = beatToHit->getEndBeat(); }
        
        int thisBar = thisBeat / (songPos.numBeatsInBar * songPos.numSubBeats);
        if (thisBar < changeBar) {
            powerupEndBeat = thisBeat;
        }
        if (thisBar < changeBar - NUM_POWERUP_BARS) {
            powerupStartBeat = thisBeat;
        }
    }

    return {powerupStartBeat, powerupEndBeat};
}

EnergyLevelTracker::EnergyLevelTracker(levelData _levelData) {
    p.currState = powerupStates::IDLE;
    p.timeAtStateChange = 0;
    p.numBeatsHit = 0;
    p.numBeatsInSection = 0;
    level = _levelData;
    currEnergyLevel = 1;
}

void EnergyLevelTracker::newBeat(songPosition songPos) {
    //this will be bugged - beat can trigger before window to hit beat ends, what do?
    //probably means we need to update state in on hit/miss as well, not just here.... probably use it for as little as possible to min duplication
    //BUT if we dont have anything to hit on first beat of bar.. we should do it here.
    //so ig we need info about beatmap :( tho was probably necessary anyway
    //i think for now we just assume last beat switches it.. feels more satisfying that way probably
    //think we need another state in transistin from before to hit - warn player they want to hit now, but dont penalise them for not hitting immediately.
    
    int currBeat = songPos.globalBeat * songPos.numSubBeats + songPos.subBeat;
    std::tuple<int, int> powerup = getNextLevelChange(currBeat, songPos);
    int powerupStart = std::get<0>(powerup);
    int powerupEnd = std::get<1>(powerup);

    bool isIdleTime = currBeat < powerupStart - WARNING_LENGTH;
    bool isBeforeTime = powerupStart - WARNING_LENGTH <= currBeat && currBeat < powerupStart;
    bool isJustBeforeTime = currBeat == powerupStart;
    bool isComboTime = powerupStart < currBeat && currBeat < powerupEnd;

    int barLevel = level.perBarEnergyLevel[songPos.bar];
    if (barLevel < currEnergyLevel && p.currState != powerupStates::WIN) { //could have a bug here if win screen hides before we go onto next bar - hopefully unlikely
        currEnergyLevel = barLevel;
    }


    if (isIdleTime) {
        p.currState = powerupStates::IDLE; //nothing interesting here
        return;
    }
    
    if (isBeforeTime && p.currState != powerupStates::BEFORE) {
        p.currState = powerupStates::BEFORE; //warn player
        p.timeAtStateChange = songPos.globalBeat;
        p.numBeatsInSection = WARNING_LENGTH;
        p.numBeatsHit = 0;
    }
    if (isJustBeforeTime && p.currState != powerupStates::JUST_BEFORE) {
        p.currState = powerupStates::JUST_BEFORE;
        p.numBeatsHit = 0;
        p.timeAtStateChange = songPos.globalBeat;
        p.numBeatsInSection = powerupEnd - powerupStart;
    } 
    if (isComboTime && p.currState == powerupStates::JUST_BEFORE) {
        p.currState = powerupStates::GET_COMBO;
    }
    
    if (p.currState == powerupStates::WIN || p.currState == powerupStates::LOSE) { //there are situations we may not reach this and be stuck forever...
        if (songPos.globalBeat - p.timeAtStateChange > COOLDOWN_LENGTH) {
            p.currState = powerupStates::IDLE;
        }
    }

    if (p.currState == powerupStates::GET_COMBO && currBeat > powerupEnd) { //should not be allowed !!
        Debugger::error("stuck in state GET_COMBO!");
    }

    //actually changing level should be dependent on hit/miss so not done at this time

}

//i wonder if we should do an event system for on beat hit/miss? a lot of classes may be intersted in this
//is that overkill?
void EnergyLevelTracker::beatHit(int registerBeat, songPosition songPos) {
    //we pass in a diff beat to register than songPos.globalBeat as player can still legally hit a little bit after the beat actually changes
    //in which case songPos.globalBeat is not the value we want. instead registerBeat = the expected beat of an interactable.
    //allows moving up a level if done at right time
    //don't update combo if we didn't get it at very first hit (?) or do we want a second chance
    //probably easiest to demand we get them all right; want combo complete to show on final beat, not before

    //i think better to check by beat hit and not specific timings - there are 2 possible beats we could be hitting and 1 is right 1 is not????
    //or send out beat change a little earlier - like valid beat and not actual beat - because hit/miss should distinguish between
    //but what seems to be happening is that it's waiting until beat 3 to register hits and im actually on beat 2.8 so what do : (
    //or can just allow for it a bit earlier i guess ?
    //nah probably best to pass in info about beat that has been hit, we can see if this is valid or not
    if (p.currState != powerupStates::GET_COMBO && p.currState != powerupStates::JUST_BEFORE) {
        return;
    }

    std::tuple<int, int> powerup = getNextLevelChange(registerBeat, songPos);
    int powerupEnd = std::get<1>(powerup);

    //int dist = getDistToNextLevelChange(registerBeat, songPos);
    //Debugger::print("hit at beat %d with dist %d", beat, dist);
    p.numBeatsHit += 1;
    if (registerBeat == powerupEnd) {//hit for next section
        p.currState = powerupStates::WIN;
        p.timeAtStateChange = registerBeat / songPos.numSubBeats; //super vague if beat is granular or not. need some kind of better clarity around this.
        //need to change level to that of the next bar (NOT this current one)
        int currBar = registerBeat / (songPos.numBeatsInBar * songPos.numSubBeats);
        int nextBar = currBar + 1;

        if (nextBar >= 0 && (size_t)nextBar < level.perBarEnergyLevel.size()) {
            //will update slightly too late (early animations won't show), unsure if should fix or not
            int maxLevel = level.perBarEnergyLevel[nextBar];
            if (currEnergyLevel < maxLevel) {
                currEnergyLevel += 1;
            }
        }
    } 
}

void EnergyLevelTracker::beatEarly(int beat) {
    if (p.currState == powerupStates::GET_COMBO || p.currState == powerupStates::JUST_BEFORE) {
        p.currState = powerupStates::LOSE;
        p.timeAtStateChange = beat;
    }
}

void EnergyLevelTracker::beatMiss(int beat) {
    if (p.currState == powerupStates::GET_COMBO) {
        p.currState = powerupStates::LOSE;
        p.timeAtStateChange = beat;
    }
    
}

int EnergyLevelTracker::getEnergyLevel() {
    return currEnergyLevel;
}
