#ifndef ANIMATION_COMMAND_C
#define ANIMATION_COMMAND_C

#include <vector>
#include <tuple>
#include "animator.cpp"
#include "mathHelpers.h"
#include "vectorShapes.h"
#include "debugTools.h"



class AnimationCommand {
    private:
    int energyLevel;

    protected:

    bool inStartPhase(int beat, int startBeat, int preBeats) {
        return beat >= startBeat - preBeats && beat < startBeat;
    }

    bool inActivePhase(int beat, int startBeat, int endBeat) {
        return beat >= startBeat && beat <= endBeat;
    }

    bool inEndPhase(int beat, int endBeat, int postBeats) {
        return beat > endBeat && beat <= endBeat + postBeats;
    }

    int getOffset(int beatA, int progressA, int beatB, int progressB) {
        int timeA = beatA * 100 + progressA;
        int timeB = beatB * 100 + progressB;
        int offset = timeB - timeA;
        return offset;
    }

    int getBeatIndex(int beat, std::vector<playableBeatStatus> beatStates) {
        for (size_t i = 0; i < beatStates.size(); i++) {
            if (beatStates[i].beatStart == beat) {
                return i;
            }
        }
        return -1;
    }

    void setEnergyLevel(int l) {
        energyLevel = l;
    }

    public:

    int getEnergyLevel() {
        return energyLevel;
    }

    virtual void update(int beat, int progress, std::vector<playableBeatStatus> beatStates, Vec2d penPos) = 0;

    virtual int getVal() {
        return 0;
    }
};

class SineWaveAnimation : public AnimationCommand {
    private:
    int startBeat;
    int endBeat;
    int preBeats = 2;
    int postBeats = 2;
    direction wallSide;
    Animator animator;

    public:
    SineWaveAnimation(int energy, int _startBeat, int _endBeat, direction _wall) { //can customize this with specific parameters
        startBeat = _startBeat;
        endBeat = _endBeat;
        wallSide = _wall;
        animator = Animator();
        setEnergyLevel(energy);
    }

    void update(int beat, int progress, std::vector<playableBeatStatus> beatStates, Vec2d penPos) override {
        if (inStartPhase(beat, startBeat, preBeats) || inActivePhase(beat, startBeat, endBeat) || inEndPhase(beat, endBeat, postBeats) ) { 
            animator.sineWave(beat, progress, wallSide, {10, 10, 10});
        }
    }
};



/*
to keep track of animation state when it persists over a few beats is messy. there is also a lot of duplicated behaviour between implementations.
i could muddle with inheritance, but what i think i actually want is a standalone class that i can go "hey figure out the logic of whether or not
i'm allowed to render right now". i just take the output of that and animate as i want.
*/

class MultiBeatStateTracker {
    public:
    int lastHitBeat;
    int relativeBeat;
    bool started;
    bool finished;
    bool killed;

    MultiBeatStateTracker() {

    }

    void init(int _startBeat, int _numBeats, int _beatGap) {
        startBeat = _startBeat;
        numBeats = _numBeats;
        beatGap = _beatGap;
        endBeat = startBeat + numBeats * beatGap;
        started = false;
        killed = false;
        finished = false;
        lastHitBeat = _startBeat - _beatGap;
        relativeBeat = findRelativeBeat(lastHitBeat);
    }
 
    bool update(int currentBeat, std::vector<playableBeatStatus> beatStates) { //return true if progressed
        if (reachedLastBeat(currentBeat)) {
            finished = true;
            return false;
        }
        if (killed) { return false; }

        if (currentBeat >= startBeat - 1) { 
            started = true; 
        } else {
            return false;
        }

        if (beatInSequenceMissed(beatStates)) {
            killed = true;
            lastHitBeat = -1;
            relativeBeat = -1;
            return false;
        }

        int nextHitBeat = nextBeatToHit(currentBeat);
        bool hit = certainBeatHit(currentBeat, nextHitBeat, beatStates);
        if (hit) {
            lastHitBeat = nextHitBeat;
            relativeBeat = findRelativeBeat(lastHitBeat);
            return true;
        } 
        return false; 
    }

