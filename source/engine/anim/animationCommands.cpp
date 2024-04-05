
#include <vector>
#include <tuple>
#include "animator.h"
#include "../../sysWrappers/mathHelpers.h"
#include "../../sysWrappers/vectorShapes.h"
#include "../../layerDefinitions.h"
#include "multiBeatStateTracker.h"

#include "animationCommands.h"


//all animations have a set energy level so variable is defined in base class
void AnimationCommand::setEnergyLevel(int l) {
    energyLevel = l;
}

int AnimationCommand::getEnergyLevel() {
    return energyLevel;
}

//helper functions used by animation commands
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


//animation command implementations

SineWaveAnimation::SineWaveAnimation(int energy, int _startBeat, int _endBeat, direction _wall) { //can customize this with specific parameters
    startBeat = _startBeat;
    endBeat = _endBeat;
    wallSide = _wall;
    setEnergyLevel(energy);
}

void SineWaveAnimation::update(int beat, int progress, std::vector<playableBeatStatus> beatStates, Vec2d penPos) {
    if (inStartPhase(beat, startBeat, preBeats) || inActivePhase(beat, startBeat, endBeat) || inEndPhase(beat, endBeat, postBeats) ) { 
        Animator::sineWave(beat, progress, wallSide, {10, 10, 10});
    }
}

FillTankAnimation::FillTankAnimation(int energy, int _startBeat, int _numBeats, int _beatGap) {
    startBeat = _startBeat;
    numBeats = _numBeats;
    beatGap = _beatGap;
    stateTracker.init(startBeat, numBeats, beatGap);
    timeAtHit = 0;
    setEnergyLevel(energy);
}

void FillTankAnimation::update(int beat, int progress, std::vector<playableBeatStatus> beatStates, Vec2d penPos) {

    bool newBeat = stateTracker.update(beat, beatStates);

    if ((stateTracker.killed || stateTracker.finished) || !stateTracker.started) { return; }

    if (newBeat) { 
        timeAtHit = beat * 100 + progress;
    }

    int now = beat * 100 + progress;
    Animator::fillTank(stateTracker.relativeBeat, now - timeAtHit, numBeats, {5, 5, 5});

    if (stateTracker.relativeBeat == numBeats - 1) { //last beat
        //animate fun at spot
        int index = getBeatIndex(stateTracker.lastHitBeat, beatStates);
        if (index == -1) {return; }
        Vec2d pos = beatStates[index].startPos;
        int radius = 20 - progress / 10;
        //Animator::dancingStarfish(pos, progress, radius);
    }
    
}


SlidingStarfishAnimation::SlidingStarfishAnimation(int energy, int _startBeat) { //can customize this with specific parameters
    startBeat = _startBeat;
    setEnergyLevel(energy);
}

void SlidingStarfishAnimation::update(int beat, int progress, std::vector<playableBeatStatus> beatStates, Vec2d penPos) {
    int index = getBeatIndex(startBeat, beatStates);
    if (index == -1) return;
    playerStatus state = beatStates[index].playerState;
    startPos = beatStates[index].startPos;
    endPos = beatStates[index].endPos;
    endBeat = beatStates[index].beatEnd;

    if (penPos.x > 0) { lastKnownPenPos = penPos; }

    if (state == playerStatus::SLIDER_HIT) {
        Animator::slidingStarfish(beat, progress, startPos, endPos, lastKnownPenPos);
    } else if (state == playerStatus::SLIDER_END && beat < endBeat + 2) {
        Animator::slidingStarfish(beat, progress, endPos, endPos, lastKnownPenPos);
        //Animator::dancingStarfish(endPos, progress, beat);
    }
    
}


ThrowingBallAnimation::ThrowingBallAnimation(int energy, int _startBeat, Vec2d _landPos, int time) { //can customize this with specific parameters
    startBeat = _startBeat;
    //endBeat = _endBeat;
    //bstartPos = _startPos;
    //sliderEndPos = _sliderEndPos;
    landPos = _landPos;
    setEnergyLevel(energy);
    throwTime = time;
}

