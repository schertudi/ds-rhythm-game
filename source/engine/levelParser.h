#pragma once
#include <utility>
#include "hitBeat.h"
#include "anim/animationCommands.h"

struct levelData {
	std::vector<HitBeatAbstract*> beatInteracts;
	std::vector<AnimationCommand*> animations;
	//should be able to lookup a bar and get a level. bar is seq of continous ints so can use array
	std::vector<int> perBarEnergyLevel;
};

namespace LevelDataParser {
    levelData setup(int numBeatsInBar);
}