    int findRelativeBeat(int beat) {
        //relative beat will be 0 if we are at start, and at n if we have completed sequence of n beats
        if (lastHitBeat < startBeat) { return -1; }
        int rel = beat;
        rel -= startBeat;
        rel /= beatGap;
        return rel;
    }

    private:
    int startBeat;
    int numBeats;
    int beatGap;
    int endBeat;

    //DUPLICATED CODE, TODO: FIX
    int getBeatIndex(int beat, std::vector<playableBeatStatus> beatStates) {
        for (size_t i = 0; i < beatStates.size(); i++) {
            if (beatStates[i].beatStart == beat) {
                return i;
            }
        }
        return -1;
    }

    bool certainBeatHit(int beat, int checkBeat, std::vector<playableBeatStatus> beatStates) {
        int index = getBeatIndex(checkBeat, beatStates);
        if (index == -1) return false;

        playerStatus state = beatStates[index].playerState;
        if (beat == checkBeat - 1 || beat == checkBeat) { //if just before, exactly at, or just after window for beat
            if (state == CORRECT_HIT) {
                return true;
            }
        }

        return false;
    }

    bool reachedLastBeat(int currentBeat) {
        return currentBeat >= startBeat + numBeats * beatGap;
    }

    int nextBeatToHit(int currentBeat) {
        int next = lastHitBeat + beatGap;
        return next;
    }

    

    bool beatInSequenceMissed(std::vector<playableBeatStatus> beatStates) {
        //check ALL beats from start to end of sequence
        for (int checkBeat = startBeat; checkBeat <= endBeat; checkBeat += beatGap) {
            int index = getBeatIndex(checkBeat, beatStates);
            if (index == -1) { continue; }
            playerStatus state = beatStates[index].playerState;

            if (state == EARLY_HIT || state == MISS) {
                return true;
            }
        }

        return false;
    }

};

class FillTankAnimation : public AnimationCommand {
    int startBeat;
    int numBeats;
    int beatGap;
    Animator animator;
    MultiBeatStateTracker stateTracker;
    int timeAtHit;

    public:
    FillTankAnimation(int energy, int _startBeat, int _numBeats, int _beatGap) {
        startBeat = _startBeat;
        numBeats = _numBeats;
        beatGap = _beatGap;
        animator = Animator();
        stateTracker.init(startBeat, numBeats, beatGap);
        timeAtHit = 0;
        setEnergyLevel(energy);
    }

    void update(int beat, int progress, std::vector<playableBeatStatus> beatStates, Vec2d penPos) {

        bool newBeat = stateTracker.update(beat, beatStates);

        if ((stateTracker.killed || stateTracker.finished) || !stateTracker.started) { return; }

        if (newBeat) { 
            timeAtHit = beat * 100 + progress;
        }

        int now = beat * 100 + progress;
        animator.fillTank(stateTracker.relativeBeat, now - timeAtHit, numBeats, {0, 20, 20});

        if (stateTracker.relativeBeat == numBeats - 1) { //last beat
            //animate fun at spot
            int index = getBeatIndex(stateTracker.lastHitBeat, beatStates);
            if (index == -1) {return; }
            Vec2d pos = beatStates[index].startPos;
            int radius = 20 - progress / 10;
            animator.dancingStarfish(pos, progress, radius);
        }
        
    }

};

class SlidingStarfishAnimation : public AnimationCommand {
    int startBeat;
    int endBeat;
    Vec2d startPos;
    Vec2d endPos;
    Animator animator;
    Vec2d lastKnownPenPos;

    public:
    SlidingStarfishAnimation(int energy, int _startBeat) { //can customize this with specific parameters
        startBeat = _startBeat;
        animator = Animator();
        setEnergyLevel(energy);
    }

