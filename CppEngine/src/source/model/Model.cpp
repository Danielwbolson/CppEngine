
#include "Model.h"

#include "Mesh.h"
#include "Bounds.h"
#include "Material.h"


Model::Model() : meshes(std::vector<Mesh*>()), materials(std::vector<Material*>()), bounds(nullptr) {

}

Model::~Model() {
	for (int i = 0; i < meshes.size(); i++) {
		meshes[i] = nullptr;
	}
	meshes.clear();

	for (int i = 0; i < materials.size(); i++) {
		materials[i] = nullptr;
	}
	materials.clear();
}