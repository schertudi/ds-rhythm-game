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
#include "constants.h"


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
	BeatManager beatManager(120, 2); //it doesnt like high bpm with fine granularity (eg 120,4); suspect (sub)progress not always hitting 0
	RhythmPath path(audioManager);
	

	//mmLoad( MOD_FLATOUTLIES );

	//mmStart( MOD_FLATOUTLIES, MM_PLAY_LOOP );

	//

	while(1)
	{
		scanKeys();
		int key = keysHeld();

		frame++;

		int beatStatus = beatManager.updateBeat(frame);
		songPosition songPos = beatManager.getSongPosition();
		int beat = songPos.globalBeat;
		//int progress = songPos.globalBeatProgress;

		if (beatStatus == 1) {
			//audioManager.metronome(1);
		}
		if (beatStatus == 2) {
			//audioManager.metronome(2);
		}

		if (beatStatus > 0) {
			if (beatStatus == 2 && beat == 2) { //just changed main beat to 2
				audioManager.startMusic();
			}

			if ( (songPos.localBeat) % 4 == 0) {
				//audioManager.metronome(1);
			}

			
			
			path.OnBeat(songPos);
			
		}

		if(key & KEY_TOUCH) {
			touchRead(&touch);

			path.updateBeats(songPos, touch.px, touch.py);
			
			pointerEffect.basicCircle(touch.px, touch.py, songPos.globalBeatProgress);
			touchTracker.logTouch(touch.px, touch.py);
		} else {
			path.updateBeats(songPos, -100, -100);
			//pointerEffect.basicCircle(touch.px, touch.py, beatManager.getProgress());
		}
		touchTracker.deleteOldEntries();
		//touchTracker.drawTrail(frame);

		consoleClear();
		iprintf("\x1b[8;1Hbeat# %i", songPos.localBeat * songPos.numSubBeats + songPos.subBeat);
		iprintf("\x1b[9;1Hbar# %i", songPos.bar);
		iprintf("\x1b[10;1Hcombo %i", path.getCombo());
		/*
		iprintf("\x1b[8;1HglobalBeat# %i", songPos.globalBeat);
		iprintf("\x1b[9;1HlocalBeat# %i", songPos.localBeat);
		iprintf("\x1b[10;1Hbar# %i", songPos.bar);
		iprintf("\x1b[11;1Hprogress# %i", songPos.globalBeatProgress);
		iprintf("\x1b[12;1H ");
		iprintf("\x1b[13;1HsubBeat# %i", songPos.subBeat);
		iprintf("\x1b[14;1HsubProgress# %i", songPos.subBeatProgress);
		iprintf("\x1b[15;1HfineBeat# %i", songPos.localBeat * songPos.numSubBeats + songPos.subBeat);
		//iprintf("\x1b[11;1Hcombo# %i", path.getCombo());
		*/

		glFlush(0);

		swiWaitForVBlank();

		
	}

	return 0;
	
}



