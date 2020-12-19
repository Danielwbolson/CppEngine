
#ifndef GLOBALS_H_
#define GLOBALS_H_

#include "MemoryManager.h"
#include "AssetManager.h"
#include "MemoryAllocator.h"

class Camera;
class Scene;
class BVH;


extern Camera* mainCamera;
extern Scene* mainScene;
extern BVH* bvh;

extern int windowWidth;
extern int windowHeight;

#endif // GLOBALS_H_
