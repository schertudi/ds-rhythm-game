#ifndef ANIMATOR_H
#define ANIMATOR_H

#include <nds.h>
#include <nds/arm9/console.h>
#include <stdio.h>
#include <gl2d.h>
#include <tuple>
#include "vscode_fix.h"
#include "vectorShapes.h"

#include "constants.h"
#include "mathHelpers.h"


class Animator {

    public: 
    Animator() {

    }

    void bouncingBallStraight(int progress, int beat, int numBeats) { //at 0 we are furthest, 50 we hit wall, 100 we are furthest again
        beat = beat % numBeats;
        //map progress so that beat 0 progress 0 -> 0, beat numBeat progress 100 -> 100
        //put progress in range of 0 to 100 * numBeats
        if (beat > 0) {
            progress += beat * 100;
        }
        //normalize
        progress /= numBeats;

        Vec2d start = {100, 100};
        Vec2d hit = {50, 100};
        //move between start and hit depending on progress
        int prog = 0;
        if (progress < 50) {
            //0 maps to 0, 50 maps to 100
            prog = progress * 2;
        } else {
            //have values in range of 50 to 100, want to flip it so 50 maps to 100 and 100 maps to 0
            int t = inverseLerp(50, 100, progress);
            prog = lerp(100, 0, t);
        }

        //bias towards 100; speedup when hitting wall
        prog = ((prog * prog / 10) + (prog * 10)) / 20;

        int x = lerp(start.x, hit.x, prog);
        int y = lerp(start.y, hit.y, prog);
        vectorCircle(x, y, 10, {31, 31, 31}, ANIMATION_FG_LAYER);
    }

    void bouncingBallDiagonal(int progress, int beat) { //beat 0 start and then we offset from there
        Vec2d start = {20, 100};
        Vec2d hit = {50, 80};
        Vec2d end = {80, 100};
        int xOffset = beat * (end.x - start.x);
        start.x += xOffset;
        hit.x += xOffset;
        end.x += xOffset;

        int prog = 0;
        int x, y;
        if (progress < 50) {
            //0 maps to 0, 50 maps to 100
            prog = progress * 2;
            //want speedup when closer to 0
            int p = 100 - prog;
            p = ((p * p / 10) + (p * 10)) / 20;
            prog = 100 - p;

            x = lerp(start.x, hit.x, prog);
            y = lerp(start.y, hit.y, prog);
            
        } else {
            //have values in range of 50 to 100, want to flip it so 50 maps to 100 and 100 maps to 0
            prog = (progress - 50) * 2;
            //speedup when closest to 100
            prog = prog * prog / 100;
            
            x = lerp(hit.x, end.x, prog);
            y = lerp(hit.y, end.y, prog);
        }

        vectorCircle(x, y, 10, {31, 31, 31}, ANIMATION_FG_LAYER);
    }

    void fillTank(int progress, int beat, int numBeats) {
        beat = beat % numBeats;
        if (beat < numBeats - 2) {
            int y = (SCREEN_HEIGHT / numBeats * (numBeats - beat));
            vectorRect(0, y, SCREEN_WIDTH, SCREEN_HEIGHT, {0, 31, 25}, ANIMATION_BG_LAYER);
        } else if (beat < numBeats - 1) {
            int y = (SCREEN_HEIGHT / numBeats * (numBeats - beat));
            vectorRect(0, y, SCREEN_WIDTH, SCREEN_HEIGHT, {0, 31, 15}, ANIMATION_BG_LAYER);
        } else {
            //animate it being drained; at progress = 0 we are at min y and at progress = 0 we are at SCREEN_HEIGHT
            //if (progress < 50) progress = 0;
            int y = lerp((SCREEN_HEIGHT / numBeats * (numBeats - beat + 1)), SCREEN_HEIGHT, progress);
            vectorRect(0, y, SCREEN_WIDTH, SCREEN_HEIGHT, {0, 31, 15}, ANIMATION_BG_LAYER);
        }
    }

