#include <stdlib.h>
#include <nds.h>
#include <math.h>

#include <tuple>
#include "vscode_fix.h"
#include "mathHelpers.h"


int intAtan2(int y, int x) {
    //http://dspguru.com/dsp/tricks/fixed-point-atan2-with-self-normalization/
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
    //int sqrtInt = sqrtFixed >> 12;
    //int sqrtFlt = sqrtFixed & 0xFFF;
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

int sqrDist(Vec2d a, Vec2d b) {
    int x = a.x - b.x;
    int y = a.y - b.y;
    return x * x + y * y;
}

int dist(Vec2d a, Vec2d b) {
    return sqrtf32(sqrDist(a, b) << 12 ) >> 12;
}

Colour lerpColour(Colour c1, Colour c2, int t) {
    return {
        lerp(c1.r, c2.r, t),
        lerp(c1.g, c2.g, t),
        lerp(c1.b, c2.b, t)
    };
}

std::tuple<Vec2d, int> projectPointToLine(Vec2d start, Vec2d end, Vec2d penPos) { //so that player can slide object along line defined by start to end
    Vec2d lineDir = {end.x - start.x, end.y - start.y};
    lineDir = normalizeVec(lineDir);

    int penDist = dist(start, penPos);
    int startEndDist = dist(start, end); 
    if (penDist > startEndDist) penDist = startEndDist;

    Vec2d linePos = {start.x + (lineDir.x * penDist) / 100, start.y + (lineDir.y * penDist) / 100};
    int lineProgress = inverseLerp(0, startEndDist, penDist);
    std::tuple<Vec2d, int> t = {linePos, lineProgress};
    return t;
}

std::tuple<int, int, int> findQuadraticEquationFromGradients(int x1, int y1, int m1, int x2, int m2) {
    /*
    y = ax^2 + bx + c
    dy/dx = 2ax + b
    for eq 1, dy/dx = m1 = 2*a*x1 + b
    for eq 2, dy/dx = m2 = 2*a*x2 + b
    we want to solve these equations simultaneously

    solve by making eq 1 = eq 2:
    0 = 2*a*x1 + b - m1
    0 = 2*a*x2 + b - m2
    -> 2*a*x1 + b - m1 = 2*a*x2 + b - m2

    simplify:
    (2*a*x1) - (2*a*x2) = -m2 + m1
    x1, x2, m1, m2 are known so this is solveable
    a(2*x1) - a(2*x2) = -m2 + m1
    (2*x1 - 2*x2) * a = -m2 + m1
    a = (-m2 + m1) / (2*x1 - 2*x2)

    then to find b:
    m1 = 2*a*x1 + b
    b = m1 - 2*a*x1

    and to find c:
    c = y - (ax^2 + bx)
    */
    int a = (-m2 + m1) * 10000 / (2*x1 - 2*x2);
    int b = m1 * 10000 - 2*a*x1;
    int c = y1 * 10000 - (a*x1*x1 + b*x1);

    return {a,b,c};
}

std::tuple<int, int, int> findQuadraticEquationFromPoints(int x1, int y1, int m1, int x2, int y2) {
    /*
    y = ax^2 + bx + c
    dy/dx = 2ax + b
    m1 = 2a(x1) + b
    b = m1 - 2a(x1)
    y1 = a * (x1^2) + (m1 - 2a(x1)) * x1 + c
    y1 = a(x1^2 - 2*x1) + m1*x1 + c
    0 = a(x1^2 - 2*x1) + m1*x1 + c - y1

    y2 = a * (x2^2) + (m1 - 2a(x1)) * x2 + c
    y2 = a * (x2^2 - 2*x1*x2) + x2*m1 + c
    0 = a * (x2^2 - 2*x1*x2) + x2*m1 + c - y2

    a(x1^2 - 2*x1) + m1*x1 + c - y1 = a * (x2^2 - 2*x1*x2) + x2*m1 + c - y2
    a((x1^2 - 2*x1) - (x2^2 - 2*x1*x2)) = x2*m1 - m1*x1 + c - c - y2 + y1
    a = (x2*m1 - m1*x1 - y2 + y1) / ((x1^2 - 2*x1) - (x2^2 - 2*x1*x2))
    b = m1 - 2a(x1)
    y1 = a * (x1^2) + b * x1 + c
    c = y1 - (a * (x1^2) + b * x1)
    */

    int a = ((x2*m1)/100 - (m1*x1/100) - y2 + y1) * 10000 / ((x1*x1 - 2*x1) - (x2*x2 - 2*x1*x2));
    int b = (m1 * 10000)/100 - 2*a*x1;
    int c = y1 * 10000 - (a * (x1*x1) + b * x1);

    return {a,b,c};
}

Vec2d threePointBezier(Vec2d start, Vec2d control, Vec2d end, int t) {
    int x = (start.x * (100 - t) * (100 - t) + 2 * control.x * t * (100 - t) + end.x * t * t) / 10000;
    int y = (start.y * (100 - t) * (100 - t) + 2 * control.y * t * (100 - t) + end.y * t * t) / 10000;
    return {x, y};
}

int convertBeatToTime(int beat, int progress) {
    return beat * 100 + progress;
}