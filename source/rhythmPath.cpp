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
#include "debugTools.h"

class RhythmPath {
    /*
    in path we need x,y pos, beat at which it is triggered, beat it ends at, trigger for animation if used
    */
    private:
    int beatLookAhead;
    int margin; // if we are 10% close to beat or 10% past this is still correct
    std::vector<BeatInteractable*> beatInteractObjs;
    std::vector<BeatInteractable*> spawnedBeats;

    public:
    RhythmPath(int _lookahead, int _margin, std::vector<BeatInteractable*> _beatInteractObjs) {
        beatLookAhead = _lookahead;
        margin = _margin;
        beatInteractObjs = _beatInteractObjs;
    }
   
    void onBeat(songPosition pos) {
        activateCurrentBeat(pos);
        //is just based on a timer, will remove items from list 5 beats after they were deactivated
		killInactiveBeats(pos.globalBeat, 5);
    }

    std::vector<playableBeatStatus> getBeatStates(songPosition pos, int touchX, int touchY, bool isAutomatedPlay) {
        std::vector<playableBeatStatus> states;
        int beat = pos.globalBeat * pos.numSubBeats + pos.subBeat;
        int progressToNext = pos.subBeatProgress;

        for (size_t i = 0; i < spawnedBeats.size(); i++) {
            BeatInteractable* b = spawnedBeats[i];
            int p = b->getBeatProgress(beat, progressToNext, margin);
            
            bool isHit = b->isHit(touchX, touchY, beat, progressToNext, margin);
            if (isAutomatedPlay) {
                isHit = p >= 100 && p <= 200;
            }

            playableBeatStatus beatState;
            beatState.beatStart = b->getStartBeat();
            beatState.beatEnd = b->getEndBeat();
            beatState.timingProgress = p;
            beatState.isHit = isHit;
            beatState.oldPlayerState = b->getPlayerState();
            beatState.startPos = b->getStartPos();
            beatState.endPos = b->getEndPos();

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


    void playSound(int beatStart, AudioManager man) {
        for (size_t i = 0; i < spawnedBeats.size(); i++) {
            if (spawnedBeats[i]->getStartBeat() == beatStart) {
                spawnedBeats[i]->playSound(man);
            }
        }
    }

    private:

    void activateCurrentBeat(songPosition pos) {
        //get current index based on beat
        //slightly quicker method would be to do direct lookup beatInteractObjs[beat], but this relies on a bunch of nullpointers which takes more space
        int beat = (pos.globalBeat + beatLookAhead) * pos.numSubBeats + pos.subBeat;

        BeatInteractable* b = nullptr;
        for (size_t i = 0; i < beatInteractObjs.size(); i++) { //could start from last known index, just adds a little more complexity codewise
            if (beatInteractObjs[i]->getStartBeat() == beat) {
                b = beatInteractObjs[i];
                break;
            }

            if (beatInteractObjs[i]->getStartBeat() > beat) { //won't find anything now but can break out of loop a bit sooner
                break;
            }
        }

        if (!b) {
            return;
        }

        spawnedBeats.push_back(b);
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

        return os; //happens if no change in state detected
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