    void update(int beat, int progress, std::vector<playableBeatStatus> beatStates, Vec2d penPos) override {
        int index = getBeatIndex(startBeat, beatStates);
        if (index == -1) return;
        playerStatus state = beatStates[index].playerState;
        startPos = beatStates[index].startPos;
        endPos = beatStates[index].endPos;
        endBeat = beatStates[index].beatEnd;

        if (penPos.x > 0) { lastKnownPenPos = penPos; }

        if (state == playerStatus::SLIDER_HIT) {
            animator.slidingStarfish(beat, progress, startPos, endPos, lastKnownPenPos);
        } else if (state == playerStatus::SLIDER_END && beat < endBeat + 2) {
            animator.slidingStarfish(beat, progress, endPos, endPos, lastKnownPenPos);
            //animator.dancingStarfish(endPos, progress, beat);
        }
        
    }

};

class ThrowingBallAnimation : public AnimationCommand {
    int startBeat;
    int endBeat;
    Vec2d startPos;
    Vec2d sliderEndPos;
    Vec2d landPos;
    Animator animator;

    public:
    ThrowingBallAnimation(int energy, int _startBeat, Vec2d _landPos) { //can customize this with specific parameters
        startBeat = _startBeat;
        //endBeat = _endBeat;
        //bstartPos = _startPos;
        //sliderEndPos = _sliderEndPos;
        landPos = _landPos;
        animator = Animator();
        setEnergyLevel(energy);
    }

    void update(int beat, int progress, std::vector<playableBeatStatus> beatStates, Vec2d penPos) override {
        int index = getBeatIndex(startBeat, beatStates);
        if (index == -1) return;
        playerStatus state = beatStates[index].playerState;
        startPos = beatStates[index].startPos;
        sliderEndPos = beatStates[index].endPos;
        endBeat = beatStates[index].beatEnd;

        if (state == playerStatus::SLIDER_HIT) {
            animator.slidingCircle(startPos, sliderEndPos, penPos);
        } else if (state == playerStatus::SLIDER_END) {
            int animStart = convertBeatToTime(endBeat, 0);
            int animEnd = convertBeatToTime(endBeat + 2, 0);
            int currTime = convertBeatToTime(beat, progress);
            animator.flyingBall(animStart, animEnd, currTime, sliderEndPos, landPos, 10);
        }
        
    }

};

class ColourSliderAnimation : public AnimationCommand {
    int absoluteStartBeat;
    int absoluteEndBeat;
    int startBeat;
    int endBeat;
    Colour startColour;
    Colour endColour;
    Animator animator;
    int preBeats = 2;
    int postBeats = 2;
    bool killed = false;
    std::vector<int> startBeats;
    int startBeatIndex;
    Vec2d startPos;
    Vec2d endPos;
    Vec2d rectTop;
    Vec2d rectBottom;

    public:
    ColourSliderAnimation(int energy, std::vector<int> _startBeats, int _endBeat, Colour _startColour, Colour _endColour, Vec2d _rectTop, Vec2d _rectBottom) { //can customize this with specific parameters
        startColour = _startColour;
        endColour = _endColour;
        animator = Animator();
        killed = false;
        startBeats = _startBeats;
        startBeatIndex = 0;
        startBeat = startBeats[startBeatIndex];
        absoluteStartBeat = startBeats[0];
        absoluteEndBeat = _endBeat;
        rectTop = _rectTop;
        rectBottom = _rectBottom;
        setEnergyLevel(energy);
    }

    void update(int beat, int progress, std::vector<playableBeatStatus> beatStates, Vec2d penPos) override {
        if (beat < absoluteStartBeat - preBeats || beat > absoluteEndBeat + postBeats) {
            return;
        }

        if (beat < startBeat) { 
            vectorRect(rectTop.x, rectTop.y, rectBottom.x, rectBottom.y, startColour, ANIMATION_BG_LAYER);
            return; 
        }

        if (beat == startBeat) {
            //find related slider
            int index = getBeatIndex(startBeat, beatStates);
            endBeat = beatStates[index].beatEnd;
            startPos = beatStates[index].startPos;
            endPos = beatStates[index].endPos;
        }

        if (startBeat <= beat && beat <= endBeat) {
            //vectorRect(0, 0, 100, 100, endColour, 0);
            animator.colourChangeSlider(startPos.x, startPos.y, endPos.y, startColour, endColour, penPos, rectTop, rectBottom);
        }

        if (beat == endBeat) { //find next
            startBeatIndex += 1;
            startBeat = startBeats[startBeatIndex];
            Colour swap = startColour;
            startColour = endColour;
            endColour = swap;
        }
    }

