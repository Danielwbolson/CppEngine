
#ifndef ASSET_MANAGER_H_
#define ASSET_MANAGER_H_

#include <memory>
#include <string>
#include <vector>

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"

class Mesh;
class Material;
class Scene;
class Map;

class AssetManager {
public:

	AssetManager();
	~AssetManager();

	static std::vector<Mesh*> meshes;
	static std::vector<Material*> materials;
	//std::vector<Texture*> textures;

	static Mesh* LoadObj(const std::string fileName);
	static Scene* LoadScene(const std::string fileName);
	static Map* LoadMap(const std::string fileName, Scene* scene);
	static Material* LoadMaterial(const glm::vec3&, const glm::vec3&, const glm::vec3&, const glm::vec3&,
		const std::string&, const std::string&);
};

#endif // ASSET_MANAGER_H_