#include "../sysWrappers/vectorShapes.h"
#include "../sysWrappers/mathHelpers.h"
#include "../layerDefinitions.h"
#include "hitBeat.h"

/*
more interactable ideas:
swipe (need pen at fixed starting position, then quickly move it in a certain direction, less precision needed than slider)
multi-tap (successive beats appear at same x,y position, but rendering and timing is done so that it actually works) (can then use use bouncingBallStraight)

need a more consistent naming scheme: hitBeat (as opposed to timeBeat)
*/


void HitBeatAbstract::setSlider(bool value) {
    _isSlider = value;
}
void HitBeatAbstract::deactivate(int beat) {
    timeAtDeactivate = beat;
    isDead = true; //beat can be a negative value, so not safe to just say timeAtDeactivate == -1 means active
}

bool HitBeatAbstract::isActive() const {
    return !isDead;
}

int HitBeatAbstract::getTimeSinceDeactivate(int beat) {
    if (!isDead) return -1;
    return beat - timeAtDeactivate;
}

bool HitBeatAbstract::isSlider() const {
    return _isSlider;
} 

void HitBeatAbstract::setPlayerState(playerStatus state) {
    playerState = state;
}

playerStatus HitBeatAbstract::getPlayerState() const {
    return playerState;
}



SingleHitBeat::SingleHitBeat(int _beat, int _x, int _y, int _len, int _pitch) {
    beat = _beat;
    x = _x;
    y = _y;
    radius = 15;
    length = _len;
    pitch = _pitch;
    setSlider(false);
}

void SingleHitBeat::render(int progress) {
    int r;
    if (progress < 0) { r = 5; }
    else if (progress == 100) { r = 10; }
    else if (progress > 100) { r = 2; }
    else r = 5 + progress / 20;

    if (progress == 100) {
        if (beat >= 88 && beat < 96) {
            vectorCircle(x, y, r, {0, 10, 28}, BEATPATH_LAYER);
        } else {
            vectorCircle(x, y, r, {0, 20, 10}, BEATPATH_LAYER);
        }
        
    } else {
        if (beat >= 88 && beat < 96) {
            vectorCircle(x, y, r, {10, 15, 20}, BEATPATH_LAYER);
        } else {
            vectorCircle(x, y, r, {20, 20, 20}, BEATPATH_LAYER);
        }
    }
    
}

bool SingleHitBeat::isHit(int touchX, int touchY, int globalBeat, int progressToNext, int margin) {
    return touchX > x - radius && touchX < x + radius && touchY > y - radius && touchY < y + radius;
}

int SingleHitBeat::getStartBeat() {
    return beat;
}

int SingleHitBeat::getEndBeat() {
    return beat;
}

Vec2d SingleHitBeat::getStartPos() {
    return {x, y};
}

Vec2d SingleHitBeat::getEndPos() {
    return {x, y};
}

int SingleHitBeat::getBeatProgress(int globalBeat, int progressToNext, int margin) {
    if (globalBeat < beat - 2) { //too soon
        return 0;
    }
    if (globalBeat == beat - 2) { //too soon
        return progressToNext / 2;
    }
    if (globalBeat == beat - 1) { //about to hit
        if (progressToNext < 100 - margin) { //not yet hit
            return 50 + progressToNext / 2;
        }
        return 100; //we consider this a hit even if not quite on the beat
    } else if (globalBeat == beat) { //just hit
        if (progressToNext > margin) { //consider a miss, go over 100
            return 300;
        }
        return 100; //still a valid hit
    } else {
        return 300; //too late, 300 for consistency with slider
    }
}

void SingleHitBeat::playSound(AudioPlayer man) {
    man.playNote(length, pitch);
}




SliderHitBeat::SliderHitBeat(int _startBeat, int _endBeat, int _startX, int _startY, int _endX, int _endY, int _len, int _pitch) {
    startBeat = _startBeat;
    endBeat = _endBeat;
    startX = _startX;
    startY = _startY;
    endX = _endX;
    endY = _endY;
    lengthInBeats = endBeat - startBeat;
    noteLength = _len;
    pitch = _pitch;
    setSlider(true);
}

