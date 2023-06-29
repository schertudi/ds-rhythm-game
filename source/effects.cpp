#ifndef POINTEREFFECT_H
#define POINTEREFFECT_H

#include <nds.h>
#include <nds/arm9/console.h>
#include <stdio.h>
#include <gl2d.h>
#include "vscode_fix.h"

#include "constants.h"


class CircleEffect {

	private:
	int speed = 100;
    int baseRadius = 40;

	public:
	CircleEffect(int s, int b) {
		speed = s;
        baseRadius = b;
	}

	void basicCircle(int xOrigin, int yOrigin, int beatProgress) { // beatProgress is 0-100
	/*
		basicCircle2(0, 10, cosLerp((0 * 32767) / 100) / 500);
		basicCircle2(SCREEN_WIDTH / 4, 10, cosLerp((25 * 32767) / 100) / 500);
		basicCircle2(SCREEN_WIDTH / 2, 10, cosLerp((beatProgress * 32767) / 100) / 500);
		basicCircle2(SCREEN_WIDTH / 4 * 3, 10, cosLerp((75 * 32767) / 100) / 500);
		basicCircle2(SCREEN_WIDTH, 10, cosLerp((100 * 32767) / 100) / 500);
		*/
	
		glBegin2D();

		//draw a filled circle using triangles
		int i;

		if (beatProgress > 95) beatProgress = 95;
		if (beatProgress < 5) beatProgress = 5;
		int radius = sinLerp((beatProgress * 32767) / 100) / 500;
		//int radius = (beatProgress - 0) / 10;
		radius += baseRadius;
		//radius = baseRadius - radius;

		radius /= 3;
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
							RGB15(15,0,31) );
		}
				
		glEnd2D();
		
	}

	void basicCircle2(int xOrigin, int yOrigin, int r) { // beatProgress is 0-100
		glBegin2D();

		//draw a filled circle using triangles
		int i;

		int radius = r + baseRadius;
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
							RGB15(31,0,31) );
		}
				
		glEnd2D();
	}

};

#endif // POINTEREFFECT_H
