#pragma once

enum class sceneStates {CURRENT, MAIN_MENU, GAME, TUTORIAL, SETTINGS};

class SceneObject {

    public:
    virtual void load() = 0;
    virtual sceneStates update() = 0;
    virtual void unload() = 0;

};