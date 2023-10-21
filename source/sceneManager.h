#pragma once

#include "sceneObject.h"




class SceneManager {


    public:
    SceneManager();

    void newScene(sceneStates scene);

    void update();

    private:
    SceneObject* currentScene;
 
};