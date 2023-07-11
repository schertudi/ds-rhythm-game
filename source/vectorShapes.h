#ifndef VECTORSHAPES_H
#define VECTORSHAPES_H

struct Colour {
    int r;
    int g;
    int b;
};

struct Vec2d {
    int x;
    int y;
};

void vectorCircle(int xOrigin, int yOrigin, int radius, Colour c);
void vectorRect(int x1, int x2, int y1, int y2, Colour c);
void vectorThickLine(int x1, int y1, int x2, int y2, int lineWidth, Colour c, bool caps=true);
void vectorWideningLine(int x1, int y1, int x2, int y2, int width1, int width2, Colour c, bool caps=true);

#endif // VECTORSHAPES_H