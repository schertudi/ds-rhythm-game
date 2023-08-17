#pragma once


#include "constants.h"
#include "../sysWrappers/audioPlayer.h"


class HitBeatAbstract {
    private:
    bool _isSlider = false;
    playerStatus playerState = playerStatus::IDLE;
    int timeAtDeactivate;
    bool isDead = false;

    protected:
    void setSlider(bool value);

    public:
    virtual void render(int progress) = 0; //<100 means approaching, 100 means at beat, >100 means passed
    virtual bool isHit(int touchX, int touchY, int globalBeat, int progressToNext, int margin) = 0; //mind radius on this one, can wrongly detect collisions if too big
    virtual int getStartBeat() = 0;
    virtual int getEndBeat() = 0; //unused.. for now?
    virtual Vec2d getStartPos() = 0;
    virtual Vec2d getEndPos() = 0;
    virtual int getBeatProgress(int globalBeat, int progressToNext, int margin) = 0;
    virtual void playSound(AudioPlayer man) = 0;

    void deactivate(int beat);

    bool isActive() const;

    int getTimeSinceDeactivate(int beat);

    bool isSlider() const;

    void setPlayerState(playerStatus state);

    playerStatus getPlayerState() const;
};

class SingleHitBeat : public HitBeatAbstract {
    private:
    int beat;
    int x;
    int y;
    int radius;
    int length;
    int pitch;
    
    public:
    SingleHitBeat(int _beat, int _x, int _y, int _len, int _pitch);

    void render(int progress) override;

    bool isHit(int touchX, int touchY, int globalBeat, int progressToNext, int margin) override;

    int getStartBeat() override;

    int getEndBeat() override;

    Vec2d getStartPos() override;

    Vec2d getEndPos() override;

    int getBeatProgress(int globalBeat, int progressToNext, int margin) override;

    void playSound(AudioPlayer man) override;

};


class SliderHitBeat : public HitBeatAbstract {
    private:
    int startBeat;
    int endBeat;
    int startX;
    int startY;
    int endX;
    int endY;
    const int startRadius = 15;
    const int slideRadius = 30;
    int lengthInBeats;
    int noteLength;
    int pitch;

    public:
    SliderHitBeat(int _startBeat, int _endBeat, int _startX, int _startY, int _endX, int _endY, int _len, int _pitch);

    void render(int progress) override;

    bool isHit(int touchX, int touchY, int globalBeat, int progressToNext, int margin) override;

    int getStartBeat() override;

    int getEndBeat() override;

    Vec2d getStartPos() override;

    Vec2d getEndPos() override;

    int getBeatProgress(int globalBeat, int progressToNext, int margin) override;

    void playSound(AudioPlayer man) override;
};