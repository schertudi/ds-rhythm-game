#include <nds.h>
#include <nds/arm9/console.h>
#include <stdio.h>
#include <gl2d.h>
#include <nds/arm9/input.h>
#include "vscode_fix.h"
#include "effects.cpp"
#include "trail.cpp"
#include "beatManager.cpp"
#include "audioManager.cpp"


int main( int argc, char *argv[] )
{
	glScreen2D();

	//right screen uses 3d engine, is the main screen
	lcdMainOnBottom();
	videoSetMode( MODE_5_3D );

	//left screen uses 2d engine to print things, is the sub screen
	videoSetModeSub(MODE_5_2D);
	bgInitSub(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);

	//enable printing to left screen
	consoleDemoInit();

	int frame = 0;
	AudioManager audioManager = AudioManager();
	CircleEffect pointerEffect(800, 20);
	touchPosition touch;
	TouchTracker touchTracker(0);
	BeatManager beatManager(120, 1);
	RhythmPath path(audioManager);
	

	//mmLoad( MOD_FLATOUTLIES );

	//mmStart( MOD_FLATOUTLIES, MM_PLAY_LOOP );

	//

	while(1)
	{
		scanKeys();
		int key = keysHeld();

		frame++;

		bool newBeat = beatManager.updateBeat(frame);
		int beat = beatManager.getGlobalBeat();
		int progress = beatManager.getProgress();

		if (newBeat) {
			if (beat == 2) {
				audioManager.startMusic();
			}

			if ( (beatManager.getLocalBeat()) % 4 == 0) {
				//audioManager.metronome(1);
			}

			//audioManager.metronome(1);
			
			path.OnBeat(beat, beatManager.getLocalBeat(), beatManager.getBar(), progress);
			
		}

		if(key & KEY_TOUCH) {
			touchRead(&touch);

			path.updateBeats(beat, progress, touch.px, touch.py);
			
			pointerEffect.basicCircle(touch.px, touch.py, beatManager.getProgress());
			touchTracker.logTouch(touch.px, touch.py);
		} else {
			path.updateBeats(beat, progress, -100, -100);
			//pointerEffect.basicCircle(touch.px, touch.py, beatManager.getProgress());
		}
		touchTracker.deleteOldEntries();
		//touchTracker.drawTrail(frame);

		consoleClear();
		iprintf("\x1b[8;1Hbeat# %i", beat);
		iprintf("\x1b[9;1Hbar# %i", beatManager.getBar());
		iprintf("\x1b[10;1Hprogress# %i", progress);
		iprintf("\x1b[11;1Hcombo# %i", path.getCombo());

		glFlush(0);

		swiWaitForVBlank();

		
	}

	return 0;
	
}



