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
    bool playAfterBeatKill;

    bool isTime(int beat) {
        return beat >= startBeat - preBeats && beat <= endBeat + postBeats;
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
        playAfterBeatKill = false;
    }

    void update(int beat, int progress, std::vector<playableBeatStatus> beatStates) {
        playerStatus state;
        bool found = false;
        for (size_t i = 0; i < beatStates.size(); i++) {
            if (beatStates[i].beatStart == startBeat) {
                state = beatStates[i].playerState;
                found = true;
                break;
            }
        }

        if (!found) return;
        //if (!isTime(beat)) return;

        if (state == playerStatus::IDLE || state == playerStatus::READY_TO_HIT) {
            animator.shakingObject(beat, progress, startBeat - preBeats, startBeat, pos);
        } else if (state == playerStatus::CORRECT_HIT) {
            animator.hitObject(pos);
            offset = getOffset(startBeat, 0, beat, progress);
        } else if (state == playerStatus::CORRECT_LIFT) {
            playAfterBeatKill = true;
            animator.burstingObject(beat, progress, endBeat, endBeat + postBeats, offset, pos);
        } 
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

};

#endif