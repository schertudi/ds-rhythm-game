
#include <nds/arm9/console.h>
#include "engineController.h"
#include "../sysWrappers/debugTools.h"
#include "../sysWrappers/input.h"



void EngineController::load () {
    frame = 0;
    combo = 0;
    isAutomatedPlay = false;
    audioPlayer = AudioPlayer();
    timeTracker.init(bpm, numSubBeats);
    
    //load in level data at this point too
    levelData levelData = LevelDataParser::setup(numSubBeats * numBeatsInBar);
    path.init(2, 60, levelData.beatInteracts);
    animationCommandManager.init(levelData.animations);
    energyLevelTracker.init(levelData);
}

void EngineController::unload () {
}

sceneStates EngineController::update() {
    frame += 1;
    int oldEnergyLevel = energyLevelTracker.getEnergyLevel();

    int beatStatus = timeTracker.updateBeat(frame);
    songPosition songPos = timeTracker.getSongPosition();

    //playMetronome(beatStatus);

    if (beatStatus > 0) {
        if (beatStatus == 2 && songPos.globalBeat == 0) { //just changed main beat to 0
            audioPlayer.startMusic();
        }
        
        path.onBeat(songPos);
        energyLevelTracker.newBeat(songPos);
    }

    if (isAutoPlayButtonDown()) {
        isAutomatedPlay = !isAutomatedPlay;
    }

    Vec2d penPos = getPenPos();

    std::vector<playableBeatStatus> beatStates = path.getBeatStates(songPos, penPos.x, penPos.y, isAutomatedPlay);
    updateHitBeats(beatStates, songPos);

    int energyLevel = energyLevelTracker.getEnergyLevel();
    if (energyLevel > oldEnergyLevel) {
        audioPlayer.enableJingle();
    }
    animationCommandManager.updateAnimations(songPos, beatStates, penPos, energyLevel);

    powerupInfo p = energyLevelTracker.getCurrPowerupInfo();
    energyLevelDisplay.draw(p, songPos);

    int fineBeat = songPos.globalBeat * songPos.numSubBeats + songPos.subBeat;

    consoleClear();
    Debugger::framePrint("time .%i.", fineBeat * 100 + songPos.subBeatProgress);
    Debugger::framePrint("beat .%i.", songPos.globalBeat);
    Debugger::framePrint("bar# %i", songPos.bar);
    Debugger::framePrint("combo %i", combo);
    Debugger::framePrint("energy %i", energyLevel);
    Debugger::framePrint("automated %i", isAutomatedPlay);

    return sceneStates::CURRENT;

}

void EngineController::playMetronome(int beatStatus) {
    if (beatStatus == 1) {
        audioPlayer.metronome(1);
    }
    if (beatStatus == 2) {
        audioPlayer.metronome(2);
    }
}

void EngineController::updateHitBeats(std::vector<playableBeatStatus> beatStates, songPosition songPos) {
    
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
}
