/*
variables:
bar-to-level lookup array
need to know if combo has been broken in powerup section. if it has we go down a level. if it has not we can move up a level when appropriate.
output for main is basically just current level it should pass into animator
also need info for gui - so state, beat relative to start/end (beatsToNext) and how many in seqeuence there are (beatsInSequence)
powerup in 3,2,1...GO

main just triggers this from update()
info we need - songpos, per-beat energy level, combo (or at least if a hit/miss happened)

how does energy level state change?
on each new beat we need to check if we are 9 beats away from a new level. this lets us log if next beat is a W or an L.
might even need a few beats before this so interface can update - which is why we dont just want it to work from on hit or miss.
so we want to see if we are idle, before a powerup, activating a powerup, just finished a powerup, or lost a powerup.
I LOVE STATE MACHINES!!
when the beat's level is changed we forget about this and just set level accordingly (+= if win)
*/

#include "constants.h"
#include "debugTools.h"

class energyLevelManager {

    int powerupCombo = -1;
    bool missedSection = false;
    int powerupSectionStart = -1;
    powerupInfo p;

    void getCurrentBarLevel() { //uses songpos

    }

    //powerup section is when we are 1 or 2 bars before one in which the energy level goes up
    //getting a full combo in this section will allow us to meet this
    //losing the combo will drop us down a level (or go to level 1)
    bool isPowerupSection(int beat) {
        //lookahead to see if there is any change from (beat, beat+2)
        return false;
    }

    int getDistToNextLevelChange(int startBeat) { //so say we are at beat 0 and level changes at beat 3, we return 3. if at beat 3 we return 0 (changes now).
        int nextChange = 6; //should be last note in seq we have to hit to activate (NOT first beat in next level). breaks if a beat does not exist here.
        int diff = nextChange - startBeat;
        //if (diff < 0) {return 0;}
        return diff;
    }

    

    public:
    energyLevelManager() {
        p.currState = powerupStates::IDLE;
        p.timeAtStateChange = 0;
        p.numBeatsHit = 0;
        p.numBeatsInSection = 0;
    }

    powerupInfo getCurrPowerupInfo () {
        return p;
    }

    void newBeat(int beat) {
        //this will be bugged - beat can trigger before window to hit beat ends, what do?
        //probably means we need to update state in on hit/miss as well, not just here.... probably use it for as little as possible to min duplication
        //BUT if we dont have anything to hit on first beat of bar.. we should do it here.
        //so ig we need info about beatmap :( tho was probably necessary anyway
        //i think for now we just assume last beat switches it.. feels more satisfying that way probably
        //think we need another state in transistin from before to hit - warn player they want to hit now, but dont penalise them for not hitting immediately.

        int powerupLength = 8; //need 4 beats in a row before level change
        int powerupBeforeLength = 8; //4 beats before this we tell player to prepare
        int cooldownLength = 4; //how long do we show message before doing nothing
        
        
        int dist = getDistToNextLevelChange(beat);
        bool isIdleTime = dist > powerupLength + powerupBeforeLength;
        bool isBeforeTime = !isIdleTime && dist > powerupLength;
        bool isJustBeforeTime = dist == powerupBeforeLength;
        bool isComboTime = dist < powerupBeforeLength && dist > 0;
        bool isChangeTime = dist == 0;
        bool isCooldownTime = dist < 0 && dist > -cooldownLength;


        if (isIdleTime) {
            p.currState = powerupStates::IDLE; //nothing interesting here
            return;
        }
        
        if (isBeforeTime && p.currState != powerupStates::BEFORE) {
            p.currState = powerupStates::BEFORE; //warn player
            p.timeAtStateChange = beat;
            p.numBeatsInSection = powerupBeforeLength;
            p.numBeatsHit = 0;
        }
        if (isJustBeforeTime && p.currState != powerupStates::JUST_BEFORE) {
            p.currState = powerupStates::JUST_BEFORE;
            p.numBeatsHit = 0;
            p.timeAtStateChange = beat;
            p.numBeatsInSection = powerupLength;
        } 
        if (isComboTime && p.currState == powerupStates::JUST_BEFORE) {
            p.currState = powerupStates::GET_COMBO;
        }
        
        if (p.currState == powerupStates::WIN || p.currState == powerupStates::LOSE) { //there are situations we may not reach this and be stuck forever...
            if (beat - p.timeAtStateChange > cooldownLength) {
                p.currState = powerupStates::IDLE;
            }
        }

        if (p.currState == powerupStates::GET_COMBO && dist <= 0) { //should not be allowed !!
            Debugger::error("stuck in state GET_COMBO!");
        }

        //actually changing level should be dependent on hit/miss so not done at this time

    }

    //i wonder if we should do an event system for on beat hit/miss? a lot of classes may be intersted in this
    //is that overkill?
    void beatHit(int beat) {
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

        int dist = getDistToNextLevelChange(beat);
        Debugger::print("hit at beat %d with dist %d", beat, dist);
        if (dist == 0) {//hit for next section
            p.currState = powerupStates::WIN;
            p.timeAtStateChange = beat;
        } else {
            p.numBeatsHit += 1;
        }
    }

    void beatEarly(int beat) {
        if (p.currState == powerupStates::GET_COMBO || p.currState == powerupStates::JUST_BEFORE) {
            p.currState = powerupStates::LOSE;
            p.timeAtStateChange = beat;
        }
    }

    void beatMiss(int beat) {
        if (p.currState == powerupStates::GET_COMBO) {
            p.currState = powerupStates::LOSE;
            p.timeAtStateChange = beat;
        }
        
    }

};