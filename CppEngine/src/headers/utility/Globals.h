
#ifndef GLOBALS_H_
#define GLOBALS_H_

#include "MemoryManager.h"

class Camera;
class Scene;
class AssetManager;


extern Camera* mainCamera;
extern Scene* mainScene;
extern AssetManager* assetManager;
extern MemoryManager* memoryManager;

extern int windowWidth;
extern int windowHeight;

#endif // GLOBALS_H_
