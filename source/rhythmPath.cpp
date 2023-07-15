#ifndef RHYTHMPATH_H
#define RHYTHMPATH_H

#include <nds.h>
#include <nds/arm9/console.h>
#include <stdio.h>
#include <vector>
#include "beatToHit.cpp"
#include <iostream>
#include "audioManager.cpp"
#include "constants.h"
#include "noteDefinitions.h"
#include "vectorShapes.h"



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
    int beatLookAhead = 2;
    int margin = 60; // if we are 10% close to beat or 10% past this is still correct
    bool isComputerPlaying = false;

    
    //simple test
    std::vector<beatEntry> bar1 = {
        {0, 0, 20, 20, NOTE_C * OCT_3, QUART_BEAT, 60, 20},
        {2, 2, 50, 20, NOTE_FS * OCT_3, QUART_BEAT},
        {4, 4, 80, 20, NOTE_FS * OCT_3, QUART_BEAT}
    };

    std::vector<beatEntry> bar2 = {
        {2, 6, 20, 20, NOTE_C * OCT_3, QUART_BEAT, 50, 20}
    }; 
    

    //play rhythm, no pitch
    /*
    std::vector<beatEntry> bar1 = {
        {0, 0, 20, 20, NOTE_C * OCT_3, QUART_BEAT},
        {2, 2, 50, 20, NOTE_FS * OCT_3, QUART_BEAT},
        {4, 4, 80, 20, NOTE_AS * OCT_3, ONE_BEAT},
        {6, 6, 110, 20, NOTE_DS * OCT_4, QUART_BEAT},
    };

    std::vector<beatEntry> bar2 = {
        {0, 0, 20, 50, NOTE_AS * OCT_3, QUART_BEAT},
        {2, 2, 50, 50, NOTE_DS * OCT_3, ONE_BEAT},
        {4, 4, 80, 50, NOTE_AS * OCT_3, QUART_BEAT},
        {5, 5, 100, 50, NOTE_AS * OCT_3, QUART_BEAT},
        {6, 6, 120, 50, NOTE_FS * OCT_3, QUART_BEAT}
    };

    std::vector<beatEntry> bar3 = {
        {0, 0, 20, 80, NOTE_DS * OCT_3, QUART_BEAT},
        {1, 1, 40, 80, NOTE_DS * OCT_3, QUART_BEAT},
        {4, 4, 80, 80, NOTE_FS * OCT_3, QUART_BEAT},
        {5, 5, 100, 80, NOTE_AS * OCT_3, QUART_BEAT}
    };

    std::vector<beatEntry> bar4 = {
        {0, 0, 20, 110, NOTE_F * OCT_3, QUART_BEAT},
        {2, 2, 50, 110, NOTE_F * OCT_3, QUART_BEAT},
        {4, 4, 80, 110, NOTE_F * OCT_3, QUART_BEAT},
        {5, 5, 100, 110, NOTE_D * OCT_3, QUART_BEAT},
        {6, 6, 120, 110, NOTE_FS * OCT_3, QUART_BEAT}
    };

    std::vector<beatEntry> bar5 = {
        {0, 1, 20, 50, NOTE_F * OCT_3, QUART_BEAT}
    };
    */

    
    
    

