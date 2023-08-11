#include <nds.h>
#include <nds/arm9/console.h>
#include <stdio.h>
#include <nds/arm9/input.h>
#include "vscode_fix.h"
#include <gl2d.h>
//#include "archive/effects.cpp"
//#include "archive/trail.cpp"
#include "songTimeTracker.h"
#include "audioPlayer.h"
#include "constants.h"
#include "vectorShapes.h"
#include "mathHelpers.h"
#include "animationCommandManager.h"
#include <string.h>
#include <array>
#include "hitBeatUpdater.h"
#include "hitBeat.h"
#include "debugTools.h"
#include "levelParser.h"
#include "energyLevelTracker.h"
#include "energyLevelDisplay.h"

//inputs: click beat on red to play it, press left trigger (Q key on melonDS) to toggle automated/manual playthrough



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
	AudioPlayer audioPlayer = AudioPlayer();
	//CircleEffect pointerEffect(800, 20);
	touchPosition touch;
	//TouchTracker touchTracker(0);
	int numSubBeats = 2;
	int numBeatsInBar = 4;
	SongTimeTracker timeTracker(120, numSubBeats); //it doesnt like high bpm with fine granularity (eg 120,4); suspect (sub)progress not always hitting 0
	//it also goes insane with a bpm like (70, 2).

	levelData levelData = LevelDataParser::setup(numSubBeats * numBeatsInBar);
	HitBeatUpdater path = HitBeatUpdater(2, 60, levelData.beatInteracts);

	AnimationCommandManager animationCommandManager = AnimationCommandManager(levelData.animations);

	std::vector<int> perBarEnergyLevel = levelData.perBarEnergyLevel;

	EnergyLevelTracker energyLevelTracker = EnergyLevelTracker(levelData);
	EnergyLevelDisplay energyLevelDisplay;

	int combo = 0;
	int energyLevel = 3; //1 lowest, highest is 3
	bool isAutomatedPlay = false;
	Debugger::resetErrorMessage();

	while(1)
	{
		Debugger::resetFrameLines();
		

		scanKeys();
		int key = keysHeld();
		int justDown = keysDown();

		if (justDown & KEY_L) { //if left trigger button just pressed
			isAutomatedPlay = !isAutomatedPlay;
		}

		frame++;

		int beatStatus = timeTracker.updateBeat(frame);
		songPosition songPos = timeTracker.getSongPosition();
		int beat = songPos.globalBeat;
		

		if (beatStatus == 1) {
			//audioManager.metronome(1);
		}
		if (beatStatus == 2) {
			//audioManager.metronome(2);
		}

		if (beatStatus > 0) {
			if (beatStatus == 2 && beat == 2) { //just changed main beat to 2
				audioPlayer.startMusic();
			}

			if ( (songPos.localBeat) % 4 == 0) {
				//audioManager.metronome(1);
			}
			
			path.onBeat(songPos);
			energyLevelTracker.newBeat(songPos);
		}

		int penX = -100;
		int penY = -100;
		bool isPenDown = key & KEY_TOUCH;

		if(isPenDown) {
			touchRead(&touch);

			penX = touch.px;
			penY = touch.py;
			
			//pointerEffect.basicCircle(touch.px, touch.py, songPos.globalBeatProgress);
			//touchTracker.logTouch(touch.px, touch.py);
		}

		std::vector<playableBeatStatus> beatStates = path.getBeatStates(songPos, penX, penY, isAutomatedPlay);

		for (size_t i = 0; i < beatStates.size(); i++) {
			playerStatus status = beatStates[i].playerState;
			int beatStart = beatStates[i].beatStart;
			bool isSlider = beatStates[i].isSlider;
			if (status == beatStates[i].oldPlayerState) continue; //only interested in acting on new events
			if (!beatStates[i].isActive) continue; //don't care about beats marked for deletion

			if (status == playerStatus::EARLY_HIT) {
				combo = 0;
				path.deactivateBeat(beatStart, songPos.globalBeat);
				energyLevelTracker.beatEarly(beatStart);
			}
			if (isSlider) {
				if (status == playerStatus::SLIDER_HIT) { //starting beat, play sound (once)
					combo += 1;
					path.playSound(beatStart, audioPlayer);
					energyLevelTracker.beatHit(beatStart, songPos); //note we MIGHT have bugs if using songPos.globalBeat and not beatS
				}
				if (status == playerStatus::SLIDER_EARLY_LIFT) {
					combo = 0;
					path.deactivateBeat(beatStart, songPos.globalBeat);
				}
				if (status == playerStatus::SLIDER_END) { //play end beat
					combo += 1;
					path.playSound(beatStart, audioPlayer);
					path.deactivateBeat(beatStart, songPos.globalBeat);
					energyLevelTracker.beatHit(beatStates[i].beatEnd, songPos);
				}
			} else {
				//check if hit singular beat on time
				if (status == playerStatus::CORRECT_HIT) {
					combo += 1;
					path.playSound(beatStart, audioPlayer);
					energyLevelTracker.beatHit(beatStart, songPos);
				}
				if (status == playerStatus::CORRECT_LIFT) {
					path.deactivateBeat(beatStart, songPos.globalBeat);
				}
			}

			//late beat: not hit but passed its window
			if (status == playerStatus::MISS) {
				combo = 0;
				path.deactivateBeat(beatStart, songPos.globalBeat);
				energyLevelTracker.beatMiss(songPos.globalBeat);
			}
			
		}

		path.renderBeats(songPos);

		energyLevel = energyLevelTracker.getEnergyLevel();

		animationCommandManager.updateAnimations(songPos, beatStates, {penX, penY}, energyLevel);
		powerupInfo p = energyLevelTracker.getCurrPowerupInfo();
		energyLevelDisplay.draw(p, songPos);

		consoleClear();
		int fineBeat = songPos.globalBeat * songPos.numSubBeats + songPos.subBeat;

		Debugger::framePrint("time .%i.", fineBeat * 100 + songPos.subBeatProgress);
		Debugger::framePrint("beat .%i.", songPos.globalBeat);
		Debugger::framePrint("bar# %i", songPos.bar);
		Debugger::framePrint("combo %i", combo);
		Debugger::framePrint("energy %i", energyLevel);
		Debugger::framePrint("automated %i", isAutomatedPlay);

		Debugger::render();

		glFlush(0);

		swiWaitForVBlank();

	}

	return 0;
	
}



