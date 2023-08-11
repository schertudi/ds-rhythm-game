

#include "levelParser.h"
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include "noteDefinitions.h"
#include "constants.h"
#include "debugTools.h"

namespace LevelDataParser {
    namespace { //anonymous namespace encapsulates functions so other files don't see them

        struct beatsInBar { //literally just exists to make the type less of a mouthful as is repeated a lot
            std::vector<std::vector<std::string>> beatConfigs;
        };

        struct barConfig {
            beatsInBar bar;
            int energyLevel=-1; //-1 here means just continue from previous
        };

        std::vector<barConfig> getLevelData() {
            beatsInBar bar1 = {{
                {"single 20,50 monotone quart", "1 burstingBeat"},
                {"none"},
                {"single 50,50 monotone quart", "2 burstingBeat"},
                {"none"},
                {"single 80,50 monotone quart"},
                {"none"},
                {"single 110,50 monotone quart"} 
                //number before animation indicates lowest possible level needed for this to play
            }};

            beatsInBar bar2 = {{
                {"single 20,100 monotone quart", "1 burstingBeat", "2 sineWave top 4", "3 sineWave bottom 4"},
                {"none"},
                {"none"},
                {"none"},
                {"single 80,100 monotone quart", "3 burstingBeat"}
            }};

            beatsInBar bar3 = {{
                {"slider 20,100 50,100 2 monotone quart"},
                {"none"},
                {"none"},
                {"none"},
                {"single 80,100 monotone quart"}
            }};

            std::vector<barConfig> song = {
            //if win combo we incr energy level, if lose we do nothing (or could decr). when energy level drops (eg 3 -> 1) we reset it to new val.
                {bar3, 1}, 
                {bar2},
                {bar2, 2}, 
                {bar2},
                {bar2},
                {bar2, 3},
                {bar2},
                {bar2, 1}, 
                {bar2}, 
            };

            return song;
        }

        std::vector<std::string> splitWords(std::string sentence) { //will always have length of at least 1 due to ""
            std::string word = "";
            std::vector<std::string> words;
            for (size_t i=0; i < sentence.length(); i++) {
                if (sentence[i] == ' ' || sentence[i] == ',') {
                    words.push_back(word);
                    word = "";
                } else {
                    word += sentence[i];
                }
                
            }
            words.push_back(word);
            return words;
        }

        int strToInt(std::string str) {
            //might be good to check inputs here (currently crashes on bad input), but ds doesn't like to compile with try/catch and i am too scared to change defaults
            return std::stoi(str, nullptr, 10);
        }

        int getPitch(std::string pitchStr) {
            //note is something in form of "c3, fs2, db4"
            if (pitchStr == "monotone") {
                return 0;
            }
            std::string noteStr;
            std::string octaveStr;
            if (pitchStr.length() == 2) {
                noteStr = pitchStr.substr(0, 1);
                octaveStr = pitchStr[1];
            } else if (pitchStr.length() == 3) {
                noteStr = pitchStr.substr(0, 2);
                octaveStr = pitchStr[2];
            } else {
                //error
                Debugger::error("bad pitch %s", pitchStr.c_str());
                return -1;
            }
            
            //so ugly, but at least it's straightforward..
            int noteInt;
            if (noteStr == "c") { noteInt = NOTE_C; } 
            else if (noteStr == "cs") { noteInt = NOTE_CS; } 
            else if (noteStr == "db") { noteInt = NOTE_Db; }
            else if (noteStr == "d") { noteInt = NOTE_Db; }
            else if (noteStr == "ds") { noteInt = NOTE_DS; }
            else if (noteStr == "eb") { noteInt = NOTE_Eb; }
            else if (noteStr == "e") { noteInt = NOTE_E; }
            else if (noteStr == "f") { noteInt = NOTE_F; }
            else if (noteStr == "fs") { noteInt = NOTE_FS; }
            else if (noteStr == "gb") { noteInt = NOTE_Gb; }
            else if (noteStr == "g") { noteInt = NOTE_G; }
            else if (noteStr == "gs") { noteInt = NOTE_GS; }
            else if (noteStr == "ab") { noteInt = NOTE_Ab; }
            else if (noteStr == "a") { noteInt = NOTE_A; }
            else if (noteStr == "as") { noteInt = NOTE_AS; }
            else if (noteStr == "bb") { noteInt = NOTE_Bb; }
            else if (noteStr == "b") { noteInt = NOTE_B; }
            else { 
                Debugger::error("bad pitch %s", pitchStr.c_str());
                return -1; 
            }

            int octaveInt;
            if (octaveStr == "0") { octaveInt = OCT_0; }
            else if (octaveStr == "1") { octaveInt = OCT_1; }
            else if (octaveStr == "2") { octaveInt = OCT_2; }
            else if (octaveStr == "3") { octaveInt = OCT_3; }
            else if (octaveStr == "4") { octaveInt = OCT_4; }
            else if (octaveStr == "5") { octaveInt = OCT_5; }
            else if (octaveStr == "6") { octaveInt = OCT_6; }
            else { 
                Debugger::error("bad pitch %s", pitchStr.c_str());
                return -1; 
            }

            return noteInt * octaveInt;
        }

