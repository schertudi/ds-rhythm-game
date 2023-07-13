#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <maxmod9.h>  
#include <mm_types.h>
#include "soundbank.h" 
#include "soundbank_bin.h"
#include "noteDefinitions.h"



class AudioManager {

    private:
        bool isFirst;
        int octave = OCT_4; //base octave
        bool usePitch = false;

    public: 
    AudioManager() {
        //access soundbank that is loaded into memory
	    mmInitDefaultMem( (mm_addr)soundbank_bin );
        //mmSelectMode(MM_MODE_C);
        mmLoad( MOD_TRACKING );

        mmLoadEffect(SFX_CLICK1);
	    mmLoadEffect(SFX_CLICK2);
        mmLoadEffect(SFX_C4_1_BEAT);
        mmLoadEffect(SFX_C4_2_BEATS);
        mmLoadEffect(SFX_C4_3_BEATS);
        mmLoadEffect(SFX_C4_4_BEATS);
        mmLoadEffect(SFX_C4_HALF_BEAT);
        mmLoadEffect(SFX_C4_QUART_BEAT);

        isFirst = true;
    }

    void startMusic() {
        //mmStart(MOD_TRACKING, MM_PLAY_ONCE);
    }

    void metronome(int beat) {
        mmEffectCancel(SFX_CLICK1);
        mmEffectCancel(SFX_CLICK2);
        if (beat % 2 == 0) mmEffect(SFX_CLICK2);
        else mmEffect(SFX_CLICK1);
    }

    void playNote(int length, int note) {
        //mmEffect(SFX_CLICK2);
        //return;
        mm_sfxhand handle = 0;
        mmEffectCancel(SFX_C4_1_BEAT);
        mmEffectCancel(SFX_C4_2_BEATS);
        mmEffectCancel(SFX_C4_3_BEATS);
        mmEffectCancel(SFX_C4_4_BEATS);
        mmEffectCancel(SFX_C4_HALF_BEAT);
        mmEffectCancel(SFX_C4_QUART_BEAT);
        switch(length) {
            case 1: //1 beat
                handle = mmEffect(SFX_C4_1_BEAT);
                break;
            case 2: //2 beats
                handle = mmEffect(SFX_C4_2_BEATS);
                break;
            case 3: //3 beats
                handle = mmEffect(SFX_C4_3_BEATS);
                break;
            case 4: //4 beats
                handle = mmEffect(SFX_C4_4_BEATS);
                break;
            case 12: // 1/2 beat
                handle = mmEffect(SFX_C4_HALF_BEAT);
                break;
            case 14: // 1/4 beat
                handle = mmEffect(SFX_C4_QUART_BEAT);
                break;
            default:
                return;
                break;
            }

        if (usePitch) mmEffectRate(handle, note / 100);
        //mmEffectRelease(handle);
    }

    void setOctave(int _octave) {
        octave = _octave;
    }
};

#endif // AUDIOMANAGER_H