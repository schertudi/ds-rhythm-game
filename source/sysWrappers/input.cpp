#include <nds.h>
#include <nds/arm9/input.h>
#include "input.h"

bool isAutoPlayButtonDown() {
    scanKeys();
    int justPressed = keysDown();
    return justPressed & KEY_L;
}

Vec2d getPenPos() {
    scanKeys();
    int held = keysHeld();
    bool isPenDown = held & KEY_TOUCH;
    if (isPenDown) {
        touchPosition touch;
        touchRead(&touch);
        return {touch.px, touch.py};
    }

    return {-100, -100};
}