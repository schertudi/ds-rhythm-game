/*
probably need an actually good method of grouping properties of a certain beat together
so we have beat time, position, optional pitch, optional end time/position for slider (and extend this to any possible optional params), optional animation
the types of parameters will be context-dependent (eg provide more for a certain animation) so our structure needs to be very flexible
might honestly want a parser of sorts?
bar1:
[
	{"single 10,10 nopitch quart", "none"}
	{"single 20,10 nopitch half", "burstingBeat"}
	{"slider 2 30,10 50,10 fs3 quart", "throwingBall 70,20"} (slider for 2 beats from 30,10 to 50,10. uses throwing ball anim to land at 70,20)
	{"none", "none"}
]
bar2:
[
	{"single 10,10 nopitch quart", "diagonalBouncingBall 8 2"} (bouncing ball animation will start at this beat, persist for 8 beats, bouncing every 2 beats)
	{"single 20,10 nopitch quart", "none"}
	{"single 30,10 nopitch quart", "none"}
	{"single 40,10 nopitch quart", "burstingBeat"}
]

it could be we make a struct for each possible type of config..? not sure, need to store it in something that accepts a lot of random structs
which doesn't really make sense in c++ i think
unless if we have like {animationName, param1, param2}... but gets clunky, might be better off just having a list to contain enum tokens

note that animations can ideally interact with beats that exist past their bar, perhaps not super important to do this but would be good to have option
should not be too difficult to implement as everything uses global beat. would just be annoying to maintain in config.
an animation can also last for multiple beats, and we should probably allow multiple animations triggering on a beat. eg
	{"single 20,10 nopitch half", "burstingBeat", "sineWave top 4"} (trigger sine wave anim lasting for 4 beats here)
string manipulation might actually not be necessary - could have a big enum containing tokens

then putting it all together, we have list of bars in order along with their energy level
song:
[
	[bar1, 1]
	[bar2, 2]
	[bar1, .]
	[bar1, 1]
]

this format does however require use of a 4d list, which might be bad. idk. 
probably more memory efficient than creating objects for everything at start - this should be minimal amount of data needed to play song

at bpm 120, granularity of 2, we have 30 bars in a minute. so a 4 minute song might have 120 bars in total.
some of these bars will be repeated elements, but idk if compiler really cares. probably not.
song = [120][][][]
each bar will have 8 items signifying beat at each point (includes subbeats)
song = [120][8][][]
i guess the length of a bar item depends on how many animations we want defined at a beat.. lets say 3 max + note definition
song = [120][8][3][]
and then what's the max num of params we might have for something? maybe worst case 10 (i hope not)
song = [120][8][3][10]
120 * 8 * 3 * 10 = 28800 items. enum is 4 bytes, so we use up 4 * 28800 = 115200 bytes. this means 115.2 kb. 
which is still well within memory limits for ds, but a hefty chunk of data nonetheless. will need to be careful.

and i realise i forgot to define the energy level here. but i think doing it this way would lead to a lot of wasted memory.
alternatively i just have a second list saying what bar corresponds to what level. 
levels:
[
	{0, 1}, //level 1 at bar 0
	{5, 2} //level 2 at bar 5
	{10, 1} //level 1 at bar 10
]

or, because bars will likely be repeated a lot, i could define them as objects and just have a list of pointers to them defining song.
idk how much space is allocated to heap on nds. apparently heaps tend to be bigger though?

bar1* = new barMarkup([
	{"single 10,10 nopitch quart", "none"}
	{"single 20,10 nopitch half", "burstingBeat"}
	{"slider 2 30,10 50,10 fs3 quart", "throwingBall 70,20"} (slider for 2 beats from 30,10 to 50,10. uses throwing ball anim to land at 70,20)
	{"none", "none"}
])

levels:
[
	{bar1, 1}
]

what if we did do things by strings to make parsing more logical?
"BEAT: single 10,10; SOUND: nopitch quart; ANIMATION: diagonalBouncingBall 8 2; sineWave top 2; sineWave bottom 2" <- 24 bytes (112 chars??)
or
{"single 10,10 nopitch quart", "diagonalBouncingBall 8 2", "sineWave top 2", "sineWave bottom 2"} <- 96 bytes as array, 12 as vector (81 chars??)

as an enum we would need:
enum class tokens {single, nopitch, quart, diagonalBouncingBall, sineWave, top, bottom, slider, half, one, two, three, four};
and then {single, 10, 10, nopitch, quart, diagonalBouncingBall, 8, 2, sineWave, top, 2, sineWave, bottom, 2}
which doesn't even compile because of ints...... lol!

i think might be better to do a list of strings.... less processing to do and hopefully a bit more compact (idk)



*/

