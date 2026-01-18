#include "FileGame.h"
#include "KeyboardGame.h"
#include "GameBase.h"

int main(int argc, char* argv[]) {
	bool saveMode = false;
	bool loadMode = false;
	bool silentMode = false;

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-save") == 0) saveMode = true;
		if (strcmp(argv[i], "-load") == 0) loadMode = true;
		if (strcmp(argv[i], "-silent") == 0) silentMode = true;
	}

	if (loadMode) {
		FileGame game(silentMode);
		if (!game.loadFileGameResources()) {
			return 0;  // file upload failed
		}
		game.run();
	}
	else {
		KeyboardGame game(saveMode);
		game.showMenu();
	}

	return 0;
}
