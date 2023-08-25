#pragma once

#include "../../genericTypes.h"
#include "../engineTypes.h"

namespace Animator {

    void bouncingBallStraight(int progress, int beat, int numBeats);

    void bouncingBallDiagonal(int startTime, int endTime, int currTime, Vec2d startPos, Vec2d endPos, int height);

    void fillTank(int beat, int progress, int numBeats, Colour c);

    void sineWave(int beat, int progress, direction wallSide, Colour colour);

    int fullDancingStarfishHelper(int i, int smoothProgress, int radius, int shimmerRot);

    int partialDancingStarfishHelper(int i, int smoothProgress, int radius, int startAngle, int endAngle, int shimmerRot);

    void dancingStarfish(Vec2d pos, int progress, int radius, bool partial=false, int startAngle=0, int endAngle = 360);

    void slidingStarfish(int beat, int progress, Vec2d start, Vec2d end, Vec2d penPos);

    void slidingCircle(Vec2d start, Vec2d end, Vec2d penPos);

    void flyingBall(int startTime, int endTime, int currTime, Vec2d startPos, Vec2d endPos, int elevation);

    void colourChangeSlider(int x, int startY, int endY, Colour startC, Colour endC, Vec2d penPos, Vec2d rectTop, Vec2d rectBottom);

    void shakingObject(int beat, int progress, int startBeat, int endBeat, Vec2d pos);

    void hitObject(Vec2d pos);

    void burstingObject(int beat, int progress, int startBeat, int endBeat, int offset, Vec2d pos);

}