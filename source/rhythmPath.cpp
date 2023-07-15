#ifndef RHYTHMPATH_H
#define RHYTHMPATH_H

#include <nds.h>
#include <nds/arm9/console.h>
#include <stdio.h>
#include <vector>
#include "beatToHit.cpp"
#include <iostream>
#include "constants.h"
#include "noteDefinitions.h"

struct beatEntry {
    int beatStart;
    int beatEnd;
    int x;
    int y;
    int pitch = 0;
    int length = 1;
    int endX = x;
    int endY = y;
};

class RhythmPath {
    /*
    in path we need x,y pos, beat at which it is triggered, beat it ends at, trigger for animation if used
    */
    private:
    int beatLookAhead;
    int margin; // if we are 10% close to beat or 10% past this is still correct
    
    //simple test
    std::vector<beatEntry> bar1 = {
        {0, 0, 20, 20, NOTE_C * OCT_3, QUART_BEAT, 80, 20},
        {2, 2, 50, 20, NOTE_FS * OCT_3, QUART_BEAT},
        {4, 4, 80, 20, NOTE_FS * OCT_3, QUART_BEAT},
        {6, 6, 110, 20, NOTE_FS * OCT_3, QUART_BEAT},
        //{6, 8, 80, 20, NOTE_FS * OCT_3, QUART_BEAT, 120, 20}
    };

    std::vector<beatEntry> bar2 = {
        {2, 6, 20, 20, NOTE_C * OCT_3, QUART_BEAT, 50, 20}
    };    

    std::vector<std::vector<beatEntry>> beatMap = {
        bar1
    };


    std::vector<BeatInteractable*> spawnedBeats;

    public:
    RhythmPath(int _lookahead, int _margin) {
        beatLookAhead = _lookahead;
        margin = _margin;
    }
   
    void onBeat(songPosition pos) {
        trySpawnBeat(pos);
    }

    std::vector<playableBeatStatus> getBeatStates(songPosition pos, int touchX, int touchY) {
        std::vector<playableBeatStatus> states;
        int beat = pos.globalBeat * pos.numSubBeats + pos.subBeat;
        int progressToNext = pos.subBeatProgress;

        for (size_t i = 0; i < spawnedBeats.size(); i++) {
            BeatInteractable* b = spawnedBeats[i];
            int p = b->getBeatProgress(beat, progressToNext, margin);
            
            bool isHit = b->isHit(touchX, touchY, beat, progressToNext, margin);

            playableBeatStatus beatState;
            beatState.beatStart = b->getStartBeat();
            beatState.timingProgress = p;
            beatState.isHit = isHit;
            beatState.oldPlayerState = b->getPlayerState();

            playerStatus state;
            if (b->isSlider()) {
                beatState.isSlider = true;
                state = getPlayerStatusForSliderBeat(beatState);
            } else {
                beatState.isSlider = false;
                state = getPlayerStatusForSingleBeat(beatState);
            }
        
            b->setPlayerState(state);
            beatState.playerState = state;

            beatState.isActive = b->isActive();
            
            states.push_back(beatState);
        }

        return states;
    }

    void renderBeats(songPosition pos) {
        int beat = pos.globalBeat * pos.numSubBeats + pos.subBeat;
        int progressToNext = pos.subBeatProgress;

        for (size_t i = 0; i < spawnedBeats.size(); i++) {
            BeatInteractable* b = spawnedBeats[i];

            int p = b->getBeatProgress(beat, progressToNext, margin);
            if (b->isActive()) b->render(p);
        }

    }

    //TODO: better memory management for this
    void deactivateBeat(int beatStart, int currBeat) {
        //return;
        for (size_t i = 0; i < spawnedBeats.size(); i++) {
            if (spawnedBeats[i]->getStartBeat() == beatStart) {
                spawnedBeats[i]->deactivate(currBeat);
            }
        }
    }

    void killInactiveBeats(int currBeat, int timeout) {
        for (size_t i = 0; i < spawnedBeats.size(); i++) {
            if (!spawnedBeats[i]->isActive()) {
                int length = spawnedBeats[i]->getTimeSinceDeactivate(currBeat);
                if (length > timeout) {
                    spawnedBeats.erase(spawnedBeats.begin() + i);
                    i--;
                }
            }
        }
    }

