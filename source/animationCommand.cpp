#ifndef ANIMATION_COMMAND_C
#define ANIMATION_COMMAND_C

#include <vector>


#include "animator.cpp"



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
    private:
    int startBeat;
    int endBeat;
    int preBeats = 4;
    int postBeats = 2;
    Vec2d pos;
    Animator animator;
    int hitState = 0;
    int offset;
    bool kill;

    bool isTime(int beat) {
        return beat >= startBeat - preBeats && beat <= endBeat + postBeats;
    }

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

    public:
    InteractiveAnimationCommand(int _startBeat, int _endBeat, Vec2d _pos) { //can customize this with specific parameters
        startBeat = _startBeat;
        endBeat = _endBeat;
        pos = _pos;
        animator = Animator();
        hitState = 0;
        offset = 0;
        kill = false;
    }

    void update(int beat, int progress, std::vector<playableBeatStatus> beatStates) {
        //for an interactive animation, we want it to play the starting phase right up until the beat is hit
        //then we play active -> post animation, timing is offset by how far off player was
        //say note goes at beat 2, but we hit at beat 1 progress 80. so offset is -20% of a beat and active phase is done with those timings.
        //and if we hit at beat 2 progress 20, offset is +20%.
        //here progress is based on songPos.subBeatProgress, NOT beatStates.progress
        //when pen is lifted - we can choose to keep the active animation, or immediately skip to the post one.
        //if (beat < 0) return;
        //animator.burstingObject(beat, progress, 0, 2, -50, pos);
        //return;
        //animator.burstingObject(beat, progress, endBeat, endBeat + postBeats, -80, pos);
        //return;

        if (kill) return;
        if (!isTime(beat)) return;

        bool isHit = false; //if we don't find a corresponding beat (likely deleted) we default to false
        int interactBeatProgress = -1;
        for (size_t i = 0; i < beatStates.size(); i++) {
            if (beatStates[i].beatStart == startBeat) {
                isHit = beatStates[i].isHit;
                interactBeatProgress = beatStates[i].progress;
            }
        }

        if (interactBeatProgress > 200 && hitState == 0) { //missed beat
            kill = true;
            return;
        }

        if (hitState == 0 && isHit) { //pen just hit the beat
            hitState = 1;
            //set offset based on how far we are from (startBeat, 0)
            offset = getOffset(startBeat, 0, beat, progress);
            if (interactBeatProgress < 100) { //hit was too early, hide
                kill = true;
                return;
            }
        }

        if (hitState == 1 && !isHit) { //pen just lifted
            hitState = 2;
        }

        if (hitState == 0) { //play pre sequence
            animator.shakingObject(beat, progress, startBeat - preBeats, startBeat, pos);
        }
        if (hitState == 1) { //beat is active
            animator.hitObject(pos);
        }
        if (hitState == 2) { //play post sequence
            animator.burstingObject(beat, progress, endBeat, endBeat + postBeats, offset, pos);
            
        }
    }

    int getOffset() {
        return offset;
    }
};




class AnimationCommandManager {
    //contains a list of animation commands, to be executed on beats
    private:
    std::vector<InteractiveAnimationCommand> interactiveAnimationCommands;
    int beatLookAhead = 2;

    public:
    AnimationCommandManager() {
        //initialize animation commands
        interactiveAnimationCommands.push_back(InteractiveAnimationCommand(0, 0, {20, 20}));
        interactiveAnimationCommands.push_back(InteractiveAnimationCommand(2, 2, {50, 20}));
        interactiveAnimationCommands.push_back(InteractiveAnimationCommand(4, 4, {80, 20}));
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

    void updateInteractiveAnimations(songPosition pos, std::vector<playableBeatStatus> beatStates) {
        //ask commands to update themselves if their time is right
        int beat = pos.globalBeat * pos.numSubBeats + pos.subBeat;
        for (size_t i = 0; i < interactiveAnimationCommands.size(); i++) {
            interactiveAnimationCommands[i].update(beat, pos.subBeatProgress, beatStates);
        }
    }

    int getOffset() {
        return interactiveAnimationCommands[0].getOffset();
    }

};

#endif