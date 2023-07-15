#ifndef ANIMATION_COMMAND_C
#define ANIMATION_COMMAND_C

#include <vector>
#include <tuple>
#include "animator.cpp"
#include "mathHelpers.h"



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
    Vec2d startPos;
    int beatSpaceDist;
    Animator animator; //does not actually need to be a class
    int offset;
    int relativeBeat;
    bool killed;

    public:
    BouncingBallAnimation(int _startBeat, int _numBeats, int _beatTimeDist, Vec2d _startPos, int _beatSpaceDist) { //can customize this with specific parameters
        startBeat = _startBeat;
        numBeats = _numBeats;
        beatTimeDist = _beatTimeDist;
        startPos = _startPos;
        beatSpaceDist = _beatSpaceDist;
        animator = Animator();
        offset = 0;
        relativeBeat = -1;
        killed = false;
    }

    void update(int beat, int progress, std::vector<playableBeatStatus> beatStates, Vec2d penPos) override {

        //we want ball to stay where it is until state is hit
        //only check this on new beat change
        //TODO: instakill animation if any beat in sequence is pressed too early
        //TODO: figure out better way to handle animation if not pressed exactly on time (maybe system works via hit beat status and not time, could be more flexible too)
        //essentially rework entire function :)

        if (killed) { return; }

        if (beat >= startBeat && beat < startBeat + (numBeats * beatTimeDist)) {
            
            if ((beat - startBeat) % beatTimeDist == 0) {
                int newRelativeBeat;
                newRelativeBeat = (beat - startBeat) / beatTimeDist;
                if (newRelativeBeat == -1) { return; }

                int index = getBeatIndex(startBeat + newRelativeBeat * beatTimeDist, beatStates);
                if (index == -1) return;
                playerStatus state = beatStates[index].playerState;
                if (state == playerStatus::CORRECT_HIT || state == playerStatus::CORRECT_LIFT) { relativeBeat = newRelativeBeat; }
                if (state == playerStatus::MISS) { killed = true; }

                //int nextIndex = getBeatIndex(startBeat + (newRelativeBeat + 1) * beatTimeDist, beatStates);
                //if (index != -1) {
                //    state = beatStates[index].playerState;
                //    if (state == playerStatus::EARLY_HIT) { killed = true; }
                //}
            }
            
        }

        //if (relativeBeat == -1) { return; }

        int currBeat = startBeat + relativeBeat * beatTimeDist;
        if (beat > currBeat + beatTimeDist) { killed = true; }

        Vec2d currStart = startPos;
        currStart.x += beatSpaceDist * relativeBeat;
        Vec2d currEnd = currStart;
        currEnd.x += beatSpaceDist;

        int animStart = convertBeatToTime(currBeat, 0);
        int animEnd = convertBeatToTime(startBeat + (relativeBeat + 1) * beatTimeDist, 0);
        int currTime = convertBeatToTime(beat, progress);
        
        animator.bouncingBallDiagonal(animStart, animEnd, currTime, currStart, currEnd, 20);
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
        InteractiveAnimationCommand* c = new BouncingBallAnimation(0, 4, 2, {20, 20}, 30);
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

};

#endif