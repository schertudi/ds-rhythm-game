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
#include <utility>
#include "beatToHit.cpp"
#include "animationCommand.cpp"

struct levelData {
	std::vector<BeatInteractable*> beatInteracts;
	std::vector<AnimationCommand*> animations;
	//should be able to lookup a bar and get a level. bar is seq of continous ints so can use array
	std::vector<int> perBarEnergyLevel;
};

namespace levelDataParser {
    levelData setup(int numBeatsInBar);
}