    void playSound(int beatStart, AudioManager man) {
        for (size_t i = 0; i < spawnedBeats.size(); i++) {
            if (spawnedBeats[i]->getStartBeat() == beatStart) {
                spawnedBeats[i]->playSound(man);
            }
        }
    }

    private:

    //a beat starting at beat n should be spawned at beat n - beatLookAhead. if this < 0 then beatManager needs to lower its starting beat.
    void trySpawnBeat(songPosition pos) {
        int bar = pos.bar;
        int globalBeat = (pos.globalBeat + beatLookAhead) * pos.numSubBeats + pos.subBeat;

        if (bar >= (int)beatMap.size()) {
            return;
        }
        std::vector<beatEntry> beats = beatMap[bar];

        int index = -1;
        for (size_t i = 0; i < beats.size(); i++) {
            if (localToGlobalBeat(beats[i].beatStart, pos) == globalBeat) {
                index = i;
                break;
            }
        }

        if (index == -1) {
            return;
        }

        if (beats[index].beatEnd == beats[index].beatStart) {
            // spawn beat
            BeatToHit* newBeat = new BeatToHit(globalBeat, beats[index].x, beats[index].y, beats[index].length, beats[index].pitch);
            newBeat->setPlayerState(playerStatus::IDLE);
            spawnedBeats.push_back(newBeat);
            return;
        } else {
            //spawn beatLine
            BeatToSlide* newBeat = new BeatToSlide(globalBeat, globalBeat + (beats[index].beatEnd - beats[index].beatStart),
                beats[index].x, beats[index].y, beats[index].endX, beats[index].endY, beats[index].length, beats[index].pitch);
            spawnedBeats.push_back(newBeat);
        }

    }

    //might be good to move this somewhere else if other classes need to use
    int localToGlobalBeat(int localBeat, songPosition pos) {
        int localToGlobal = localBeat + pos.bar * pos.numBeatsInBar * pos.numSubBeats;
        return localToGlobal;
    }

    //duplicated code in these functions, but they're dense enough as it is so want to avoid extra branching
    playerStatus getPlayerStatusForSingleBeat(playableBeatStatus status) {
        //possible states: IDLE, EARLY_HIT, EARLY_LIFT, READY_TO_HIT, CORRECT_HIT, CORRECT_LIFT, MISS 
        playerStatus os = status.oldPlayerState;

        if (os == playerStatus::IDLE) {
            if (status.timingProgress >= 100) {
                return playerStatus::READY_TO_HIT;
            } else if (status.isHit) {
                return playerStatus::EARLY_HIT;
            }
        } else if (os == playerStatus::READY_TO_HIT) {
            if (status.isHit) {
                return playerStatus::CORRECT_HIT;
            } else if (status.timingProgress > 200) {
                return playerStatus::MISS;
            }
        } else if (os == playerStatus::EARLY_HIT) {
            if (!status.isHit) {
                return playerStatus::EARLY_LIFT;
            }
        } else if (os == playerStatus::CORRECT_HIT) {
            if (!status.isHit || status.timingProgress > 200) {
                return playerStatus::CORRECT_LIFT;
            }
        }
        return os; //should never run... might be good to have some kind of warning system TODO
    }

    playerStatus getPlayerStatusForSliderBeat(playableBeatStatus status) {
        //possible states: IDLE, EARLY_HIT, EARLY_LIFT, READY_TO_HIT, SLIDER_HIT, SLIDER_EARLY_LIFT, SLIDER_END, MISS 
        playerStatus os = status.oldPlayerState;

        if (os == playerStatus::IDLE) {
            if (status.timingProgress >= 100) {
                return playerStatus::READY_TO_HIT;
            } else if (status.isHit) {
                return playerStatus::EARLY_HIT;
            }
        } else if (os == playerStatus::READY_TO_HIT) {
            if (status.isHit) {
                return playerStatus::SLIDER_HIT;
            } else if (status.timingProgress > 100) {
                return playerStatus::MISS;
            }
        } else if (os == playerStatus::EARLY_HIT) {
            if (!status.isHit) {
                return playerStatus::EARLY_LIFT;
            }
        } else if (os == playerStatus::SLIDER_HIT) {
            if (!status.isHit || status.timingProgress > 200) {
                return playerStatus::SLIDER_EARLY_LIFT;
            }
            if (status.timingProgress >= 200) {
                return playerStatus::SLIDER_END;
            }
        } 
        return os;
    }

};


#endif // RHYTHMPATH_H
