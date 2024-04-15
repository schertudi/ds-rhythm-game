#pragma once

#include "sceneObject.h"
#include "sysWrappers/audioPlayer.h"




class SceneManager {


    public:
    SceneManager(AudioPlayer* audioPlayer);

    void newScene(sceneStates scene);

    void update();

    private:
    SceneObject* currentScene;
    AudioPlayer* audioPlayer;
 
};