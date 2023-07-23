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
#include "vectorShapes.h"
#include "mathHelpers.h"
#include "animationCommand.cpp"
#include <string.h>
#include <array>
#include "rhythmPath.cpp"
#include "beatToHit.cpp"
#include "debugTools.h"
#include "levelData.h"


/*
how to do different levels of tension?
tracker might have say 3 channels, 1 channel for each level
song is split into sections which indicates the max level it can have at that point
also have a number of beats need to get a combo in (eg get every beat in the last 2 bars right) 
when this combo is completed, you are allowed to move to the next level. if this combo is not achieved, you don't move
might want some kind of condition for prematurely going down a level, idk
might also want a second chance to reach this level - like 2 level 2's in a row - and if this is missed you go down again, if reached just stay at level 2

at a higher level, more channels play music and more animations will play. could even have a different hit sound, idk.

need a gui on top screen showing that you are about to move up a level - could show your current combo (counting from start)
and maybe in a bright colour. or just something flashing and saying "keep your combo". 
or a loading-bar thing that increases on each hit - indicating building pressure - "explodes" when you hit the next section correctly
and fizzles out if you lose combo

how is this enforced in code....?





*/
enum tokens {single, nopitch, quart, diagonalBouncingBall, sineWave, top, bottom, slider, half, one, two, three, four};

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
	//it also goes insane with a bpm like (70, 2).
	RhythmPath path = RhythmPath(2, 60);

	AnimationCommandManager animationCommandManager = AnimationCommandManager();
	Animator animator = Animator();

	int combo = 0;
	Debugger::resetErrorMessage();

	//getPitch("c3");
	levelData::setup();
	//creates list of BeatInteractable* which should be given to rhythmPath (need to tweak a few things to get this to work)
	//and AnimationCommand* which should be given to animationCommandManager
	

	//mmLoad( MOD_FLATOUTLIES );

	//mmStart( MOD_FLATOUTLIES, MM_PLAY_LOOP );

	//

	/*
	while (1) {
		//draw fun animations........
		scanKeys();
		int key = keysHeld();

		frame++;

		beatManager.updateBeat(frame);
		songPosition songPos = beatManager.getSongPosition();

		//animator.bouncingBallStraight(songPos.globalBeatProgress, songPos.globalBeat, 2);
		//animator.bouncingBallDiagonal(songPos.globalBeatProgress, songPos.globalBeat);
		//animator.fillTank(songPos.globalBeatProgress, songPos.globalBeat, 6);
		//animator.sineWave(songPos.globalBeatProgress, songPos.globalBeat, 6, 1);
		//animator.sineWave(songPos.globalBeatProgress, songPos.globalBeat, 6, 2, true);
		//animator.dancingStarfish(songPos.globalBeatProgress, songPos.globalBeat);
		//animator.slidingStarfish({100,50}, {200, 100}, {30, 30}, songPos.globalBeatProgress, songPos.globalBeat);
		//animator.shakingObject(songPos.globalBeat, songPos.globalBeatProgress);

		
		//if (songPos.globalBeat < 2) {
		//	animator.shakingObject(songPos.globalBeat, songPos.globalBeatProgress);
		//} else {
		//	if (songPos.globalBeat == 2 && songPos.globalBeatProgress < 20) animator.hitObject(songPos.globalBeatProgress);
		//	else animator.burstingObject(songPos.globalBeat - 2, songPos.globalBeatProgress);
		//}
		
		
		

		
		int x = SCREEN_WIDTH / 2;
		int y = -100;
		if(key & KEY_TOUCH) {
			touchRead(&touch);
			x = touch.px;
			y = touch.py;
		} else {
		}

		animationCommandManager.updateAnimations(songPos.globalBeat, songPos.globalBeatProgress);

		//animator.slidingStarfish({20, 20}, {100, 50}, {x, y}, songPos.globalBeatProgress, songPos.globalBeat);
		//animator.colourChangeSlider(20, SCREEN_HEIGHT - 60, 60, {30, 15, 0}, {25, 0, 15}, {x, y});
		//animator.slidingCircle({20, 20}, {100, 50}, {x, y});
		//animator.flyingBall(songPos.globalBeatProgress, songPos.globalBeat, {0, 50}, {200, 100}, 20);
		//throwObject.draw(frame, songPos.globalBeat, songPos.globalBeatProgress);


		iprintf("\x1b[8;1Hbeat# .%i.", songPos.globalBeat);
		iprintf("\x1b[11;1Hprogress# .%i.", songPos.globalBeatProgress);
		

		glFlush(0);

		swiWaitForVBlank();
	}

	*/
	while(1)
	{
		Debugger::resetLines();
		

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

			
			
			path.onBeat(songPos);
			
		}

		//int hitState;
		int penX = -100;
		int penY = -100;
		bool isPenDown = key & KEY_TOUCH;

		if(isPenDown) {
			touchRead(&touch);

			penX = touch.px;
			penY = touch.py;

			//hitState = path.updateBeats(songPos, touch.px, touch.py);
			
			pointerEffect.basicCircle(touch.px, touch.py, songPos.globalBeatProgress);
			touchTracker.logTouch(touch.px, touch.py);
		} else {
			//hitState = path.updateBeats(songPos, -100, -100);
			//pointerEffect.basicCircle(touch.px, touch.py, beatManager.getProgress());
		}

		//how efficient is this?
		std::vector<playableBeatStatus> beatStates = path.getBeatStates(songPos, penX, penY);

		
		for (size_t i = 0; i < beatStates.size(); i++) {
			playerStatus status = beatStates[i].playerState;
			int beatStart = beatStates[i].beatStart;
			bool isSlider = beatStates[i].isSlider;
			if (status == beatStates[i].oldPlayerState) continue; //only interested in acting on new events
			if (!beatStates[i].isActive) continue; //don't care about beats marked for deletion

			if (status == playerStatus::EARLY_HIT) {
				combo = 0;
				path.deactivateBeat(beatStart, songPos.globalBeat);
			}
			if (isSlider) {
				if (status == playerStatus::SLIDER_HIT) { //starting beat, play sound (once)
					combo += 1;
					path.playSound(beatStart, audioManager);
				}
				if (status == playerStatus::SLIDER_EARLY_LIFT) {
					combo = 0;
					path.deactivateBeat(beatStart, songPos.globalBeat);
				}
				if (status == playerStatus::SLIDER_END) { //play end beat
					combo += 1;
					path.playSound(beatStart, audioManager);
					path.deactivateBeat(beatStart, songPos.globalBeat);
				}
			} else {
				//check if hit singular beat on time
				if (status == playerStatus::CORRECT_HIT) {
					combo += 1;
					path.playSound(beatStart, audioManager);
				}
				if (status == playerStatus::CORRECT_LIFT) {
					path.deactivateBeat(beatStart, songPos.globalBeat);
				}
			}

			//late beat: not hit but passed its window
			if (status == playerStatus::MISS) {
				combo = 0;
				path.deactivateBeat(beatStart, songPos.globalBeat);
			}
			
		}

		//is just based on a timer, will remove items from list 5 beats after they were deactivated
		path.killInactiveBeats(songPos.globalBeat, 5);

		path.renderBeats(songPos);


		touchTracker.deleteOldEntries();

		animationCommandManager.updateInteractiveAnimations(songPos, beatStates, {penX, penY});
		//animator.bouncingBallDiagonal(songPos.globalBeatProgress, songPos.globalBeat);
		//touchTracker.drawTrail(frame);
		//animator.fillTank(songPos.globalBeat, songPos.globalBeatProgress, 4, {0, 20, 20});
		int val = animationCommandManager.getVal();


		consoleClear();
		int fineBeat = songPos.globalBeat * songPos.numSubBeats + songPos.subBeat;

		Debugger::framePrint("time .%i.", fineBeat * 100 + songPos.subBeatProgress);
		Debugger::framePrint("beat .%i.", songPos.globalBeat);
		Debugger::framePrint("bar# %i", songPos.bar);
		Debugger::framePrint("combo %i", combo);
		


		Debugger::render();


		glFlush(0);

		swiWaitForVBlank();

		
	}

	

	return 0;
	
}



