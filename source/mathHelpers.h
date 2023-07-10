#ifndef MATH_HELPERS_H
#define MATH_HELPERS_H

#include "vectorShapes.h"

int intAtan2(int y, int x);

int normalizeAngle(int angle);

int angleLerp(int a0, int a1, int t);

int shortAngleDist(int a0, int a1);

Vec2d perpVec(Vec2d vec);

int lerp(int a, int b, int t);

int inverseLerp(int a, int b, int value);

Vec2d normalizeVec(Vec2d vec);

#endif