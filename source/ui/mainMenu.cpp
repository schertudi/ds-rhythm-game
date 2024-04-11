#include "../genericTypes.h"
#include "../sysWrappers/vectorShapes.h"
#include <vector>
#include "../sysWrappers/debugTools.h"
#include "../sysWrappers/input.h"
#include "../sysWrappers/debugTools.h"
#include "../sceneObject.h"





/*
button has a position from (x,y) to (x,y)
it draws itself (in a generic way) to these coords, on the bottom screen
it can detect if it has been pressed given mouse pos
if so, it notifies its master (in this case MainMenu) that it has been clicked, and gives it the command (probably enum value)
then main menu can handle loading rather than that duty being all of buttons
*/

enum class buttonCommand { LOAD_TUTORIAL, LOAD_GAME, LOAD_SETTINGS };

class Button {
    public:
    Button(Vec2d _start, Vec2d _end, buttonCommand _command) {
        command = _command;
        startPos = _start;
        endPos = _end;
        penDown = false;
        activated = false;
    }

    void update(Vec2d penPos) {
        

        activated = false; //reset flag so it only reports true on the frame pen is lifted
        if (!penDown && isPressed(penPos)) {
            penDown = true;
        }
        if (penDown && !isPressed(penPos)) {
            activated = true;
            penDown = false;
        }

        Colour c = {0, 10, 20};
        if (activated) {
            c = {10, 15, 30};
        } else if (penDown) {
            c = {0, 5, 10};
        }

        vectorRect(startPos.x, startPos.y, endPos.x, endPos.y, c, 0);
    }

    

    bool isActivated() {
        return activated;
    }
    //we actually dont want press on pen down but on pen up - so need to internally log a press after draw(), then when no longer pressed we tell parent

    buttonCommand getCommand() {
        return command;
    }

    private:
    buttonCommand command;
    Vec2d startPos;
    Vec2d endPos;
    bool penDown;
    bool activated;

    bool isPressed(Vec2d penPos) {
        if (penPos.x >= startPos.x && penPos.x <= endPos.x) {
            return penPos.y >= startPos.y && penPos.y <= endPos.y;
        }
        return false;
    } 
};



class MainMenu : public SceneObject {

    

    public:
    void load(AudioPlayer _audioPlayer) override {
        

        int height = 192;
        int width = 256;

        int yPadding = 20;
        int xPadding = 50;
        int btnHeight = 30;

        int y2Start = height / 2 - btnHeight / 2;
        int y2End = height / 2 + btnHeight / 2; 

        int y1End = y2Start - yPadding;
        int y1Start = y1End - btnHeight;

        int y3Start = y2End + yPadding;
        int y3End = y3Start + btnHeight;

        int xStart = xPadding;
        int xEnd = width - xPadding;

        Button tutBtn = Button( {xStart, y1Start}, {xEnd, y1End}, buttonCommand::LOAD_TUTORIAL);
        Button playBtn = Button( {xStart, y2Start}, {xEnd, y2End}, buttonCommand::LOAD_GAME);
        Button settingsBtn = Button( {xStart, y3Start}, {xEnd, y3End}, buttonCommand::LOAD_SETTINGS);

        buttons.push_back(tutBtn);
        buttons.push_back(playBtn);
        buttons.push_back(settingsBtn);
    }

    void unload() override {
    
    }

    sceneStates update() override {
        Vec2d penPos = getPenPos();

        for (size_t i=0; i < buttons.size(); i++) {
            buttons[i].update(penPos);
            if (buttons[i].isActivated()) {
                buttonCommand cmd = buttons[i].getCommand();
                Debugger::print("btn press %d", cmd);
                sceneStates newScene = getSceneFromCommand(cmd);
                return newScene;
            }
        }

        Debugger::clearConsole();
        Debugger::framePrint("buttons");

        return sceneStates::CURRENT;
    }

    private:
    std::vector<Button> buttons;

    sceneStates getSceneFromCommand(buttonCommand cmd) {
        if (cmd == buttonCommand::LOAD_TUTORIAL) {
            return sceneStates::TUTORIAL;
        }
        if (cmd == buttonCommand::LOAD_GAME) {
            return sceneStates::GAME;
        }
        if (cmd == buttonCommand::LOAD_SETTINGS) {
            return sceneStates::SETTINGS;
        }

        Debugger::error("cannot switch scene from command %d", cmd);
        return sceneStates::CURRENT;
    }
 
    




};