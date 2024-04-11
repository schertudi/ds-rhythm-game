#pragma once

#include <tuple>
#include "../genericTypes.h"
#include <nds/ndstypes.h>


#define _angleToDegrees(angle)   ((angle) * 360 / (1 << 15))

#define _degreesToAngle(degrees) ((degrees) * (1 << 15) / 360)

/*! \brief fixed point cosine
	\param angle (-32768 to 32767)
	\return 4.12 fixed point number with the range [-1, 1]
*/
s16 _cosLerp(s16 angle);

/*! \brief fixed point sine
	\param angle (-32768 to 32767)
	\return 4.12 fixed point number with the range [-1, 1]
*/
s16 _sinLerp(s16 angle);

/**
*   \brief Fixed point sqrt
*   \param a Takes 20.12
*   \return returns 20.12 result
*/
int32 _sqrtf32(int32 a);

int intAtan2(int y, int x);

int normalizeAngle(int angle);

int angleLerp(int a0, int a1, int t);

int shortAngleDist(int a0, int a1);

Vec2d perpVec(Vec2d vec);

int lerp(int a, int b, int t);

int inverseLerp(int a, int b, int value);

Vec2d normalizeVec(Vec2d vec);

int sqrDist(Vec2d a, Vec2d b);

int dist(Vec2d a, Vec2d b);

Colour lerpColour(Colour c1, Colour c2, int t);

//project any given point onto line using distance from start pos
//returns the point on the line closest to penPos, and the distance from penPos to that point
//these involve very similar calculations so it's slightly more efficient to do them together at cost of being a slightly clunkier function
std::tuple<Vec2d, int> projectPointToLine(Vec2d start, Vec2d end, Vec2d penPos); 

std::tuple<int, int, int>  findQuadraticEquationFromGradients(int x1, int y1, int m1, int x2, int m2); //USELESS 

std::tuple<int, int, int> findQuadraticEquationFromPoints(int x1, int y1, int m1, int x2, int y2); //USELESS

Vec2d threePointBezier(Vec2d start, Vec2d control, Vec2d end, int t);

int convertBeatToTime(int beat, int progress);
