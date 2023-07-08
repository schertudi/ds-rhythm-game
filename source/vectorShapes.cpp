//works with gl lib to draw vector shapes using nds 3d hardware

#include <nds.h>
#include <nds/arm9/console.h>
#include <stdio.h>
#include <gl2d.h>
#include "vscode_fix.h"
#include "constants.h"
#include "vectorShapes.h"

void vectorCircle(int xOrigin, int yOrigin, int radius, Colour c) { // beatProgress is 0-100
    glBegin2D();

    //draw a filled circle using triangles
    int i;

    for( i = 0; i < BRAD_PI*2; i += 256)
    {
        int x = (cosLerp(i) * (radius) ) >> 12;
        int y = (sinLerp(i) * (radius) ) >> 12;

        int x2 = (cosLerp(i + 256) * (radius) ) >> 12;
        int y2 = (sinLerp(i + 256) * (radius) ) >> 12;

        // draw a triangle
        glTriangleFilled( xOrigin + x, yOrigin + y,
                        xOrigin + x2, yOrigin + y2,
                        xOrigin, yOrigin,
                        RGB15(c.r, c.g, c.b) );
    }
            
    glEnd2D();
}

void vectorRect(int x1, int y1, int x2, int y2, Colour c) {
    glBegin2D();
    glBoxFilled(x1, y1, x2, y2, RGB15(c.r, c.g, c.b));
    glEnd2D();
}

int lerp(int a, int b, int t) { //t is in range of 0 to 100
    //a = 0, b = 200, t = 50 gives 100
    return ((100 - t) * a + t * b) / 100;
}

int inverseLerp(int a, int b, int value) {
    //a = 0, b = 200, value = 100 gives 50
    return ((value - a) * 100) / ((b - a) * 1);
}