    void sineWave(int progress, int beat, int numBeats, int wall, bool invert=false) { //wall=1: top; wall=2: bottom
        glBegin2D();

        int curveDepth = cosLerp((progress * 32767) / 100) / 500;

        //draw a filled circle using triangles
        int i;
        //int curveDepth = 10 + (progress / 6);
        int curveWidth = SCREEN_WIDTH / 4;
        int yOffset = 20;
        int xOffset = 0;

        if (wall == 2) yOffset = SCREEN_HEIGHT - yOffset;
        if (invert) curveDepth = -curveDepth;

        for( i = 0; i < SCREEN_WIDTH; i += 1)
        {
            int x = i;
            int y = yOffset + ((sinLerp((i - xOffset) * curveWidth) * (curveDepth) ) >> 12);

            int x2 = (i + 1);
            int y2 = yOffset + ((sinLerp(((i - xOffset) + 1) * curveWidth) * (curveDepth) ) >> 12);

            int pivotY = 0;
            if (wall == 2) pivotY = SCREEN_HEIGHT;

            // draw a triangle
            glTriangleFilled(x, y,
                            x2, y2,
                            x, pivotY,
                            RGB15(21, 0, 21), ANIMATION_BG_LAYER);
        }
                
        glEnd2D();
    }

    int fullDancingStarfishHelper(int i, int smoothProgress, int radius, int shimmerRot) {
        int shimmerDensity = 7;
        int shimmerWidth = 4 + smoothProgress;
        int r = radius + ((cosLerp(i * shimmerDensity + shimmerRot) * shimmerWidth) >> 12);
        r = r * r * r / 1000 + 10;
        return r;
    }

    int partialDancingStarfishHelper(int i, int smoothProgress, int radius, int startAngle, int endAngle, int shimmerRot) {
        int shimmerDensity = 10;
        int shimmerWidth = 5 + smoothProgress;

        int r = radius / 2 + smoothProgress / 3;

        int currAngle = angleToDegrees(i);

        bool betweenIfStartSmall = currAngle > startAngle && currAngle < endAngle;
        bool betweenIfStartBig = currAngle > startAngle || currAngle < endAngle;
        
        if ( (startAngle < endAngle && betweenIfStartSmall) || (startAngle > endAngle && betweenIfStartBig) ) { 
            //smooth out effect so it fades to nothing near angle extents
            int rx = radius + ((cosLerp(i * shimmerDensity + shimmerRot) * shimmerWidth) >> 12);
            rx = rx * rx * rx / 1000 + 10;

            //find dist from curr to start (account for wrap)
            //find dist between start and end 
            //can then map as a % where 0 is you are at start and 100 means you are at end
            //convert so that it goes from 0 to 100 back to 0 and not 0 to 100
            //use that to lerp

            int distToStart = shortAngleDist(startAngle, currAngle);
            int startEndDist = shortAngleDist(startAngle, endAngle);
            int perc = (distToStart * 100) / startEndDist;
            if (perc > 50) perc = 100 - perc;
            perc *= 2;

            r = lerp(r, rx, perc);
            return r;

        }

        return r;
    }

    void dancingStarfish(Vec2d pos, int progress, int beat, bool partial=false, int startAngle=0, int endAngle = 360) {
        int smoothProgress = sinLerp((progress * 32767) / 100) / 1000;
        if (smoothProgress < 0) smoothProgress /= 2;
       
        int radius = 20;
        int shimmerRot = 0;
        if (!partial) shimmerRot = (progress - 50) * (32767) / 100;

        int xOrigin = pos.x;
        int yOrigin = pos.y;

        int step = 256;

        startAngle = normalizeAngle(startAngle);
        endAngle = normalizeAngle(endAngle);

        glBegin2D();

        //draw a filled circle using triangles
        int i;
        int r; 
        int r2;

        for( i = 0; i < 0 + BRAD_PI*2; i += step)
        {
            
            if (!partial) r = fullDancingStarfishHelper(i, smoothProgress, radius, shimmerRot);
            else r = partialDancingStarfishHelper(i, smoothProgress, radius, startAngle, endAngle, shimmerRot);
            
            int x = (cosLerp(i) * (r) ) >> 12;
            int y = (sinLerp(i) * (r) ) >> 12;

            if (!partial) r2 = fullDancingStarfishHelper(i + step, smoothProgress, radius, shimmerRot);
            else r2 = partialDancingStarfishHelper(i + step, smoothProgress, radius, startAngle, endAngle, shimmerRot);

            int x2 = (cosLerp(i + step) * (r2) ) >> 12;
            int y2 = (sinLerp(i + step) * (r2) ) >> 12;

            // draw a triangle
            glTriangleFilled( xOrigin + x, yOrigin + y,
                            xOrigin + x2, yOrigin + y2,
                            xOrigin, yOrigin,
                            RGB15(10, 30, 10), ANIMATION_FG_LAYER);
        }
                
        glEnd2D();
    }

