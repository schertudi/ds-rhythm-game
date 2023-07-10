#include <stdlib.h>
#include <nds.h>

#include "vectorShapes.h"


int intAtan2(int y, int x) {
    //http://dspguru.com/dsp/tricks/fixed-point-atan2-with-self-normalization/
    int pi = 314; //*100
    int coeff_1 = 78; //pi / 4 ie 0.78525
    int coeff_2 = 235; //pi / 4 * 3 ie 2.35575
    x *= 1000;
    y *= 1000;
    int abs_y = abs(y) + 1; //prevent 0 division

    int angle;
    if (x>=0) {
        int r = (x - abs_y) * 100 / (x + abs_y);
        angle = coeff_1 - ((coeff_1 * r) / 100);
    } else {
        int r = (x + abs_y) * 100 / (abs_y - x);
        angle = coeff_2 - ((coeff_1 * r) / 100);
    }

    if (y < 0) return -angle;     // negate if in quad III or IV
    else return angle;
}

int normalizeAngle(int angle) {
    if (angle < 0) angle += 360;
    if (angle > 360) angle -= 360;
    return angle;
}

//https://gist.github.com/shaunlebron/8832585
int shortAngleDist(int a0, int a1) {
    int max = 360;
    int da = (a1 - a0) % max;
    return 2*da % max - da;
}

int angleLerp(int a0, int a1, int t) {
    return a0 + shortAngleDist(a0,a1)*t;
}

Vec2d perpVec(Vec2d vec) { 
    return {-vec.y, vec.x};
}

int lerp(int a, int b, int t) { //t is in range of 0 to 100
    //a = 0, b = 200, t = 50 gives 100
    return ((100 - t) * a + t * b) / 100;
}

int inverseLerp(int a, int b, int value) {
    //a = 0, b = 200, value = 100 gives 50
    return ((value - a) * 100) / ((b - a) * 1);
}

Vec2d normalizeVec(Vec2d vec) { //returns to 2dp, so 100 -> 1.0
    int sqrtFixed = sqrtf32( (vec.x * vec.x + vec.y * vec.y) << 12 );
    int sqrtInt = sqrtFixed >> 12;
    int sqrtFlt = sqrtFixed & 0xFFF;
    int xFixed = vec.x << 12;
    int yFixed = vec.y << 12;
    //int sqrt3dp = sqrtInt * 100 + sqrtFlt;
    int normX = xFixed * 100 / sqrtFixed;
    int normY = yFixed * 100 / sqrtFixed;
    if (normX < 0) normX *= -1; //cant get float from signed int
    if (normY < 0) normY *= -1;
    normX = normX & 0xFFF; //get float portion, NOTE if it was exactly 1000 would get 0
    normY = normY & 0xFFF;
    //normX /= 10;
    //normY /= 10;
    if (vec.x < 0) normX = -normX;
    if (vec.y < 0) normY = -normY;
    return {normX, normY};
}