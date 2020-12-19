
#ifndef MODEL_H_
#define MODEL_H_

#include <vector>
#include <string>
#include "MemoryAllocator.h"

class Mesh;
class Material;
class Bounds;
class ModelRenderer;

class Model {

public:
	std::string name;
	std::vector<Mesh*, MemoryAllocator<Mesh*> > meshes;
	std::vector<Material*, MemoryAllocator<Material*> > materials;
	Bounds* bounds;
	ModelRenderer* modelRenderer;

	Model();
	~Model();

};

#endif