    void slidingStarfish(Vec2d start, Vec2d end, Vec2d penPos, int progress, int beat) {
        Vec2d lineDir = {end.x - start.x, end.y - start.y};
        lineDir = normalizeVec(lineDir);

        int startThickness = 4;
        int endThickness = 10;

        Vec2d farStart = {start.x - (lineDir.x * startThickness) / 100, start.y - (lineDir.y * startThickness) / 100};

        int angleRad = intAtan2(penPos.y - farStart.y, penPos.x - farStart.x);
        int angleDeg = angleRad * 180 / 314;
        angleDeg += 180;
        
        std::tuple<Vec2d, int> proj = projectPointToLine(start, end, penPos);
        Vec2d starfishPos = std::get<0>(proj);
        int lineProgress = std::get<1>(proj);

        int width = lerp(startThickness, endThickness, lineProgress);

        vectorWideningLine(start.x, start.y, starfishPos.x, starfishPos.y, startThickness, width, {10, 10, 10}, ANIMATION_MG_LAYER);

        dancingStarfish(starfishPos, progress, beat, true, angleDeg - 80, angleDeg + 80);
    }

    void slidingCircle(Vec2d start, Vec2d end, Vec2d penPos) {
        std::tuple<Vec2d, int> proj = projectPointToLine(start, end, penPos);
        Vec2d circlePos = std::get<0>(proj);
        vectorThickLine(start.x, start.y, end.x, end.y, 2, {10, 10, 10}, ANIMATION_FG_LAYER);
        vectorCircle(circlePos.x, circlePos.y, 10, {31, 31, 31}, ANIMATION_FG_LAYER);
    }

    void flyingBall(int progress, int beat, Vec2d start, Vec2d end, int elevation) {

        Vec2d peak = {(start.x + end.x) / 2, start.y - elevation}; //highest point; gradient = 0
        
        //calc middle control point - https://stackoverflow.com/questions/22237780/how-to-model-quadratic-equation-using-a-bezier-curve-calculate-control-point
        int midX = 2*peak.x -start.x/2 -end.x/2;
        int midY = 2*peak.y -start.y/2 -end.y/2;

        int time = progress + beat * 100;
        int endBeat = 200;
        //lerp time to fit in range of startX to endX
        int t = inverseLerp(0, endBeat, time);
        if (t > 100) t = 100;

        Vec2d point = threePointBezier(start, {midX, midY}, end, t);
        //vectorCircle(start.x, start.y, 5, {10, 31, 31});
        //vectorCircle(end.x, end.y, 5, {31, 31, 10});
        vectorCircle(point.x, point.y, 10, {31, 31, 31}, ANIMATION_FG_LAYER);

    }

    void colourChangeSlider(int x, int startY, int endY, Colour startC, Colour endC, Vec2d penPos) {
        Vec2d sliderPos = {x, startY};
        
        if (startY < endY) {
            if (penPos.y < startY) sliderPos.y = startY;
            else if (penPos.y > endY) sliderPos.y = endY;
            else sliderPos.y = penPos.y;
        } else {
            if (penPos.y > startY) sliderPos.y = startY;
            else if (penPos.y < endY) sliderPos.y = endY;
            else sliderPos.y = penPos.y;
        }
        
        int lerp = inverseLerp(startY, endY, sliderPos.y);
        Colour c = lerpColour(startC, endC, lerp);

        vectorRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, c, ANIMATION_BG_LAYER);

        vectorRect(x - 2, startY, x + 2, endY, {5, 5, 5}, ANIMATION_FG_LAYER);

