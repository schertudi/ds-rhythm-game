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
            animator.burstingObject(beat, progress, endBeat + 1, endBeat + postBeats + 1, pos);
        }
    }
};




class AnimationCommandManager {
    //contains a list of animation commands, to be executed on beats
    private:
    std::vector<BackgroundAnimationCommand> animationCommands;
    int beatLookAhead = 2;

    public:
    AnimationCommandManager() {
        //initialize animation commands
        animationCommands.push_back(BackgroundAnimationCommand(0, 0, {20, 50}));
        animationCommands.push_back(BackgroundAnimationCommand(2, 2, {50, 50}));
        animationCommands.push_back(BackgroundAnimationCommand(4, 4, {80, 50}));
    }

    void updateAnimations(songPosition pos) {
        //ask commands to update themselves if their time is right
        int beat = pos.globalBeat * pos.numSubBeats + pos.subBeat - 2 * pos.numSubBeats;
        for (size_t i = 0; i < animationCommands.size(); i++) {
            animationCommands[i].update(beat, pos.subBeatProgress);
        }
    }

};

#endif