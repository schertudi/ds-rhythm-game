//works with gl lib to draw vector shapes using nds 3d hardware

#include <nds.h>
#include <nds/arm9/console.h>
#include <nds/ndstypes.h>
#include <stdio.h>
#include <gl2d.h>
#include "vscode_fix.h"
#include "constants.h"
#include "vectorShapes.h"
#include "mathHelpers.h"

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

void vectorThickLine(int x1, int y1, int x2, int y2, int lineWidth, Colour c, bool caps) {
    vectorWideningLine(x1, y1, x2, y2, lineWidth, lineWidth, c, caps);
}

void vectorWideningLine(int x1, int y1, int x2, int y2, int width1, int width2, Colour c, bool caps) {
    Vec2d perp = perpVec({x2 - x1, y2 - y1});
    perp = normalizeVec(perp);
    
    int lineOffsetX1 = (perp.x * width1) / 100;
    int lineOffsetY1 = (perp.y * width1) / 100;
    int lineOffsetX2 = (perp.x * width2) / 100;
    int lineOffsetY2 = (perp.y * width2) / 100;

    Vec2d corn1 = {x1 + lineOffsetX1, y1 + lineOffsetY1};
    Vec2d corn2 = {x1 - lineOffsetX1, y1 - lineOffsetY1};
    Vec2d corn3 = {x2 + lineOffsetX2, y2 + lineOffsetY2};
    Vec2d corn4 = {x2 - lineOffsetX2, y2 - lineOffsetY2};

    glBegin2D();
        glTriangleFilled(corn1.x, corn1.y, corn2.x, corn2.y, corn3.x, corn3.y, RGB15(c.r, c.g, c.b));
        glTriangleFilled(corn2.x, corn2.y, corn3.x, corn3.y, corn4.x, corn4.y, RGB15(c.r, c.g, c.b));
    glEnd2D();

    //could optimise a bit by not drawing what is covered by line
    if (caps) {
        vectorCircle(x1, y1, width1 - 1, c);
        vectorCircle(x2, y2, width2 - 1, c);
    }
}