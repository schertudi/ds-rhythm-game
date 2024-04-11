#pragma once
#include "sysWrappers/audioPlayer.h"

enum class sceneStates {CURRENT, MAIN_MENU, GAME, TUTORIAL, SETTINGS};

class SceneObject {

    public:
    virtual void load(AudioPlayer _audioPlayer) = 0;
    virtual sceneStates update() = 0;
    virtual void unload() = 0;

};