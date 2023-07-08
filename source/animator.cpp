#ifndef ANIMATOR_H
#define ANIMATOR_H

#include <nds.h>
#include <nds/arm9/console.h>
#include <stdio.h>
#include <gl2d.h>
#include "vscode_fix.h"
#include "vectorShapes.h"

#include "constants.h"


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
        vectorCircle(x, y, 10, {31, 31, 31});
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

        vectorCircle(x, y, 10, {31, 31, 31});
    }

    void fillTank(int progress, int beat, int numBeats) {
        beat = beat % numBeats;
        if (beat < numBeats - 2) {
            int y = (SCREEN_HEIGHT / numBeats * (numBeats - beat));
            vectorRect(0, y, SCREEN_WIDTH, SCREEN_HEIGHT, {0, 31, 25});
        } else if (beat < numBeats - 1) {
            int y = (SCREEN_HEIGHT / numBeats * (numBeats - beat));
            vectorRect(0, y, SCREEN_WIDTH, SCREEN_HEIGHT, {0, 31, 15});
        } else {
            //animate it being drained; at progress = 0 we are at min y and at progress = 0 we are at SCREEN_HEIGHT
            //if (progress < 50) progress = 0;
            int y = lerp((SCREEN_HEIGHT / numBeats * (numBeats - beat + 1)), SCREEN_HEIGHT, progress);
            vectorRect(0, y, SCREEN_WIDTH, SCREEN_HEIGHT, {0, 31, 15});
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
                            RGB15(21, 0, 21) );
        }
                
        glEnd2D();
    }

};



#endif