void ThrowingBallAnimation::update(int beat, int progress, std::vector<playableBeatStatus> beatStates, Vec2d penPos) {
    int index = getBeatIndex(startBeat, beatStates);
    if (index == -1) return;
    playerStatus state = beatStates[index].playerState;
    startPos = beatStates[index].startPos;
    sliderEndPos = beatStates[index].endPos;
    endBeat = beatStates[index].beatEnd;

    if (state == playerStatus::SLIDER_HIT) {
        Animator::slidingCircle(startPos, sliderEndPos, penPos);
    } else if (state == playerStatus::SLIDER_END || state == playerStatus::CORRECT_HIT) {
        int animStart = convertBeatToTime(endBeat, 0);
        int animEnd = convertBeatToTime(endBeat + throwTime, 0);
        int currTime = convertBeatToTime(beat, progress);
        if (currTime > animEnd) {
            return;
        }
        Animator::flyingBall(animStart, animEnd, currTime, sliderEndPos, landPos, 10);
    }
    
}


ColourSliderAnimation::ColourSliderAnimation(int energy, std::vector<int> _startBeats, int _endBeat, Colour _startColour, Colour _endColour, Vec2d _rectTop, Vec2d _rectBottom) { //can customize this with specific parameters
    startColour = _startColour;
    endColour = _endColour;
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

void ColourSliderAnimation::update(int beat, int progress, std::vector<playableBeatStatus> beatStates, Vec2d penPos) {
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
        Animator::colourChangeSlider(startPos.x, startPos.y, endPos.y, startColour, endColour, penPos, rectTop, rectBottom);
    }

    if (beat == endBeat) { //find next
        startBeatIndex += 1;
        startBeat = startBeats[startBeatIndex];
        Colour swap = startColour;
        startColour = endColour;
        endColour = swap;
    }
}



DiagonalBouncingBallAnimation::DiagonalBouncingBallAnimation(int energy, int _startBeat, int _numBeats, int _beatTimeDist) { //can customize this with specific parameters
    startBeat = _startBeat;
    numBeats = _numBeats;
    beatTimeDist = _beatTimeDist;

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

void DiagonalBouncingBallAnimation::update(int beat, int progress, std::vector<playableBeatStatus> beatStates, Vec2d penPos) { 
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
            //killed = true; //TODO this triggers incorrectly sometimes (beat gap of 1). disable it for now, it just sits there
        }

        if (state == playerStatus::EARLY_HIT || state == playerStatus::MISS) {
            killed = true;
        }
    }

    if (!startPlaying) { return; }

    int currTime = convertBeatToTime(beat, progress);
    Animator::bouncingBallDiagonal(currentBeatStartTime, currentBeatEndTime, currTime, currentBeatPos, nextBeatPos, 20);
}


BurstingBeatAnimation::BurstingBeatAnimation(int energy, int _startBeat, int _endBeat) { //can customize this with specific parameters
    startBeat = _startBeat;
    endBeat = _endBeat;
    offset = 0;
    setEnergyLevel(energy);
}

void BurstingBeatAnimation::update(int beat, int progress, std::vector<playableBeatStatus> beatStates, Vec2d penPos) {
    int index = getBeatIndex(startBeat, beatStates);
    if (index == -1) return;
    playerStatus state = beatStates[index].playerState;
    Vec2d pos = beatStates[index].startPos;

    if (state == playerStatus::IDLE || state == playerStatus::READY_TO_HIT) {
        Animator::shakingObject(beat, progress, startBeat - preBeats, startBeat, pos);
    } else if (state == playerStatus::CORRECT_HIT) {
        Animator::hitObject(pos);
        offset = getOffset(startBeat, 0, beat, progress);
    } else if (state == playerStatus::CORRECT_LIFT) {
        Animator::burstingObject(beat, progress, endBeat, endBeat + postBeats, offset, pos);
    } 
}



