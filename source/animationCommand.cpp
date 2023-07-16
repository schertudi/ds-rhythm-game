#ifndef ANIMATION_COMMAND_C
#define ANIMATION_COMMAND_C

#include <vector>
#include <tuple>
#include "animator.cpp"
#include "mathHelpers.h"
#include "vectorShapes.h"



class BackgroundAnimationCommand {
    private:
    int startBeat;
    int endBeat;
    int preBeats = 4;
    int postBeats = 2;
    Vec2d pos;
    Animator animator;

    bool inStartPhase(int beat, int startBeat, int preBeats) {
        return beat >= startBeat - preBeats && beat < startBeat;
    }

    bool inActivePhase(int beat, int startBeat, int endBeat) {
        return beat >= startBeat && beat <= endBeat;
    }

    bool inEndPhase(int beat, int endBeat, int postBeats) {
        return beat > endBeat && beat <= endBeat + postBeats;
    }

    public:
    BackgroundAnimationCommand(int _startBeat, int _endBeat, Vec2d _pos) { //can customize this with specific parameters
        startBeat = _startBeat;
        endBeat = _endBeat;
        pos = _pos;
        animator = Animator();
    }

    void update(int beat, int progress) {
        if (inStartPhase(beat, startBeat, preBeats)) { //play pre sequence
            animator.shakingObject(beat, progress, startBeat - preBeats, startBeat, pos);
        }
        if (inActivePhase(beat, startBeat, endBeat)) { //beat is active
            //animator.dancingStarfish(pos, progress, beat);
            animator.hitObject(pos);
        }
        if (inEndPhase(beat, endBeat, postBeats)) { //play post sequence
            animator.burstingObject(beat, progress, endBeat + 1, endBeat + postBeats + 1, 0, pos);
        }
    }
};

class InteractiveAnimationCommand {
    protected:

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

    public:

    virtual void update(int beat, int progress, std::vector<playableBeatStatus> beatStates, Vec2d penPos) = 0;

    virtual int getVal();
};

class SlidingStarfishAnimation : public InteractiveAnimationCommand {
    int startBeat;
    int endBeat;
    Vec2d startPos;
    Vec2d endPos;
    Animator animator;
    Vec2d lastKnownPenPos;

    public:
    SlidingStarfishAnimation(int _startBeat, int _endBeat, Vec2d _startPos, Vec2d _endPos) { //can customize this with specific parameters
        startBeat = _startBeat;
        endBeat = _endBeat;
        startPos = _startPos;
        endPos = _endPos;
        animator = Animator();
    }

    void update(int beat, int progress, std::vector<playableBeatStatus> beatStates, Vec2d penPos) override {
        int index = getBeatIndex(startBeat, beatStates);
        if (index == -1) return;
        playerStatus state = beatStates[index].playerState;

        if (penPos.x > 0) { lastKnownPenPos = penPos; }

        if (state == playerStatus::SLIDER_HIT) {
            animator.slidingStarfish(beat, progress, startPos, endPos, lastKnownPenPos);
        } else if (state == playerStatus::SLIDER_END && beat < endBeat + 2) {
            animator.slidingStarfish(beat, progress, endPos, endPos, lastKnownPenPos);
            //animator.dancingStarfish(endPos, progress, beat);
        }
        
    }

};

class ThrowingBallAnimation : public InteractiveAnimationCommand {
    int startBeat;
    int endBeat;
    Vec2d startPos;
    Vec2d sliderEndPos;
    Vec2d landPos;
    Animator animator;

    public:
    ThrowingBallAnimation(int _startBeat, int _endBeat, Vec2d _startPos, Vec2d _sliderEndPos, Vec2d _landPos) { //can customize this with specific parameters
        startBeat = _startBeat;
        endBeat = _endBeat;
        startPos = _startPos;
        sliderEndPos = _sliderEndPos;
        landPos = _landPos;
        animator = Animator();
    }

    void update(int beat, int progress, std::vector<playableBeatStatus> beatStates, Vec2d penPos) override {
        int index = getBeatIndex(startBeat, beatStates);
        if (index == -1) return;
        playerStatus state = beatStates[index].playerState;

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

class BouncingBallAnimation : public InteractiveAnimationCommand {
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
    BouncingBallAnimation(int _startBeat, int _numBeats, int _beatTimeDist) { //can customize this with specific parameters
        startBeat = _startBeat;
        numBeats = _numBeats;
        beatTimeDist = _beatTimeDist;
        animator = Animator();

        nextBeat = startBeat;
        currentBeat = -100;
        
        startPlaying = false;
        stopPlaying = false;
        killed = false;
    }

    //TODO: fix bug where having beatTimeDist > 2 means that a miss/early hit won't always be caught immediately

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

class BurstingBeatAnimation : public InteractiveAnimationCommand {
    private:
    int startBeat;
    int endBeat;
    int preBeats = 4;
    int postBeats = 2;
    Vec2d pos;
    Animator animator; //does not actually need to be a class
    int offset;

    public:
    BurstingBeatAnimation(int _startBeat, int _endBeat, Vec2d _pos) { //can customize this with specific parameters
        startBeat = _startBeat;
        endBeat = _endBeat;
        pos = _pos;
        animator = Animator();
        offset = 0;
    }

    void update(int beat, int progress, std::vector<playableBeatStatus> beatStates, Vec2d penPos) override {
        int index = getBeatIndex(startBeat, beatStates);
        if (index == -1) return;
        playerStatus state = beatStates[index].playerState;

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




class AnimationCommandManager {
    //contains a list of animation commands, to be executed on beats
    private:
    std::vector<InteractiveAnimationCommand*> interactiveAnimationCommands;
    int beatLookAhead = 2;

    public:
    AnimationCommandManager() {
        //initialize animation commands
        InteractiveAnimationCommand* a = new BurstingBeatAnimation(0, 0, {20, 20});
        InteractiveAnimationCommand* b = new BurstingBeatAnimation(2, 2, {50, 20});
        //InteractiveAnimationCommand* c = new ThrowingBallAnimation(2, 6, {20, 20}, {80, 20}, {150, 80});
        //InteractiveAnimationCommand* c = new SlidingStarfishAnimation(2, 6, {20, 20}, {80, 20});
        InteractiveAnimationCommand* c = new BouncingBallAnimation(0, 6, 2);
        //interactiveAnimationCommands.push_back(a);
        //interactiveAnimationCommands.push_back(b);
        interactiveAnimationCommands.push_back(c);
    }

    /*
    void updateBackgroundAnimations(songPosition pos) {
        //ask commands to update themselves if their time is right
        int beat = pos.globalBeat * pos.numSubBeats + pos.subBeat - 2 * pos.numSubBeats;
        for (size_t i = 0; i < bgAnimationCommands.size(); i++) {
            bgAnimationCommands[i].update(beat, pos.subBeatProgress);
        }
    }
    */

    void updateInteractiveAnimations(songPosition pos, std::vector<playableBeatStatus> beatStates, Vec2d penPos) {
        //ask commands to update themselves if their time is right
        int beat = pos.globalBeat * pos.numSubBeats + pos.subBeat;
        for (size_t i = 0; i < interactiveAnimationCommands.size(); i++) {
            interactiveAnimationCommands[i]->update(beat, pos.subBeatProgress, beatStates, penPos);
        }
    }

    int getVal() {
        return interactiveAnimationCommands[0]->getVal();
    }

};

#endif