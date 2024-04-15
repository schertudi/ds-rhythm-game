/*
this handles the loading/unloading of various parts of the game, decides when to update what
eg should i be updating main screen, or game, or tutorial, or settings
objects that change flow of game (eg main menu button) can tell this object to do something
*/

#include "sceneManager.h"

#include "sysWrappers/debugTools.h"

#include "engine/engineController.h"
#include "ui/mainMenu.cpp"



SceneManager::SceneManager(AudioPlayer* _audioPlayer) {
    audioPlayer = _audioPlayer;
    currentScene = new MainMenu();
    currentScene->load(audioPlayer);
}

void SceneManager::newScene(sceneStates scene) {
    currentScene->unload();

    if (scene == sceneStates::MAIN_MENU) {
        currentScene = new MainMenu();
    } else if (scene == sceneStates::GAME) {
        currentScene = new EngineController();
    } else {
        //error!
        Debugger::error("no definition for scene %d", scene);
        return;
    }

    currentScene->load(audioPlayer);
}

void SceneManager::update() {
    sceneStates nextState = currentScene->update(); //check return value of this to figure out if should load new scene or not
    if (nextState != sceneStates::CURRENT) { //if it has changed, we move on
        newScene(nextState);
    }
}
