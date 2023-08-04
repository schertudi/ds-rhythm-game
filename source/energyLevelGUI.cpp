//communicates current energy level to player
/*
need to know:
current state
time at state change (can calc num beats since ez since we want curr time anyway)
numBeatsHit
*but* if we are on win/lose screen we want to know how many we got at last section, probs easiest to have this as a pre-calculated number "num beats hit"
still want time at state change so it's easy to animate things tho

song pos

*/

#include "debugTools.h"
#include "constants.h"



class EnergyLevelGUI {

    public:
    void draw(powerupInfo p, songPosition pos) {
        if (p.currState == powerupStates::BEFORE) {
            drawBeforeSection(pos.time, p.numBeatsHit);
        } else if (p.currState == powerupStates::ACTIVATING) {
            drawPowerupSection(pos.time, p.numBeatsHit);
        } else if (p.currState == powerupStates::WIN) {
            drawWinSection(pos.time, p.timeAtStateChange, p.numBeatsHit);
        } else if (p.currState == powerupStates::LOSE) {
            drawFailSection(pos.time, p.timeAtStateChange, p.numBeatsHit);
        } 
        Debugger::framePrint("");
    }

    private:

    void printNChars(int n, char c) {
        char buffer[n + 1];
        for (int i = 0; i < n; i++){
            buffer[i] = c;
        }
        buffer[n] = '\0';

        Debugger::framePrint(buffer);
    }

    void drawBeforeSection(int currTime, int numComplete) {
        Debugger::framePrint("get ready..");
        printNChars(numComplete, '-');
    }

    void drawPowerupSection(int currTime, int numComplete) {
        Debugger::framePrint("do your best!");
        printNChars(numComplete, '*');
    }

    //these should last n beats from time state switched to them
    void drawFailSection(int currTime, int startTime, int numComplete) {
        Debugger::framePrint("next time...");
        printNChars(numComplete, '*');
        //need to know how many we got for accurate animation
    }

    void drawWinSection(int currTime, int startTime, int numComplete) {
        Debugger::framePrint("level up!");
        printNChars(numComplete, '_');
        //need to know how many we got for accurate animation
    }


};