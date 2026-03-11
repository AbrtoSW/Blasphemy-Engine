#include <iostream>
#include "engine/BlasphemyEngine.h"

int main() {
	
	BlasphemyEngine engine;
	engine.init();
	engine.run();
	engine.shutdown();

	return 0;
}
