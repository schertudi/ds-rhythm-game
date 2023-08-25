#pragma once

#include "engineTypes.h"


class EnergyLevelDisplay {

    public:
    void draw(powerupInfo p, songPosition pos);

    private:

    void printNChars(int n, char c);

    void drawBeforeSection(powerupInfo p, songPosition pos);

    void drawJustBeforeSection(powerupInfo p, songPosition pos);

    void drawPowerupSection(powerupInfo p, songPosition pos);

    //these should last n beats from time state switched to them
    void drawFailSection(powerupInfo p, songPosition pos);

    void drawWinSection(powerupInfo p, songPosition pos);


};