void SliderHitBeat::render(int progress)  {
    int bigRadius = 10;
    int smallRadius = 3;
    int midRadius = 7;
    vectorCircle(endX, endY, smallRadius, {20, 20, 20}, BEATPATH_LAYER - 100);
    
    if (progress < 100) {
        int r = lerp(smallRadius, bigRadius, progress);
        vectorCircle(startX, startY, r, {15, 15, 31}, BEATPATH_LAYER - 99);
        
        vectorThickLine(startX, startY, endX, endY, smallRadius, {20, 20, 20}, BEATPATH_LAYER - 100, false);
    } else if (progress == 100) {
        vectorCircle(startX, startY, bigRadius, {0, 20, 10}, BEATPATH_LAYER - 99);
        vectorThickLine(startX, startY, endX, endY, smallRadius, {20, 20, 20}, BEATPATH_LAYER - 100, false);
    } else if (progress <= 200) {
        int lerpPos = progress - 100; //so 0 if at start of slide and 100 if at end of slide
        if (lerpPos < 0) lerpPos = 0;
        if (lerpPos > 100) lerpPos = 100;
        int targetX = lerp(startX, endX, lerpPos);
        int targetY = lerp(startY, endY, lerpPos);
        
        //if (progress == 100) { vectorCircle(startX, startY, bigRadius, {15, 15, 31}, BEATPATH_LAYER); }
        vectorCircle(startX, startY, midRadius, {15, 15, 31}, BEATPATH_LAYER - 100);
        vectorThickLine(startX, startY, targetX, targetY, midRadius, {15, 15, 31}, BEATPATH_LAYER - 100, false);
        
        vectorThickLine(targetX, targetY, endX, endY, smallRadius, {20, 20, 20}, BEATPATH_LAYER - 100, false);
        vectorCircle(targetX, targetY, bigRadius, {15, 15, 31}, BEATPATH_LAYER - 99);
    } 
    
}

bool SliderHitBeat::isHit(int touchX, int touchY, int globalBeat, int progressToNext, int margin) {
    int p = getBeatProgress(globalBeat, progressToNext, margin);

    if (p < 100) {
        return touchX > startX - startRadius && touchX < startX + startRadius 
            && touchY > startY - startRadius && touchY < startY + startRadius;
    } else if (p <= 200) {
        int midX = startX + (endX - startX) * (p - 100) / 100;
        int midY = startY + (endY - startY) * (p - 100) / 100;
        
        return touchX > midX - slideRadius && touchX < midX + slideRadius 
            && touchY > midY - slideRadius && touchY < midY + slideRadius;
    } 
    return touchX > endX - slideRadius && touchX < endX + slideRadius 
        && touchY > endY - slideRadius && touchY < endY + slideRadius;
}

int SliderHitBeat::getStartBeat() {
    return startBeat;
}

int SliderHitBeat::getEndBeat() {
    return endBeat;
}

Vec2d SliderHitBeat::getStartPos() {
    return {startX, startY};
}

Vec2d SliderHitBeat::getEndPos() {
    return {endX, endY};
}

int SliderHitBeat::getBeatProgress(int globalBeat, int progressToNext, int margin) {
    //0: no movement. 0-99: about to hit beat. 100: hit start. 101-199: in middle of slide. 200: hit end. 200+: missed.
    //in a slider, you want to hit at start and lift at end.
    if (globalBeat < startBeat - 2) { //too soon
        return 0;
    }
    if (globalBeat == startBeat - 2) { //too soon
        return progressToNext / 2;
    }
    if (globalBeat == startBeat - 1) { //about to hit start
        if (progressToNext < 100 - margin) { //not yet hit
            return 50 + progressToNext / 2;
        }
        return 100; //consider this start hit
    } 
    if (globalBeat == startBeat) { //just hit start
        if (progressToNext < margin) { //consider this still on start
            return 100;
        }
        if (globalBeat == endBeat - 1 && progressToNext > 100 - margin) { 
            //this is a middle ground where you lifted a little early but could still legally be in middle - we should accept both
            return 200;
        }
        return 100 + (progressToNext / lengthInBeats); //consider this in middle of slide, needs to be calibrated for multi-beat slides
    }
    if (globalBeat > startBeat && globalBeat < endBeat) { //in middle of slide
        if (globalBeat == endBeat - 1 && progressToNext > 100 - 0) {  //0 should be margin but rn dont care if not lifted in time and early 200 causes slight issues
            return 200;
        }
        int past = (100 / lengthInBeats) * (globalBeat - startBeat + 0);
        return 100 + (progressToNext / lengthInBeats) + past; 
    }
    if (globalBeat == endBeat) { //just hit end
        if (progressToNext < margin) {
            return 200; //lift margin, ok if pen not quite up yet
        }
        return 300; //slide over, if pen was still down when it hits this you missed
    }
    if (globalBeat > endBeat) { //far too late
        return 300;
    }
    return 300;
}

void SliderHitBeat::playSound(AudioPlayer man) {
    man.playNote(noteLength, pitch);
}
