#include "animationCommandManager.h"

AnimationCommandManager::AnimationCommandManager(std::vector<AnimationCommand*> _animationCommands) {
    animationCommands = _animationCommands;
}

void AnimationCommandManager::updateAnimations(songPosition pos, std::vector<playableBeatStatus> beatStates, Vec2d penPos, int energyLevel) {
    //ask commands to update themselves if their time is right
    int beat = pos.globalBeat * pos.numSubBeats + pos.subBeat;
    for (size_t i = 0; i < animationCommands.size(); i++) {
        if (animationCommands[i]->getEnergyLevel() > energyLevel) {
            continue;
        }
        animationCommands[i]->update(beat, pos.subBeatProgress, beatStates, penPos);
    }
}