//play backing melody

    /*
    std::vector<beatEntry> bar1 = {
        {1, 1, 20, 20, NOTE_DS * OCT_3, ONE_BEAT},
        {2, 2, 50, 20, NOTE_FS * OCT_3, ONE_BEAT},
        {3, 3, 80, 20, NOTE_AS * OCT_3, ONE_BEAT},
        {4, 4, 110, 20, NOTE_DS * OCT_4, HALF_BEAT},
    };
    std::vector<beatEntry> bar2 = {
        {1, 1, 20, 50, NOTE_AS * OCT_3, HALF_BEAT},
        {2, 2, 50, 50, NOTE_DS * OCT_3, ONE_BEAT},
        {3, 3, 80, 50, NOTE_AS * OCT_3, HALF_BEAT},
        {4, 4, 110, 50, NOTE_FS * OCT_3, ONE_BEAT}
    };
    std::vector<beatEntry> bar3 = {
        {1, 1, 20, 50, NOTE_DS * OCT_3, ONE_BEAT},
        {2, 2, 50, 50, NOTE_FS * OCT_3, ONE_BEAT},
        {3, 3, 80, 50, NOTE_AS * OCT_3, ONE_BEAT},
        {4, 4, 110, 50, NOTE_GS * OCT_3, HALF_BEAT}
    };
    std::vector<beatEntry> bar4 = {
        {1, 1, 20, 50, NOTE_F * OCT_3, HALF_BEAT},
        {2, 2, 50, 50, NOTE_F * OCT_3, HALF_BEAT},
        {3, 3, 80, 50, NOTE_F * OCT_3, ONE_BEAT},
        {4, 4, 110, 50, NOTE_D * OCT_3, HALF_BEAT}
    };
    */
    

    std::vector<std::vector<beatEntry>> beatMap = {
        bar1
    };


    std::vector<BeatInteractable*> spawnedBeats;

    public:
    RhythmPath() {
        
    }
   
    void OnBeat(songPosition pos) {
        //int beat = (globalBeat) % 4 + 1;
        //int bar = (globalBeat) / 4;
        trySpawnBeat(pos);
    }

    /*
    need a better way of finding status of beats, including whether or not they were hit on time.
    this is too tightly coupled and we need the same logic to work in different objects
    all this function basically does is go through each active beat, find what its progress is (yet to hit? ready to hit? past being hit? in slider?)
    and then performs some game logic based on that. we want to remove the game logic and just provide information to figure out how to handle each beat.
    struct with beat start time (this is unique), progress of that beat whether or not beat was hit (start note in case of slider).
    if slider, we say whether or not pen is touching the right part (why are these 2 different functions)
    isPenTouching() is current state as obj knows it , while isHit() checks and updates this value. so history. probably useful unless if can refactor enough (should do)
    */

    void renderBeats(songPosition pos) {
        //rather than handling EVERYTHING (kills, music, combo) in here should probably try and get a status that other things can work with
        int beat = pos.globalBeat * pos.numSubBeats + pos.subBeat;
        int progressToNext = pos.subBeatProgress;

        for (size_t i = 0; i < spawnedBeats.size(); i++) {
            BeatInteractable* b = spawnedBeats[i];

            int p = b->getBeatProgress(beat, progressToNext, margin);
            if (b->isActive()) b->render(p);
        }

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

    std::vector<playableBeatStatus> getBeatStates(songPosition pos, int touchX, int touchY) {
        std::vector<playableBeatStatus> states;

        int beat = pos.globalBeat * pos.numSubBeats + pos.subBeat;
        //int beat = pos.globalBeat;
        int progressToNext = pos.subBeatProgress;
        //int progressToNext = pos.globalBeatProgress;

        for (size_t i = 0; i < spawnedBeats.size(); i++) {
            BeatInteractable* b = spawnedBeats[i];
            int p = b->getBeatProgress(beat, progressToNext, margin);
            
            bool isHit = b->isHit(touchX, touchY);

            if (isHit) {
                if (b->isSlider()) { //for slider a hit needs to be in the right place, not just anywhere along it
                    //if early we still want to log (even invalid) hits
                    bool rightPlace = b->isPenInRightPlace(beat, progressToNext, margin, touchX, touchY);
                    if (!rightPlace) isHit = false;
                } 
            }

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

    //TODO: better memory management for this
    void deactivateBeat(int beatStart, int currBeat) {
        //return;
        for (size_t i = 0; i < spawnedBeats.size(); i++) {
            if (spawnedBeats[i]->getStartBeat() == beatStart) {
                spawnedBeats[i]->deactivate(currBeat);
                //spawnedBeats.erase(spawnedBeats.begin() + i);
                //i--;
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

    int localToGlobalBeat(int localBeat, songPosition pos) {
        int localToGlobal = localBeat + pos.bar * pos.numBeatsInBar * pos.numSubBeats;
        return localToGlobal;
    }
    //a beat starting at beat n should be spawned at beat n - beatLookAhead. if this < 0 then beatManager needs to lower its starting beat.
    void trySpawnBeat(songPosition pos) {
        int bar = pos.bar;
        //int localBeat = pos.localBeat * pos.numSubBeats + pos.subBeat;
        //int localBeat = pos.localBeat + 2;
        int globalBeat = (pos.globalBeat + beatLookAhead) * pos.numSubBeats + pos.subBeat;
        //int globalBeat = pos.globalBeat + 2;

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
            //std::cout << "spawn beat global " << globalBeat << " beat " << beat << " bar " << bar << std::endl;

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

   
};


#endif // RHYTHMPATH_H