        vectorRect(sliderPos.x - 10, sliderPos.y - 5, sliderPos.x + 10, sliderPos.y + 5, {5, 5, 5}, ANIMATION_FG_LAYER);
    }

    void shakingObject(int beat, int progress) {
        int time = progress + beat * 100;
        int endBeat = 200;

        int x = SCREEN_WIDTH / 2;
        int y = SCREEN_HEIGHT / 2;

        int numShakesInBeat = 2;
        int shakeWidth = 5; //move 5px at most

        int shakeX = progress % (100 / numShakesInBeat); 
        int t = inverseLerp(0, (100 / numShakesInBeat), shakeX);
        shakeX = lerp(0, shakeWidth * 2, t);
        if (shakeX > shakeWidth) shakeX = shakeWidth * 2 - shakeX;
        
        int shakeY = sinLerp(progress * 600) / 1500;

        int startSize = 10;
        int endSize = 12;
        t = inverseLerp(0, endBeat, time);
        if (t > 100) t = 100;
        int size = lerp(startSize, endSize, t);

        vectorCircle(x + shakeX, y + shakeY, size, {31, 31, 31}, ANIMATION_FG_LAYER);
    }

    void hitObject(int progress) {
        int baseX = SCREEN_WIDTH / 2;
        int baseY = SCREEN_HEIGHT / 2;
        vectorCircle(baseX, baseY, 17, {31, 31, 31}, ANIMATION_FG_LAYER);
    }

    void burstingObject(int beat, int progress) {
        int numParticles = 5;
        int time = progress + beat * 100;
        int numBeats = 1;
        //draw 5 small circles around a larger circle
        int baseX = SCREEN_WIDTH / 2;
        int baseY = SCREEN_HEIGHT / 2;
        int particleRadius = 5;
        int startRadius = 2;
        int endRadius = 50;
        int objStartRadius = 10;
        int objEndRadius = 0;

        int t = inverseLerp(0, numBeats * 100, time);
        if (t >= 100) return;
        int posRadius = lerp(startRadius, endRadius, t);
        int rot = degreesToAngle(t * 180 / 100);
        if (t > 70) particleRadius = 4;
        if (t > 80) particleRadius = 3;
        if (t > 90) particleRadius = 2;

        int i;

        for (i = 0; i < numParticles; i++) {
            int angle = i * (BRAD_PI * 2 / numParticles);
            int partX = (cosLerp(angle + rot) * (posRadius) ) >> 12;
            int partY = (sinLerp(angle + rot) * (posRadius) ) >> 12;

            vectorCircle(baseX + partX, baseY + partY, particleRadius, {31, 31, 31}, ANIMATION_MG_LAYER);
        }

        int objRadius = lerp(objStartRadius, objEndRadius, (t * t) / 100);
        vectorCircle(baseX, baseY, objRadius, {31, 31, 31}, ANIMATION_FG_LAYER);


    }
};

class ThrowableObject { //persists after animation is done, can be interacted with by pen
    int x;
    int y;
    Vec2d velTotal;
    Vec2d vel;
    int velCount;
    bool useVel;
    bool isPenDown;
    bool useGravity = true;
    int gravity;
    Vec2d targetPos = {SCREEN_WIDTH / 2, SCREEN_HEIGHT / 4 * 3};
    int targetBeat = 5;

    public:
    ThrowableObject(int x, int y) {
        this->x = x;
        this->y = y;
        useVel = false;
        isPenDown = false;
        resetVelocity();
    }

    void resetVelocity() {
        velTotal = {0, 0};
        vel = {0, 0};
        velCount = 0;
        gravity = 0;
    }

    void draw(int frame, int beat, int progress) {
        vectorCircle(x, y, 10, {31, 31, 31}, ANIMATION_FG_LAYER);

        if ((!isPenDown)) {
            x += vel.x;
            y += vel.y;

            if (useGravity) {
                if (frame % 3 == 0 && beat != targetBeat - 1) {
                    if (gravity > -5) gravity -= 1;
                }
                y -= gravity;
            }
        }

        if (beat == targetBeat - 1) {
            //lerp to target based on progress
            x = lerp(x, targetPos.x, progress);
            y = lerp(y, targetPos.y, progress);
        }

        if (beat >= targetBeat) {
            x = targetPos.x;
            y = targetPos.y;
        }
    }

    void penDown(int beat, int _x, int _y) {
        if (beat < targetBeat) goTo(_x, _y);
        if (isPenDown) return;
        
        isPenDown = true;
        resetVelocity();
        useVel = false;
    }

    void penUp() {
        if (!isPenDown) return;
        isPenDown = false;
        useVel = true;
    }

    void goTo(int _x, int _y) {
        int oldX = x;
        int oldY = y;
        x = _x;
        y = _y;

        velTotal.x += x - oldX;
        velTotal.y += y - oldY;
        velCount += 1;

        Vec2d dir = {velTotal.x / velCount, velTotal.y / velCount};
        dir.x /= 2;
        dir.y /= 2;

        //vel = normalise(dir);
        vel = dir;

        
        
    }

};


#endif