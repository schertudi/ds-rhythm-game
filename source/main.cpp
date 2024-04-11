#include <nds.h>
#include <nds/arm9/console.h>
#include <stdio.h>
#include "vscode_fix.h"
#include <gl2d.h>
#include "sysWrappers/debugTools.h"
#include "engine/engineController.h"

#include "sceneManager.h"

//inputs: click beat on green to play it, press left trigger (Q key on melonDS) to toggle automated/manual playthrough


int main( int argc, char *argv[] )
{
	glScreen2D();

	//right screen uses 3d engine, is the main screen
	lcdMainOnBottom();
	videoSetMode( MODE_5_3D );

	//left screen uses 2d engine to print things, is the sub screen
	videoSetModeSub(MODE_5_2D);
	bgInitSub(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);

	//enable printing to left screen
	consoleDemoInit();

	Debugger::resetErrorMessage();
	//EngineController engineController;
	//engineController.init();

	AudioPlayer player;

	SceneManager manager = SceneManager(player);

	while(1) {
		Debugger::resetFrameLines();
		

		manager.update();
		Debugger::render();

		glFlush(0);

		swiWaitForVBlank();
	}

	return 0;
	
}