        int getSoundLength(std::string str) {
            if (str == "quart") { return QUART_BEAT; }
            else if (str == "half") { return HALF_BEAT; }
            else if (str == "one") { return ONE_BEAT; }
            else if (str == "two") { return TWO_BEAT; }
            else if (str == "three") { return THREE_BEAT; }
            else if (str == "four") { return FOUR_BEAT; }
            Debugger::error("bad len %s", str.c_str());
            return -1;
        }

        direction getDirection(std::string str) {
            if (str == "top") { return direction::TOP; }
            else if (str == "bottom") { return direction::BOTTOM; }
            else if (str == "left") { return direction::LEFT; }
            else if (str == "right") { return direction::RIGHT; }
            Debugger::error("bad dir %s", str.c_str());
            return direction::NONE; //error
        }

        HitBeatAbstract* parseBeatConfigSection(std::string config, int startBeat) {
            /*
            possible inputs:
            single beatPos {pitch|monotone} audioLength
            slider beatStart beatEnd beatLength {pitch|monotone} audioLength
            pitch is something in form of "c3", "fs2"
            */
            
            std::vector<std::string> split = splitWords(config);

            if (split[0] == "none") { //allowed to not define an interaction on this beat, just have to be explicit
                return nullptr; 
            }

            if (split[0] == "single") {
                if (split.size() != 5) {
                    Debugger::error("tokens != 5 b%i: %s", startBeat, config.c_str());
                    return nullptr;
                }
                int x = strToInt(split[1]);
                int y = strToInt(split[2]);
                int pitch = getPitch(split[3]); //would need to convert this if not nopitch, todo
                int soundLength = getSoundLength(split[4]); //need to convert to enum or const 
                SingleHitBeat* newBeat = new SingleHitBeat(startBeat, x, y, soundLength, pitch);
                return newBeat;
            } 
            else if (split[0] == "slider") {
                if (split.size() != 8) {
                    Debugger::error("tokens != 8 b%i: %s", startBeat, config.c_str());
                    return nullptr;
                }
                int startX = strToInt(split[1]);
                int startY = strToInt(split[2]);
                int endX = strToInt(split[3]);
                int endY = strToInt(split[4]);
                int numBeats = strToInt(split[5]); //add current beat to this
                int pitch = getPitch(split[6]);
                int soundLength = getSoundLength(split[7]);
                SliderHitBeat* newBeat = new SliderHitBeat(startBeat, startBeat + numBeats,
                        startX, startY, endX, endY, 
                        soundLength, pitch);
                return newBeat;
                
            }
            //error!
            Debugger::error("bad b%i: %s", startBeat, config.c_str());
            return nullptr;
        }

