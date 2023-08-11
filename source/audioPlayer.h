#pragma once
#include "noteDefinitions.h"


class AudioPlayer {

    private:
        bool isFirst;
        int octave = OCT_4; //base octave
        bool usePitch = false;

    public: 
    AudioPlayer();

    void startMusic();

    void metronome(int beat);

    void playNote(int length, int note);

    void setOctave(int _octave);
};

