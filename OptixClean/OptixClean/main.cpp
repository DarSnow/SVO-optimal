#include "Scene\scene.h"
#  if defined(_WIN32)
#    include <GL/wglew.h>
#  endif
#include "optixPart\optixSampler.h"

//extern void cudamain(int argc, char *argv[]);

int main(int argc, char *argv[]) {
	//cudamain(argc, argv);
	Scene* scene = Scene::getInstance();
	//OptixSampler *op = new OptixSampler();
	scene->init();
	// have to initialize after sceneinit
	//op->Initialize(&scene);
	scene->run();
}
