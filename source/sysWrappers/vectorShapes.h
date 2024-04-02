#ifndef VECTORSHAPES_H
#define VECTORSHAPES_H

#include "../genericTypes.h"

void vectorCircle(int xOrigin, int yOrigin, int radius, Colour c, int depth);
void vectorRect(int x1, int y1, int x2, int y2, Colour c, int depth);
void vectorThickLine(int x1, int y1, int x2, int y2, int lineWidth, Colour c, int depth, bool caps=true);
void vectorWideningLine(int x1, int y1, int x2, int y2, int width1, int width2, Colour c, int depth, bool caps=true);
void vectorTriangle(int x1, int y1, int x2, int y2, int x3, int y3, Colour c, int depth);
void startShape();
void endShape();

#endif // VECTORSHAPES_H