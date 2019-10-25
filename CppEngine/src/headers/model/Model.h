
#ifndef MODEL_H_
#define MODEL_H_

#include <vector>
#include <string>

class Mesh;
class Material;
class Bounds;

class Model {

public:
	std::string name;
	std::vector<Mesh*> meshes;
	std::vector<Material*> materials;
	Bounds* bounds;

	Model();
	~Model();

};

#endif