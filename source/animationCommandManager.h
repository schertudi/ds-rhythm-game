#include <vector>
#include "animationCommands.h"

class AnimationCommandManager {
    private:
    std::vector<AnimationCommand*> animationCommands;
    int beatLookAhead = 2;

    public:
    AnimationCommandManager(std::vector<AnimationCommand*> _animationCommands);

    void updateAnimations(songPosition pos, std::vector<playableBeatStatus> beatStates, Vec2d penPos, int energyLevel);

};