    int getVal() override {
        return 0;
    }

};

class DiagonalBouncingBallAnimation : public AnimationCommand {
    private:
    int startBeat;
    int numBeats;
    int beatTimeDist;

    Animator animator; //does not actually need to be a class

    int currentBeat;
    int nextBeat;

    int currentBeatStartTime;
    int currentBeatEndTime;

    Vec2d currentBeatPos;
    Vec2d nextBeatPos;

    bool startPlaying;
    bool stopPlaying;
    bool killed;

    public:
    DiagonalBouncingBallAnimation(int energy, int _startBeat, int _numBeats, int _beatTimeDist) { //can customize this with specific parameters
        startBeat = _startBeat;
        numBeats = _numBeats;
        beatTimeDist = _beatTimeDist;
        animator = Animator();

        nextBeat = startBeat;
        currentBeat = -100;
        
        startPlaying = false;
        stopPlaying = false;
        killed = false;
        setEnergyLevel(energy);
    }

    //TODO: fix bug where having beatTimeDist > 2 means that a miss/early hit won't always be caught immediately
    //this can be done by refactoring to use new class
    // beatTimeDist = 2 works, beatTimeDist = 1 is untested

    void update(int beat, int progress, std::vector<playableBeatStatus> beatStates, Vec2d penPos) override { 
        //find next beat played in stream of beats
        //look at state of next beat(s) to see what to do with ball

        if (startPlaying && beat > startBeat + numBeats) { killed = true; }
        if (killed) { return; }
        
        if ((beat == nextBeat - 1 || beat == nextBeat) && !stopPlaying ) {
            //vectorRect(0, 0, 10, 10, {10, 20, 20}, 100);
            //find next hitBeat, should be at startbeat
            int currIndex = getBeatIndex(nextBeat, beatStates);
            if (currIndex == -1) { return; }

            int nextIndex = getBeatIndex(nextBeat + beatTimeDist, beatStates);

            playerStatus state = beatStates[currIndex].playerState;
            playerStatus nextState = beatStates[nextIndex].playerState;
            if (state == playerStatus::CORRECT_HIT) {
                startPlaying = true;

                currentBeat = nextBeat;
                nextBeat = currentBeat + beatTimeDist;

                currentBeatStartTime = convertBeatToTime(beat, progress);
                currentBeatEndTime = convertBeatToTime(nextBeat, 0);

                
                if (nextIndex == -1) {
                    Vec2d offset =  { (nextBeatPos.x - currentBeatPos.x), (nextBeatPos.y - currentBeatPos.y) };
                    nextBeatPos.x += offset.x;
                    nextBeatPos.y += offset.y;
                    stopPlaying = true;
                } else {
                    nextBeatPos = beatStates[nextIndex].startPos;
                }
                currentBeatPos = beatStates[currIndex].startPos;
               
            }

            if (state == playerStatus::EARLY_HIT || state == playerStatus::MISS
                || nextState == playerStatus::EARLY_HIT || nextState == playerStatus::MISS) {
                killed = true;
            }
        }

        if (!startPlaying) { return; }

        int currTime = convertBeatToTime(beat, progress);
        animator.bouncingBallDiagonal(currentBeatStartTime, currentBeatEndTime, currTime, currentBeatPos, nextBeatPos, 20);
    }

    int getVal() override {
        return killed;
    }
};

