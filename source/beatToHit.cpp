#include "vectorShapes.h"
#include "audioManager.cpp"
#include "constants.h"

class BeatInteractable {
    private:
    bool isDead = false;
    bool _isPenTouching = false;
    bool _isSlider = false;

    public:
    virtual void render(int progress) = 0; //<100 means approaching, 100 means at beat, >100 means passed
    virtual bool isHit(int touchX, int touchY) = 0;
    virtual int getHitBeat();
    virtual int getStartBeat();
    virtual int getEndBeat();
    virtual int getBeatProgress(int globalBeat, int progressToNext, int margin);

    int getRadiusByProgress(int progress) {
        if (progress < 0) {
            return 10;
        }
        if (progress == 100) return 5;
        if (progress > 100) {
            return 2;
        }
        return 10 - progress / 20;
    }

    virtual bool isPenInRightPlace(int globalBeat, int progressToNext, int margin, int touchX, int touchY);

    void markForKill() {
        isDead = true;
    }

    bool isMarkedForKill() {
        return isDead;
    }

    void setSlider(bool value) {
        _isSlider = value;
    }

    bool isSlider() const {
        return _isSlider;
    } 

    void setIsPenTouching(bool value) {
        _isPenTouching = value;
    }

    bool isPenTouching() const {
        return _isPenTouching;
    } 

    virtual void tryPlaySound(AudioManager man);
    virtual void resetSound();
};

class BeatToHit : public BeatInteractable {
    public:
    int beat;
    int x;
    int y;
    int radius;
    int length;
    int pitch;
    bool hasNotePlayed = false;
    

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
        int r = getRadiusByProgress(progress);
        vectorCircle(x, y, r, {31, 31, 31}, BEATPATH_LAYER);
    }

    bool isHit(int touchX, int touchY) override {
        if (touchX > x - radius && touchX < x + radius && touchY > y - radius && touchY < y + radius) {
            return true;
        }
        return false;
    }

    int getHitBeat() override {
        return beat;
    }

    int getStartBeat() override {
        return beat;
    }

    int getEndBeat() override {
        return beat;
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

    bool isPenInRightPlace(int globalBeat, int progressToNext, int margin, int touchX, int touchY) override { 
        return true;
    }

    void tryPlaySound(AudioManager man) override {
        if (hasNotePlayed) return;
        man.playNote(length, pitch);
        hasNotePlayed = true;
    }

    void resetSound() override {
        hasNotePlayed = false;
    }


};

class BeatToSlide : public BeatInteractable {
    public:
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
    bool hasNotePlayed = false;
    

    BeatToSlide(int _startBeat, int _endBeat, int _startX, int _startY, int _endX, int _endY, int _len, int _pitch) {
        startBeat = _startBeat;
        endBeat = _endBeat;
        startX = _startX;
        startY = _startY;
        endX = _endX;
        endY = _endY;
        radius = 10;
        lengthInBeats = endBeat - startBeat;
        noteLength = _len;
        pitch = _pitch;
        setSlider(true);
    }

    void render(int progress) override {
        int r = 10;
        vectorCircle(endX, endY, r, {31, 31, 31}, BEATPATH_LAYER);
        
        if (progress < 100) {
            
            r = 10 - progress / 20;
            vectorCircle(startX, startY, r, {31, 31, 31}, BEATPATH_LAYER);

            r = 10;
            vectorRect(startX, startY - r, endX, endY + r, {31, 31, 31}, BEATPATH_LAYER);
        } else if (progress <= 200) {
            int r1 = 5;
            int r2 = 10;
            int midX = startX + (endX - startX) * (progress - 100) / 100;

            /*
            int lerpPos = progress - 100; //so 0 if at start of slide and 100 if at end of slide
            if (lerpPos < 0) lerpPos = 0;
            if (lerpPos > 100) lerpPos = 100;
            int targetX = lerp(startX, endX, lerpPos);
            int targetY = lerp(startY, endY, lerpPos);
            */

            //int midY = (startY + endY) * (progress - 100) / 100;
            if (progress == 100) vectorCircle(startX, startY, r1, {15, 15, 31}, BEATPATH_LAYER);
            vectorRect(startX, startY - r1, midX, endY + r1, {15, 15, 31}, BEATPATH_LAYER);
            vectorRect(midX, startY - r2, endX, endY + r2, {31, 31, 31}, BEATPATH_LAYER);
        } else {
            r = 10;
            
        }

        
    }

    bool isHit(int touchX, int touchY) override {
        //we check using box from circle 1 to circle 2
        if (touchX > startX - radius && touchX < endX + radius && touchY > startY - radius && touchY < endY + radius) {
            setIsPenTouching(true);
            return true;
        }
        setIsPenTouching(false);
        return false;
    }

    int getHitBeat() override {
        return endBeat;
    }

    int getStartBeat() override {
        return startBeat;
    }

    int getEndBeat() override {
        return endBeat;
    }

    int getBeatProgress(int globalBeat, int progressToNext, int margin) {
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

    bool isPenInRightPlace(int globalBeat, int progressToNext, int margin, int touchX, int touchY) override {
        int p = getBeatProgress(globalBeat, progressToNext, margin);
        //touchX depends on p; lerp between start and end
        //int targetX, targetY;
        if (p < 100) {
            return false; // too soon
        } else if (p <= 200) {
            int midX = startX + (endX - startX) * (p - 100) / 100;
            int midY = startY + (endY - startY) * (p - 100) / 100;

            //TODO why the fuck does lerp not work??
            /*
            int lerpPos = p - 100; //so 0 if at start of slide and 100 if at end of slide
            targetX = lerp(startX, endX, lerpPos);
            targetY = lerp(startY, endY, lerpPos);
            */

            //vectorCircle(midX, targetY, 10, {31, 0, 0});

            /*
            int minLerp = lerpPos - margin; //the mouse is allowed to lag behind by margin, so if margin = 5 then we can be 5% behind
            if (minLerp < 0) minLerp = 0;

            int minX = lerp(startX, endX, minLerp);
            int minY = lerp(startY, endY, minLerp);
            */
            int allowedRadius = 30; //must be within 10px radius of target

            //now check if pen in this allowed circle
            //return true;
            return touchX > midX - allowedRadius && touchX < midX + allowedRadius 
                && touchY > midY - allowedRadius && touchY < midY + allowedRadius;
        } 
        return false; // too late
    }

    void tryPlaySound(AudioManager man) {
        if (hasNotePlayed) return;
        man.playNote(noteLength, pitch);
        hasNotePlayed = true;
    }

    void resetSound() override {
        hasNotePlayed = false;
    }
};