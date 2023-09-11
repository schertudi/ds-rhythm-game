
#include "audioPlayer.h"
#include <maxmod9.h>  
#include <mm_types.h>
#include <nds/ndstypes.h>
#include "soundbank.h" 
#include "soundbank_bin.h"
#include "debugTools.h"
#include <limits>

mm_word myEventHandler( mm_word msg, mm_word param ) {
    
	switch(msg ){

	case MMCB_SONGMESSAGE:	// process song messages
		Debugger::print("EVENT!! %d", param);
        
        break;
		
    case MMCB_SONGFINISHED:	// process song finish message (only triggered in songs played with MM_PLAY_ONCE)
        Debugger::print("FINISHED!!");
		break;
    }
	
	return 0;
}


AudioPlayer::AudioPlayer() {
    //access soundbank that is loaded into memory
    mmInitDefaultMem( (mm_addr)soundbank_bin );
    //mmSelectMode(MM_MODE_C);
    mmLoad( MOD_IEVAN2 );

    mmLoadEffect(SFX_CLICK1);
    mmLoadEffect(SFX_CLICK2);
    mmLoadEffect(SFX_C4_1_BEAT);
    mmLoadEffect(SFX_C4_2_BEATS);
    mmLoadEffect(SFX_C4_3_BEATS);
    mmLoadEffect(SFX_C4_4_BEATS);
    mmLoadEffect(SFX_C4_HALF_BEAT);
    mmLoadEffect(SFX_C4_QUART_BEAT);

    mmSetEventHandler(myEventHandler);

    mmSelectMode(MM_MODE_A);
    mm_word allLocked = std::numeric_limits<mm_word>::max();
    //appears that sounds are dynamically allocated to channel, possibly using some kind of queue, rather than a consistent mapping from tracker channels :(
    //this also creates conflicts with beat sfx if that uses maxmod as well
    //so will probably need some kind of travel through mod file for dynamic sound... err.
    //or at least play a 2nd mod file in sync
    //mmLockChannels(BIT(0) | BIT(1) | BIT(3) | BIT(4) | BIT(5) | BIT(6) | BIT(7) | BIT(8) | BIT(9) | BIT(10) | BIT(11) | BIT(12) | BIT(13) | BIT(14) | BIT(15) | BIT(16));
    //mmLockChannels(BIT(0));
    //mmPause();

    isFirst = true;
}

void AudioPlayer::startMusic() {
    mmSetModuleVolume(1024 / 2); //50% volume
    
    
    
    mmStart(MOD_IEVAN2, MM_PLAY_ONCE);
}

void AudioPlayer::metronome(int beat) {
    mmEffectCancel(SFX_CLICK1);
    mmEffectCancel(SFX_CLICK2);
    if (beat % 2 == 0) mmEffect(SFX_CLICK2);
    else mmEffect(SFX_CLICK1);
}

void AudioPlayer::playNote(int length, int note) {
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

void AudioPlayer::setOctave(int _octave) {
    octave = _octave;
}