class BurstingBeatAnimation : public AnimationCommand {
    private:
    int startBeat;
    int endBeat;
    int preBeats = 4;
    int postBeats = 2;
    //Vec2d pos;
    Animator animator; //does not actually need to be a class
    int offset;

    public:
    BurstingBeatAnimation(int energy, int _startBeat, int _endBeat) { //can customize this with specific parameters
        startBeat = _startBeat;
        endBeat = _endBeat;
        //pos = _pos;
        animator = Animator();
        offset = 0;
        setEnergyLevel(energy);
    }

    void update(int beat, int progress, std::vector<playableBeatStatus> beatStates, Vec2d penPos) override {
        int index = getBeatIndex(startBeat, beatStates);
        if (index == -1) return;
        playerStatus state = beatStates[index].playerState;
        Vec2d pos = beatStates[index].startPos;

        if (state == playerStatus::IDLE || state == playerStatus::READY_TO_HIT) {
            animator.shakingObject(beat, progress, startBeat - preBeats, startBeat, pos);
        } else if (state == playerStatus::CORRECT_HIT) {
            animator.hitObject(pos);
            offset = getOffset(startBeat, 0, beat, progress);
        } else if (state == playerStatus::CORRECT_LIFT) {
            animator.burstingObject(beat, progress, endBeat, endBeat + postBeats, offset, pos);
        } 
    }
};

class DancingStarfishAnimation : public AnimationCommand {
    private:
    int startBeat;
    Animator animator; //does not actually need to be a class
    int timeAtLift;
    bool setTime;
    int shrinkTime = 200;

    public:
    DancingStarfishAnimation(int energy, int _startBeat) { //can customize this with specific parameters
        startBeat = _startBeat;
        timeAtLift = 0;
        setTime = false;
        setEnergyLevel(energy);
    }

    void update(int beat, int progress, std::vector<playableBeatStatus> beatStates, Vec2d penPos) override {
        int index = getBeatIndex(startBeat, beatStates);
        if (index == -1) return;
        playerStatus state = beatStates[index].playerState;
        Vec2d pos = beatStates[index].startPos;

        if (state == playerStatus::IDLE || state == playerStatus::READY_TO_HIT) {
            //animator.shakingObject(beat, progress, startBeat - preBeats, startBeat, pos);
        } else if (state == playerStatus::CORRECT_HIT) {
            
            //animator.hitObject(pos);
            animator.dancingStarfish(pos, progress, 20);
        } else if (state == playerStatus::CORRECT_LIFT) {
            if (!setTime) {
                timeAtLift = convertBeatToTime(beat, progress);
                setTime = true;
            }
            int t = inverseLerp(timeAtLift, timeAtLift + shrinkTime, convertBeatToTime(beat, progress));
            if (t >= 70) { return; }
            int radius = lerp(20, 5, t);
            animator.dancingStarfish(pos, progress, radius);
            //animator.burstingObject(beat, progress, endBeat, endBeat + postBeats, offset, pos);
        } 
    }
};

//this does not really need to be a class anymore actually
class AnimationCommandManager {
    //contains a list of animation commands, to be executed on beats
    private:
    std::vector<AnimationCommand*> animationCommands;
    //std::vector<BackgroundAnimationCommand*> backgroundAnimationCommands;
    int beatLookAhead = 2;

    public:
    AnimationCommandManager(std::vector<AnimationCommand*> _animationCommands) {
        animationCommands = _animationCommands;
    }

    void updateAnimations(songPosition pos, std::vector<playableBeatStatus> beatStates, Vec2d penPos, int energyLevel) {
        //ask commands to update themselves if their time is right
        int beat = pos.globalBeat * pos.numSubBeats + pos.subBeat;
        for (size_t i = 0; i < animationCommands.size(); i++) {
            if (animationCommands[i]->getEnergyLevel() > energyLevel) {
                Debugger::framePrint("skip %i", energyLevel);
                continue;
            }
            animationCommands[i]->update(beat, pos.subBeatProgress, beatStates, penPos);
        }
    }

};

#endif