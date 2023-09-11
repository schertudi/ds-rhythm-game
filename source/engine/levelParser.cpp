

#include "levelParser.h"
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include "../noteDefinitions.h"
#include "engineTypes.h"
#include "../sysWrappers/debugTools.h"

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
            beatsInBar intro1 = {{
                {"single 88,96 monotone quart"},
                {"none"},
                {"single 168,96 monotone quart"},
                {"none"},
                {"single 88,121 monotone quart"},
                {"none"},
                {"single 128,121 monotone quart"},
                {"single 168,121 monotone quart"},
            }};

            beatsInBar intro2 = {{
                {"single 168,96 monotone quart"},
                {"none"},
                {"single 88,96 monotone quart"},
                {"none"},
                {"single 168,71 monotone quart"},
                {"none"},
                {"single 128,71 monotone quart"},
                {"single 88,71 monotone quart"},
            }};

            beatsInBar intro3 = {{
                {"single 88,96 monotone quart"},
                {"none"},
                {"single 168,96 monotone quart"},
                {"none"},
                {"single 128,121 monotone quart"}
            }};

            
            beatsInBar verse1 = {{
                {"single 32,128 monotone quart", "1 diagonalBouncingBall 2 1"},
                {"single 64,128 monotone quart"},
                {"single 96,128 monotone quart"},
                {"none"},
                {"single 160,128 monotone quart", "1 diagonalBouncingBall 3 1"},
                {"single 192,128 monotone quart"},
                {"single 224,128 monotone quart"},
            }};
            

            /*
            beatsInBar verse1 = {{
                {"single 32,128 monotone quart", "1 diagonalBouncingBall 4 2"},
                {"none"},
                {"single 96,128 monotone quart"},
                {"none"},
                {"single 160,128 monotone quart"},
                {"none"},
                {"single 224,128 monotone quart"},
            }};
            */

            beatsInBar verse2 = {{
                {"single 224,98 monotone quart"},
                {"none"},
                {"single 192,128 monotone quart"},
                {"none"},
                {"slider 128,98 70,98 2 monotone quart", "1 throwBall 32,128 1"}
            }};

            beatsInBar verse3 = {{
                {"slider 188,76 68,76 6 monotone quart", "1 slideStarfish"}
            }};

             beatsInBar verse4 = {{
                {"single 192,96 monotone quart", "1 dancingStarfish"},
                {"none"},
                {"single 128,76 monotone quart", "1 dancingStarfish"},
                {"none"},
                {"single 64,96 monotone half", "1 burstingBeat"},
            }};

            
            std::vector<barConfig> song = {
                {verse2, 1},
                {verse1}
            };
            

            /*
            std::vector<barConfig> song = {
                {intro1, 1},
                {intro2},
                {intro1},
                {intro3},
                {verse1},
                {verse2},
                {verse1},
                {verse3},
                {verse1},
                {verse2},
                {verse1},
                {verse4}
            };
            */

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
                int pitch = getPitch(split[3]);
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
            else if (split[1] == "throwBall" && split.size() == 5) {
                int energy = strToInt(split[0]);
                int landX = strToInt(split[2]);
                int landY = strToInt(split[3]);
                int throwTime = strToInt(split[4]);
                AnimationCommand* anim = new ThrowingBallAnimation(energy, startBeat, {landX, landY}, throwTime);
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