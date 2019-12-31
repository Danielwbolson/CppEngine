
#ifndef ASSET_MANAGER_H_
#define ASSET_MANAGER_H_

#include <memory>
#include <string>
#include <vector>

#include "tiny_obj_loader.h"

#include "ofbx.h"
#include "miniz.h"

#include "glad/glad.h"

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"

class Model;
class Material;
class Scene;
class Texture;
class Shader;

class AssetManager {
public:

	AssetManager();
	~AssetManager();

	static std::vector<Model*> models;
	static std::vector<Material*> materials;
	static std::vector<Texture*> textures;
	static std::vector<Shader*> shaders;

	static std::vector<GLuint> diffuseTextures;
	static std::vector<GLuint> specularTextures;
	static std::vector<GLuint> specularHighLightTextures;
	static std::vector<GLuint> bumpTextures;
	static std::vector<GLuint> normalTextures;
	static std::vector<GLuint> displacementTextures;
	static std::vector<GLuint> alphaTextures;

	static GLuint nullTexture;
	static GLubyte nullData[4];

	static Model* tinyLoadObj(const std::string fileName, bool useTinyMats = false);
	static Scene* LoadScene(const std::string fileName);
	static void LoadGameObjects(const std::string fileName, Scene* scene);
	static Material* LoadMaterial(const std::string& fileName);
	static Material* tinyLoadMaterial(const tinyobj::material_t& mat, const std::string& name);

	static void LoadTextureToGPU(const std::string texType, const int vecIndex, const int texIndex, Texture* tex);


};

#endif // ASSET_MANAGER_H_