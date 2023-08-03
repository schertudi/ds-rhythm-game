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

class energyLevelManager {

    int powerupCombo = -1;
    bool missedSection = false;
    int powerupSectionStart = -1;

    

    powerupStates currState = powerupStates::IDLE;

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
        int nextChange = 12;
        int diff = nextChange - startBeat;
        //if (diff < 0) {return 0;}
        return diff;
    }

    

    public:
    powerupStates getCurrState() {
        return currState;
    }

    powerupInfo getCurrPowerupInfo () {
        powerupInfo p;
        p.currState = currState;
        p.numBeatsHit = 2;
        return p;
    }

    void newBeat(int beat) {

        int powerupLength = 4; //need 4 beats in a row before level change
        int powerupBeforeLength = 4; //4 beats before this we tell player to prepare
        int cooldownLength = 4; //how long do we show message before doing nothing
        
        
        int dist = getDistToNextLevelChange(beat);
        if (dist > powerupLength + powerupBeforeLength) {
            currState = powerupStates::IDLE; //nothing interesting here
            return;
        } else if (dist > powerupLength) {
            currState = powerupStates::BEFORE; //warn player
        } else if (dist > 0) {
            currState = powerupStates::ACTIVATING;
            if (currState == powerupStates::BEFORE) {
                currState = powerupStates::ACTIVATING; //warn player
            }
            //need to check if player is getting full combo here
        } else if (dist == 0 || dist > -cooldownLength) {
            //if we didnt lose it we go up a level (maybe if we lost it we go down idk?)
            currState = powerupStates::ACHIEVED;
        } else if (dist < -cooldownLength) {
            //error...
            currState = powerupStates::LOST;
        }

    }

    //i wonder if we should do an event system for on beat hit/miss? a lot of classes may be intersted in this
    //is that overkill?
    void beatHit() {
        //allows moving up a level if done at right time
        //don't update combo if we didn't get it at very first hit (?) or do we want a second chance
        //probably easiest to demand we get them all right; want combo complete to show on final beat, not before
        if (missedSection) {
            return;
        }
        if (powerupCombo == -1) { //just entered

        }
    }

    void beatMiss() {
        //set level to base
        int beat = 0;
        int bar = 0;
        if (isPowerupSection(bar)) {
            //set level to 1
        }
    }

};