DancingStarfishAnimation::DancingStarfishAnimation(int energy, int _startBeat) { //can customize this with specific parameters
    startBeat = _startBeat;
    timeAtLift = 0;
    setTime = false;
    setEnergyLevel(energy);
}

void DancingStarfishAnimation::update(int beat, int progress, std::vector<playableBeatStatus> beatStates, Vec2d penPos) {
    int index = getBeatIndex(startBeat, beatStates);
    if (index == -1) return;
    playerStatus state = beatStates[index].playerState;
    Vec2d pos = beatStates[index].startPos;

    if (state == playerStatus::CORRECT_HIT) {
        Animator::dancingStarfish(pos, progress, 20);
    } else if (state == playerStatus::CORRECT_LIFT) {
        if (!setTime) {
            timeAtLift = convertBeatToTime(beat, progress);
            setTime = true;
        }
        int t = inverseLerp(timeAtLift, timeAtLift + shrinkTime, convertBeatToTime(beat, progress));
        if (t >= 70) { return; }
        int radius = lerp(20, 5, t);
        Animator::dancingStarfish(pos, progress, radius);
    } 
}

PulsingCircleAnimation::PulsingCircleAnimation(int energy, int beat) {
    hitBeat = beat;
    setEnergyLevel(energy);
}

void PulsingCircleAnimation::update(int beat, int progress, std::vector<playableBeatStatus> beatStates, Vec2d penPos) {

    if (beat > hitBeat) { return; }
    int index = getBeatIndex(hitBeat, beatStates);
    if (index == -1) return;
    playerStatus state = beatStates[index].playerState;
    Vec2d pos = beatStates[index].startPos;

    int time = beat * 100 + progress;
    int hitTime = (hitBeat + 2) * 100;

    Animator::pulsingCircle(hitTime - 400, hitTime - 200, time, pos, 20);
    if (state == playerStatus::CORRECT_HIT) {
        
    }
    
}

PulseNextCircleAnimation::PulseNextCircleAnimation(int energy, int beat) {
    hitBeat = beat;
    setEnergyLevel(energy);
}

void PulseNextCircleAnimation::update(int beat, int progress, std::vector<playableBeatStatus> beatStates, Vec2d penPos) {
    if (playNext) {
        int time = beat * 100 + progress;
        int hitTime = (hitBeat + 2) * 100;
        if (time > hitTime) {return;}
        Animator::pulsingCircle(hitTime - 200, hitTime, time, pos, 30);
    }

    if (beat > hitBeat) { return; }
    int index = getBeatIndex(hitBeat, beatStates);
    if (index == -1) return;
    playerStatus state = beatStates[index].playerState;

    int nextIndex = getBeatIndex(hitBeat+2, beatStates);
    if (nextIndex == -1) return;
    pos = beatStates[nextIndex].startPos;

    if (state == playerStatus::CORRECT_HIT) {
        playNext = true;
    }
    
}

SlidingBallAnimation::SlidingBallAnimation(int energy, int beat, Vec2d to, int time) {
    hitBeat = beat;
    slideTo = to;
    slideTime = time;
    setEnergyLevel(energy);
}

void SlidingBallAnimation::update(int beat, int progress, std::vector<playableBeatStatus> beatStates, Vec2d penPos) {
    if (keepPlaying) {
        int time = beat * 100 + progress;
        int endTime = hitBeat * 100 + slideTime;
        if (time > endTime) { return; }
        Animator::slidingBall(hitBeat * 100, endTime, time, slideFrom, slideTo);
    }

    if (beat > hitBeat) { return; }
    int index = getBeatIndex(hitBeat, beatStates);
    if (index == -1) return;
    playerStatus state = beatStates[index].playerState;
    slideFrom = beatStates[index].startPos;


    if (state == playerStatus::CORRECT_HIT) {
        keepPlaying = true;
    }
    
}