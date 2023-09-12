#pragma once

#include <vector>
#include "../engineTypes.h"
#include "../../genericTypes.h"
#include "multiBeatStateTracker.h"

class AnimationCommand {

private:
    int energyLevel;

protected:
    void setEnergyLevel(int l);

public:
    int getEnergyLevel();
    virtual void update(int beat, int progress, std::vector<playableBeatStatus> beatStates, Vec2d penPos) = 0;
};

class SineWaveAnimation : public AnimationCommand {
    private:
    int startBeat;
    int endBeat;
    int preBeats = 2;
    int postBeats = 2;
    direction wallSide;

    public:
    SineWaveAnimation(int energy, int _startBeat, int _endBeat, direction _wall);
    virtual void update(int beat, int progress, std::vector<playableBeatStatus> beatStates, Vec2d penPos);
};


class FillTankAnimation : public AnimationCommand {
    int startBeat;
    int numBeats;
    int beatGap;
    MultiBeatStateTracker stateTracker;
    int timeAtHit;

    public:
    FillTankAnimation(int energy, int _startBeat, int _numBeats, int _beatGap);
    virtual void update(int beat, int progress, std::vector<playableBeatStatus> beatStates, Vec2d penPos);

};

class SlidingStarfishAnimation : public AnimationCommand {
    int startBeat;
    int endBeat;
    Vec2d startPos;
    Vec2d endPos;
    Vec2d lastKnownPenPos;

    public:
    SlidingStarfishAnimation(int energy, int _startBeat);
    virtual void update(int beat, int progress, std::vector<playableBeatStatus> beatStates, Vec2d penPos);
};

class ThrowingBallAnimation : public AnimationCommand {
    int startBeat;
    int endBeat;
    Vec2d startPos;
    Vec2d sliderEndPos;
    Vec2d landPos;
    int throwTime;

    public:
    ThrowingBallAnimation(int energy, int _startBeat, Vec2d _landPos, int time);
    virtual void update(int beat, int progress, std::vector<playableBeatStatus> beatStates, Vec2d penPos);

};

class ColourSliderAnimation : public AnimationCommand {
    int absoluteStartBeat;
    int absoluteEndBeat;
    int startBeat;
    int endBeat;
    Colour startColour;
    Colour endColour;
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
    ColourSliderAnimation(int energy, std::vector<int> _startBeats, int _endBeat, Colour _startColour, Colour _endColour, Vec2d _rectTop, Vec2d _rectBottom);
    virtual void update(int beat, int progress, std::vector<playableBeatStatus> beatStates, Vec2d penPos);
};

class DiagonalBouncingBallAnimation : public AnimationCommand {
    private:
    int startBeat;
    int numBeats;
    int beatTimeDist;

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
    DiagonalBouncingBallAnimation(int energy, int _startBeat, int _numBeats, int _beatTimeDist);
    virtual void update(int beat, int progress, std::vector<playableBeatStatus> beatStates, Vec2d penPos);
};

class BurstingBeatAnimation : public AnimationCommand {
    private:
    int startBeat;
    int endBeat;
    int preBeats = 4;
    int postBeats = 2;
    int offset;

    public:
    BurstingBeatAnimation(int energy, int _startBeat, int _endBeat);
    virtual void update(int beat, int progress, std::vector<playableBeatStatus> beatStates, Vec2d penPos);
};

class DancingStarfishAnimation : public AnimationCommand {
    private:
    int startBeat;
    int timeAtLift;
    bool setTime;
    int shrinkTime = 200;

    public:
    DancingStarfishAnimation(int energy, int _startBeat);
    virtual void update(int beat, int progress, std::vector<playableBeatStatus> beatStates, Vec2d penPos);
};

class PulsingCircleAnimation : public AnimationCommand {
    private:
    int hitBeat;
    int pulseTime = 300;

    public:
    PulsingCircleAnimation(int energy, int beat);
    virtual void update(int beat, int progress, std::vector<playableBeatStatus> beatStates, Vec2d penPos);
};

class PulseNextCircleAnimation : public AnimationCommand {
    private:
    int hitBeat;
    bool playNext = false;
    Vec2d pos;

    public:
    PulseNextCircleAnimation(int energy, int beat);
    virtual void update(int beat, int progress, std::vector<playableBeatStatus> beatStates, Vec2d penPos);
};

class SlidingBallAnimation : public AnimationCommand {
    private:
    int hitBeat;
    Vec2d slideFrom;
    Vec2d slideTo;
    int slideTime;
    bool keepPlaying = false;

    public:
    SlidingBallAnimation(int energy, int beat, Vec2d to, int time);
    virtual void update(int beat, int progress, std::vector<playableBeatStatus> beatStates, Vec2d penPos);
};