        AnimationCommand* parseAnimationConfigSection(std::string config, int startBeat) { 
            /*
            possible inputs:
            sineWave direction beatLength
            fillTank numBeats beatGap
            slideStarfish
            throwBall landPos
            colourSlider startColour endColour beatsList (how do we do this??? F)
            diagonalBall numBeats beatGap
            burstingBeat
            dancingStarfish

            return AnimationCommand* created using specific class requested
            */
            
            std::vector<std::string> split = splitWords(config);

            if (split[1] == "sineWave" && split.size() == 4) {
                int energy = strToInt(split[0]);
                direction dir = getDirection(split[2]); //need to convert to directional enum from split
                int beatLength = strToInt(split[3]);
                AnimationCommand* anim = new SineWaveAnimation(energy, startBeat, startBeat + beatLength, dir);
                return anim;
            } 
            else if (split[1] == "fillTank" && split.size() == 4) {
                int energy = strToInt(split[0]);
                int numBeats = strToInt(split[2]);
                int beatGap = strToInt(split[3]);
                AnimationCommand* anim = new FillTankAnimation(energy, startBeat, numBeats, beatGap);
                return anim;
            } 
            else if (split[1] == "slideStarfish" && split.size() == 2) {
                //just spawn with starting beat
                int energy = strToInt(split[0]);
                AnimationCommand* anim = new SlidingStarfishAnimation(energy, startBeat);
                return anim;
            } 
            else if (split[1] == "throwBall" && split.size() == 4) {
                int energy = strToInt(split[0]);
                int landX = strToInt(split[2]);
                int landY = strToInt(split[3]);
                AnimationCommand* anim = new ThrowingBallAnimation(energy, startBeat, {landX, landY});
                return anim;
            } 
            else if (split[1] == "diagonalBouncingBall" && split.size() == 4) {
                int energy = strToInt(split[0]);
                int numBeats = strToInt(split[2]);
                int beatGap = strToInt(split[3]);
                AnimationCommand* anim = new DiagonalBouncingBallAnimation(energy, startBeat, numBeats, beatGap);
                return anim;
            } 
            else if (split[1] == "burstingBeat" && split.size() == 2) {
                int energy = strToInt(split[0]);
                AnimationCommand* anim = new BurstingBeatAnimation(energy, startBeat, startBeat + 1);
                return anim;
            } 
            else if (split[1] == "dancingStarfish" && split.size() == 2) {
                int energy = strToInt(split[0]);
                AnimationCommand* anim = new DancingStarfishAnimation(energy, startBeat);
                return anim;
            }
            Debugger::error("bad anim b%i: %s", startBeat, config.c_str());
            return nullptr;
        }

        void parseLine(std::vector<std::string> configs, int beat, std::vector<HitBeatAbstract*>* interacts, std::vector<AnimationCommand*>* animations) {
            /*
            takes input such as {"single 10,10 nopitch quart", "diagonalBouncingBall 8 2", "sineWave top 2"}
            and add these to their respective lists - first item is beat config which goes to interacts, next are animations
            */

            HitBeatAbstract* i = parseBeatConfigSection(configs[0], beat);
            if (i) {
                interacts->push_back(i);
            } else {
                //if no beat here, could add a nullptr (inefficent space-wise, but fast). currently not done but may be worth considering later.
                //interacts->push_back(nullptr);
            } 

            for (size_t i = 1; i < configs.size(); i++) {
                AnimationCommand* a = parseAnimationConfigSection(configs[i], beat);
                if (a) {
                    animations->push_back(a);
                } else {
                    Debugger::print("no anim on b%i from i=%i", beat, i); //nonexistent animtion should be skipped, nullptr indicates invalid input
                    return;
                }
            }

        }

        void parseSong(std::vector<barConfig> song, std::vector<HitBeatAbstract*>* interacts, std::vector<AnimationCommand*>* animations, int numBeatsInBar) {
            for (size_t bar = 0; bar < song.size(); bar++) {
                for (size_t localBeat = 0; localBeat < song[bar].bar.beatConfigs.size(); localBeat++) {
                    int beat = localBeat + numBeatsInBar * bar;
                    parseLine(song[bar].bar.beatConfigs[localBeat], beat, interacts, animations);
                }
            }

        }

    }

    levelData setup(int numBeatsInBar) {
        std::vector<barConfig> song = getLevelData();

        //init on heap as these will contain a fair amount of data; 1+ objs need to be made for every subbeat that will be hit or have an animation
        //apparently vectors store info on heap anyway? ok..
        std::vector<HitBeatAbstract*> interacts = std::vector<HitBeatAbstract*>();
        std::vector<AnimationCommand*> animations = std::vector<AnimationCommand*>();
        std::vector<int> energyLevels = std::vector<int>();

        int currEnergy = 1;
        for (size_t i = 0; i < song.size(); i++) {
            int barEnergy = song[i].energyLevel;
            if (barEnergy == -1) {
                barEnergy = currEnergy;
            } else {
                currEnergy = barEnergy;
            }
            energyLevels.push_back(barEnergy);
        }

        parseSong(song, &interacts, &animations, numBeatsInBar);

        levelData l = {interacts, animations, energyLevels};
        return l;
    }

}