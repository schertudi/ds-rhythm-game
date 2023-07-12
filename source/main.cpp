#include <nds.h>
#include <nds/arm9/console.h>
#include <stdio.h>
#include <nds/arm9/input.h>
#include "vscode_fix.h"
#include "effects.cpp"
#include "trail.cpp"
#include "beatManager.cpp"
#include "audioManager.cpp"
#include "constants.h"
#include "animator.cpp"
#include "vectorShapes.h"
#include "mathHelpers.h"



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

	Animator animator = Animator();
	ThrowableObject throwObject = ThrowableObject(0, 0);
	

	//mmLoad( MOD_FLATOUTLIES );

	//mmStart( MOD_FLATOUTLIES, MM_PLAY_LOOP );

	//

	
	while (1) {
		//draw fun animations........
		scanKeys();
		int key = keysHeld();

		frame++;

		int beatStatus = beatManager.updateBeat(frame);
		songPosition songPos = beatManager.getSongPosition();

		//animator.bouncingBallStraight(songPos.globalBeatProgress, songPos.globalBeat, 2);
		//animator.bouncingBallDiagonal(songPos.globalBeatProgress, songPos.globalBeat);
		//animator.fillTank(songPos.globalBeatProgress, songPos.globalBeat, 6);
		//animator.sineWave(songPos.globalBeatProgress, songPos.globalBeat, 6, 1);
		//animator.sineWave(songPos.globalBeatProgress, songPos.globalBeat, 6, 2, true);
		//animator.dancingStarfish(songPos.globalBeatProgress, songPos.globalBeat);
		//animator.slidingStarfish({100,50}, {200, 100}, {30, 30}, songPos.globalBeatProgress, songPos.globalBeat);
		//animator.shakingObject(songPos.globalBeat, songPos.globalBeatProgress);

		/*
		if (songPos.globalBeat < 2) {
			animator.shakingObject(songPos.globalBeat, songPos.globalBeatProgress);
		} else {
			if (songPos.globalBeat == 2 && songPos.globalBeatProgress < 20) animator.hitObject(songPos.globalBeatProgress);
			else animator.burstingObject(songPos.globalBeat - 2, songPos.globalBeatProgress);
		}
		*/
		
		

		
		int x = SCREEN_WIDTH / 2;
		int y = -100;
		if(key & KEY_TOUCH) {
			touchRead(&touch);
			throwObject.penDown(songPos.globalBeat, touch.px, touch.py);
			x = touch.px;
			y = touch.py;
		} else {
			throwObject.penUp();
		}

		animator.slidingStarfish({20, 20}, {100, 50}, {x, y}, songPos.globalBeatProgress, songPos.globalBeat);
		//animator.colourChangeSlider(20, SCREEN_HEIGHT - 60, 60, {30, 15, 0}, {25, 0, 15}, {x, y});
		//animator.slidingCircle({20, 20}, {100, 50}, {x, y});
		//animator.flyingBall(songPos.globalBeatProgress, songPos.globalBeat, {0, 50}, {200, 100}, 20);
		//throwObject.draw(frame, songPos.globalBeat, songPos.globalBeatProgress);


		iprintf("\x1b[8;1Hbeat# .%i.", songPos.globalBeat);
		iprintf("\x1b[11;1Hprogress# .%i.", songPos.globalBeatProgress);
		Vec2d norm = normalizeVec({0, 3});
		

		glFlush(0);

		swiWaitForVBlank();
	}

/*
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
		
		//iprintf("\x1b[8;1HglobalBeat# %i", songPos.globalBeat);
		//iprintf("\x1b[9;1HlocalBeat# %i", songPos.localBeat);
		//iprintf("\x1b[10;1Hbar# %i", songPos.bar);
		//iprintf("\x1b[11;1Hprogress# %i", songPos.globalBeatProgress);
		//iprintf("\x1b[12;1H ");
		//iprintf("\x1b[13;1HsubBeat# %i", songPos.subBeat);
		//iprintf("\x1b[14;1HsubProgress# %i", songPos.subBeatProgress);
		//iprintf("\x1b[15;1HfineBeat# %i", songPos.localBeat * songPos.numSubBeats + songPos.subBeat);
		//iprintf("\x1b[11;1Hcombo# %i", path.getCombo());
		

		glFlush(0);

		swiWaitForVBlank();

		
	}

	*/

	return 0;
	
}



