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

#include "energyLevelDisplay.h"
#include "debugTools.h"




void EnergyLevelDisplay::draw(powerupInfo p, songPosition pos) {
    if (p.currState == powerupStates::BEFORE) {
        drawBeforeSection(p, pos);
    } 
    else if (p.currState == powerupStates::JUST_BEFORE) {
        drawJustBeforeSection(p, pos);
    }
    else if (p.currState == powerupStates::GET_COMBO) {
        drawPowerupSection(p, pos);
    } 
    else if (p.currState == powerupStates::WIN) {
        drawWinSection(p, pos);
    } 
    else if (p.currState == powerupStates::LOSE) {
        drawFailSection(p, pos);
    } 
    Debugger::framePrint("");
}



void EnergyLevelDisplay::printNChars(int n, char c) {
    char buffer[n + 1];
    for (int i = 0; i < n; i++){
        buffer[i] = c;
    }
    buffer[n] = '\0';

    Debugger::framePrint(buffer);
}

void EnergyLevelDisplay::drawBeforeSection(powerupInfo p, songPosition pos) {
    Debugger::framePrint("get ready..");
    int beatsSinceStart = pos.globalBeat - p.timeAtStateChange;
    printNChars(beatsSinceStart + 1, '-');
}

void EnergyLevelDisplay::drawJustBeforeSection(powerupInfo p, songPosition pos) {
    Debugger::framePrint("do your best?");
    printNChars(p.numBeatsHit, '*');
}

void EnergyLevelDisplay::drawPowerupSection(powerupInfo p, songPosition pos) {
    Debugger::framePrint("do your best!");
    printNChars(p.numBeatsHit, '*');
}

//these should last n beats from time state switched to them
void EnergyLevelDisplay::drawFailSection(powerupInfo p, songPosition pos) {
    Debugger::framePrint("next time...");
    printNChars(p.numBeatsHit, '*');
    //need to know how many we got for accurate animation
}

void EnergyLevelDisplay::drawWinSection(powerupInfo p, songPosition pos) {
    Debugger::framePrint("level up!");
    printNChars(p.numBeatsHit, '-');
    //need to know how many we got for accurate animation
}
