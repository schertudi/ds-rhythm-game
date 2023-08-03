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
#include "energyLevelManager.cpp"


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
	int numSubBeats = 2;
	int numBeatsInBar = 4;
	BeatManager beatManager(120, numSubBeats); //it doesnt like high bpm with fine granularity (eg 120,4); suspect (sub)progress not always hitting 0
	//it also goes insane with a bpm like (70, 2).

	levelData levelData = levelDataParser::setup(numSubBeats * numBeatsInBar);
	RhythmPath path = RhythmPath(2, 60, levelData.beatInteracts);

	AnimationCommandManager animationCommandManager = AnimationCommandManager(levelData.animations);

	std::vector<int> perBarEnergyLevel = levelData.perBarEnergyLevel;

	energyLevelManager energyLevelManager;

	int combo = 0;
	int energyLevel = 3; //1 lowest, highest is 3
	Debugger::resetErrorMessage();

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

		if (songPos.bar >= 0 && (size_t)songPos.bar < perBarEnergyLevel.size()) {
			//will update slightly too late (early animations won't show), unsure if should fix or not
			energyLevel = perBarEnergyLevel[songPos.bar];
		}
		

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
			energyLevelManager.newBeat(songPos.globalBeat);
			
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
					energyLevelManager.beatHit();
				}
				if (status == playerStatus::SLIDER_EARLY_LIFT) {
					combo = 0;
					path.deactivateBeat(beatStart, songPos.globalBeat);
				}
				if (status == playerStatus::SLIDER_END) { //play end beat
					combo += 1;
					path.playSound(beatStart, audioManager);
					path.deactivateBeat(beatStart, songPos.globalBeat);
					energyLevelManager.beatHit();
				}
			} else {
				//check if hit singular beat on time
				if (status == playerStatus::CORRECT_HIT) {
					combo += 1;
					path.playSound(beatStart, audioManager);
					energyLevelManager.beatHit();
				}
				if (status == playerStatus::CORRECT_LIFT) {
					path.deactivateBeat(beatStart, songPos.globalBeat);
				}
			}

			//late beat: not hit but passed its window
			if (status == playerStatus::MISS) {
				combo = 0;
				path.deactivateBeat(beatStart, songPos.globalBeat);
				energyLevelManager.beatMiss();
			}
			
		}

		

		//path.renderBeats(songPos);


		//touchTracker.deleteOldEntries();

		//animationCommandManager.updateAnimations(songPos, beatStates, {penX, penY}, energyLevel);


		consoleClear();
		int fineBeat = songPos.globalBeat * songPos.numSubBeats + songPos.subBeat;

		Debugger::framePrint("time .%i.", fineBeat * 100 + songPos.subBeatProgress);
		Debugger::framePrint("beat .%i.", songPos.globalBeat);
		Debugger::framePrint("bar# %i", songPos.bar);
		Debugger::framePrint("combo %i", combo);
		Debugger::framePrint("energy %i", energyLevel);
		Debugger::framePrint("state %i", energyLevelManager.getCurrState());
		

		Debugger::render();


		glFlush(0);

		swiWaitForVBlank();

		
	}

	

	return 0;
	
}



