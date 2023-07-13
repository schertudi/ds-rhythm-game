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
    int combo = 0;
    bool isComputerPlaying = false;
    AudioManager audioManager;

    
    //simple test
    std::vector<beatEntry> bar1 = {
        {0, 0, 20, 20, NOTE_C * OCT_3, QUART_BEAT},
        {2, 2, 50, 20, NOTE_FS * OCT_3, QUART_BEAT}
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
        bar1, bar2
    };


    std::vector<BeatInteractable*> spawnedBeats;

    public:
    RhythmPath(AudioManager _man) {
        audioManager = _man;
        audioManager.setOctave(OCT_5);
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
            b->render(p);
        }

    }

    std::vector<playableBeatStatus> getBeatStates(songPosition pos, int touchX, int touchY) {
        std::vector<playableBeatStatus> states;

        int beat = pos.globalBeat * pos.numSubBeats + pos.subBeat;
        int progressToNext = pos.subBeatProgress;

        for (size_t i = 0; i < spawnedBeats.size(); i++) {
            BeatInteractable* b = spawnedBeats[i];
            int p = b->getBeatProgress(beat, progressToNext, margin);
            
            bool isHit = b->isHit(touchX, touchY);

            if (isHit) {
                if (b->isSlider()) { //for slider a hit needs to be in the right place, not just anywhere along it
                    bool rightPlace = b->isPenInRightPlace(beat, progressToNext, margin, touchX, touchY);
                    if (!rightPlace) isHit = false;
                } 
            }

            playableBeatStatus beatState;
            beatState.beatStart = b->getStartBeat();
            beatState.progress = p;
            beatState.isHit = isHit;

            if (b->isSlider()) {
                //beatState.isReadyForHit = p >= 100 && p <= 200;
                //beatState.isSliderReadyForLift = p == 200;
                beatState.isSlider = true;
            } else {
                //beatState.isReadyForHit = p == 100;
                beatState.isSlider = false;
            }
            
            states.push_back(beatState);
        }

        return states;
    }

    //TODO: better memory management for this
    void killBeat(int beatStart) {
        for (size_t i = 0; i < spawnedBeats.size(); i++) {
            if (spawnedBeats[i]->getStartBeat() == beatStart) {
                spawnedBeats.erase(spawnedBeats.begin() + i);
                i--;
            }
        }
    }

    void tryPlaySound(int beatStart, AudioManager man) {
        for (size_t i = 0; i < spawnedBeats.size(); i++) {
            if (spawnedBeats[i]->getStartBeat() == beatStart) {
                spawnedBeats[i]->tryPlaySound(man);
            }
        }
    }

    void resetSound(int beatStart) {
        for (size_t i = 0; i < spawnedBeats.size(); i++) {
            if (spawnedBeats[i]->getStartBeat() == beatStart) {
                spawnedBeats[i]->resetSound();
            }
        }
    }

    void trySpawnBeat(songPosition pos) {
        int bar = pos.bar;
        int localBeat = pos.localBeat * pos.numSubBeats + pos.subBeat;
        int globalBeat = pos.globalBeat * pos.numSubBeats + pos.subBeat + beatLookAhead * pos.numSubBeats;

        if (bar >= (int)beatMap.size()) {
            return;
        }
        std::vector<beatEntry> beats = beatMap[bar];

        int index = -1;
        for (size_t i = 0; i < beats.size(); i++) {
            if (beats[i].beatStart == localBeat) {
                index = i;
                break;
            }
        }

        if (index == -1) {
            return;
        }

        if (beats[index].beatEnd == localBeat) {
            //std::cout << "spawn beat global " << globalBeat << " beat " << beat << " bar " << bar << std::endl;

            // spawn beat
            BeatToHit* newBeat = new BeatToHit(globalBeat, beats[index].x, beats[index].y, beats[index].length, beats[index].pitch);
            spawnedBeats.push_back(newBeat);
            return;
        } else {
            //spawn beatLine
            BeatToSlide* newBeat = new BeatToSlide(globalBeat, globalBeat + (beats[index].beatEnd - beats[index].beatStart),
                beats[index].x, beats[index].y, beats[index].endX, beats[index].endY, beats[index].length, beats[index].pitch);
            spawnedBeats.push_back(newBeat);
        }

    }

    int getCombo() {
        return combo;
    }
   
};


#endif // RHYTHMPATH_H
