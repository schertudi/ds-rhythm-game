#include "vectorShapes.h"
#include "audioManager.cpp"
#include "constants.h"
#include "mathHelpers.h"

/*
more interactable ideas:
swipe (need pen at fixed starting position, then quickly move it in a certain direction, less precision needed than slider)
multi-tap (successive beats appear at same x,y position, but rendering and timing is done so that it actually works) (can then use use bouncingBallStraight)
*/

class BeatInteractable {
    private:
    bool _isSlider = false;
    playerStatus playerState = playerStatus::IDLE;
    int timeAtDeactivate;
    bool isDead = false;

    protected:
    void setSlider(bool value) {
        _isSlider = value;
    }

    public:
    virtual void render(int progress) = 0; //<100 means approaching, 100 means at beat, >100 means passed
    virtual bool isHit(int touchX, int touchY, int globalBeat, int progressToNext, int margin) = 0; //mind radius on this one, can wrongly detect collisions if too big
    virtual int getStartBeat() = 0;
    virtual int getEndBeat() = 0; //unused.. for now?
    virtual Vec2d getStartPos() = 0;
    virtual Vec2d getEndPos() = 0;
    virtual int getBeatProgress(int globalBeat, int progressToNext, int margin) = 0;
    virtual void playSound(AudioManager man) = 0;

    void deactivate(int beat) {
        timeAtDeactivate = beat;
        isDead = true; //beat can be a negative value, so not safe to just say timeAtDeactivate == -1 means active
    }

    bool isActive() {
        return !isDead;
    }

    int getTimeSinceDeactivate(int beat) {
        if (!isDead) return -1;
        return beat - timeAtDeactivate;
    }

    bool isSlider() const {
        return _isSlider;
    } 

    void setPlayerState(playerStatus state) {
        playerState = state;
    }

    playerStatus getPlayerState() {
        return playerState;
    }
};

class BeatToHit : public BeatInteractable {
    private:
    int beat;
    int x;
    int y;
    int radius;
    int length;
    int pitch;
    
    public:
    BeatToHit(int _beat, int _x, int _y, int _len, int _pitch) {
        beat = _beat;
        x = _x;
        y = _y;
        radius = 15;
        length = _len;
        pitch = _pitch;
        setSlider(false);
    }

    void render(int progress) override {
        int r;
        if (progress < 0) { r = 7; }
        else if (progress == 100) { r = 5; }
        else if (progress > 100) { r = 2; }
        else r = 7 - progress / 20;

        int a = 20;
        if (progress == 100) a = 31;
        vectorCircle(x, y, r, {a, 20, 20}, BEATPATH_LAYER);
    }

    bool isHit(int touchX, int touchY, int globalBeat, int progressToNext, int margin) override {
        return touchX > x - radius && touchX < x + radius && touchY > y - radius && touchY < y + radius;
    }

    int getStartBeat() override {
        return beat;
    }

    int getEndBeat() override {
        return beat;
    }

    Vec2d getStartPos() override {
        return {x, y};
    }

    Vec2d getEndPos() override {
        return {x, y};
    }

    int getBeatProgress(int globalBeat, int progressToNext, int margin) {
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

    void playSound(AudioManager man) override {
        man.playNote(length, pitch);
    }

};

class BeatToSlide : public BeatInteractable {
    private:
    int startBeat;
    int endBeat;
    int startX;
    int startY;
    int endX;
    int endY;
    int radius;
    int lengthInBeats;
    int noteLength;
    int pitch;
    
    public:
    BeatToSlide(int _startBeat, int _endBeat, int _startX, int _startY, int _endX, int _endY, int _len, int _pitch) {
        startBeat = _startBeat;
        endBeat = _endBeat;
        startX = _startX;
        startY = _startY;
        endX = _endX;
        endY = _endY;
        radius = 15;
        lengthInBeats = endBeat - startBeat;
        noteLength = _len;
        pitch = _pitch;
        setSlider(true);
    }

    void render(int progress) override {
        int bigRadius = 7;
        int smallRadius = 3;
        vectorCircle(endX, endY, bigRadius, {20, 20, 20}, BEATPATH_LAYER - 1);
        
        if (progress < 100) {
            int r = lerp(bigRadius, smallRadius, progress);
            vectorCircle(startX, startY, r, {15, 15, 31}, BEATPATH_LAYER + 1);
            
            vectorThickLine(startX, startY, endX, endY, bigRadius, {20, 20, 20}, BEATPATH_LAYER, false);
        } else if (progress <= 200) {
            int lerpPos = progress - 100; //so 0 if at start of slide and 100 if at end of slide
            if (lerpPos < 0) lerpPos = 0;
            if (lerpPos > 100) lerpPos = 100;
            int targetX = lerp(startX, endX, lerpPos);
            int targetY = lerp(startY, endY, lerpPos);
            
            if (progress == 100) { vectorCircle(startX, startY, smallRadius, {15, 15, 31}, BEATPATH_LAYER); }
            vectorThickLine(startX, startY, targetX, targetY, smallRadius, {15, 15, 31}, BEATPATH_LAYER, false);
            
            vectorThickLine(targetX, targetY, endX, endY, bigRadius, {20, 20, 20}, BEATPATH_LAYER, false);
            vectorCircle(targetX, targetY, smallRadius, {10, 10, 25}, BEATPATH_LAYER + 2);
        } 
        
    }

    bool isHit(int touchX, int touchY, int globalBeat, int progressToNext, int margin) override {
        int p = getBeatProgress(globalBeat, progressToNext, margin);

        if (p < 100) {
            return touchX > startX - radius && touchX < startX + radius 
                && touchY > startY - radius && touchY < startY + radius;
        } else if (p <= 200) {
            int midX = startX + (endX - startX) * (p - 100) / 100;
            int midY = startY + (endY - startY) * (p - 100) / 100;
            
            return touchX > midX - radius && touchX < midX + radius 
                && touchY > midY - radius && touchY < midY + radius;
        } 
        return touchX > endX - radius && touchX < endX + radius 
            && touchY > endY - radius && touchY < endY + radius;
    }

    int getStartBeat() override {
        return startBeat;
    }

    int getEndBeat() override {
        return endBeat;
    }

    Vec2d getStartPos() override {
        return {startX, startY};
    }

    Vec2d getEndPos() override {
        return {endX, endY};
    }

    int getBeatProgress(int globalBeat, int progressToNext, int margin) override {
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

    void playSound(AudioManager man) override {
        man.playNote(noteLength, pitch);
    }

};