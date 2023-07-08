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

int lerp(int a, int b, int t);
int inverseLerp(int a, int b, int value);

#endif // VECTORSHAPES_H