#pragma once

#include "levelData.h"
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include "beatToHit.cpp"
#include "animationCommand.cpp"
#include "noteDefinitions.h"
#include "constants.h"
#include "debugTools.h"

namespace levelDataParser {
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
                //number before animation indicates lowest possible level needed for this (TODO)
            }};

            beatsInBar bar2 = {{
                {"single 20,100 monotone quart", "1 burstingBeat", "2 sineWave top 4", "3 sineWave bottom 4"},
                {"none"},
                {"none"},
                {"none"},
                {"single 80,100 monotone quart", "3 burstingBeat"}
            }};

            std::vector<barConfig> song = {
                {bar2, 1}, 
                {bar2, 1},
                {bar2, 1},
                {bar2, 2}, 
                {bar2, 2},
                {bar2, 2},
                {bar2, 3},
                {bar2, 3},
                {bar2, 3},
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

        BeatInteractable* parseBeatConfigSection(std::string config, int startBeat) {
            /*
            possible inputs:
            single beatPos {pitch|monotone} audioLength
            slider beatStart beatEnd beatLength {pitch|monotone} audioLength
            pitch is something in form of "c3", "fs2"
            what do we return? probably a beatEntry, or could just cut to the chase and make beatInteractables. depends on memory i guess.
            might be simpler to just do a beatInteractable.
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
                BeatToHit* newBeat = new BeatToHit(startBeat, x, y, soundLength, pitch);
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
                BeatToSlide* newBeat = new BeatToSlide(startBeat, startBeat + numBeats,
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

        void parseLine(std::vector<std::string> configs, int beat, std::vector<BeatInteractable*>* interacts, std::vector<AnimationCommand*>* animations) {
            /*
            takes input such as {"single 10,10 nopitch quart", "diagonalBouncingBall 8 2", "sineWave top 2"}
            i guess add these to their respective lists
            so first item (beat config) goes to a list owned by rhythmPath (maybe pass it in?)
            and remaining items will be added to list for animationCommand
            */

            BeatInteractable* i = parseBeatConfigSection(configs[0], beat);
            if (i) {
                interacts->push_back(i);
            } else {
                //if no beat here, add a nullptr anyway (inefficent space-wise, but makes lookup code very simple, can optimise later if an issue)
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

        void parseSong(std::vector<barConfig> song, std::vector<BeatInteractable*>* interacts, std::vector<AnimationCommand*>* animations, int numBeatsInBar) {
            for (size_t bar = 0; bar < song.size(); bar++) {
                for (size_t localBeat = 0; localBeat < song[bar].bar.beatConfigs.size(); localBeat++) {
                    int beat = localBeat + numBeatsInBar * bar;
                    parseLine(song[bar].bar.beatConfigs[localBeat], beat, interacts, animations);
                    //beat++;
                }
            }

        }

    }

    levelData setup(int numBeatsInBar) {
        std::vector<barConfig> song = getLevelData();

        //init on heap as these will contain a fair amount of data; 1+ objs need to be made for every subbeat that will be hit or have an animation
        //apparently vectors store info on heap anyway? ok..
        std::vector<BeatInteractable*> interacts = std::vector<BeatInteractable*>();
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