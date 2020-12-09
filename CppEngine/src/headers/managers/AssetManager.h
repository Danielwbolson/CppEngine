
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

#include "MemoryAllocator.h"
#include "RenderTypes.h"

class BVH;
class Model;
class Material;
class Scene;
class Texture;
class Shader;

namespace AssetManager {

	void Init();
	void CleanUp();

	extern std::vector<Model*, MemoryAllocator<Model*> >* models;
	extern std::vector<Material*, MemoryAllocator<Material*> >* materials;
	extern std::vector<Texture*, MemoryAllocator<Texture*> >* textures;
	extern std::vector<Shader*, MemoryAllocator<Shader*> >* shaders;

	extern std::vector<GLuint, MemoryAllocator<GLuint> >* diffuseTextures;
	extern std::vector<GLuint, MemoryAllocator<GLuint> >* specularTextures;
	extern std::vector<GLuint, MemoryAllocator<GLuint> >* specularHighLightTextures;
	extern std::vector<GLuint, MemoryAllocator<GLuint> >* bumpTextures;
	extern std::vector<GLuint, MemoryAllocator<GLuint> >* normalTextures;
	extern std::vector<GLuint, MemoryAllocator<GLuint> >* displacementTextures;
	extern std::vector<GLuint, MemoryAllocator<GLuint> >* alphaTextures;

	// GPU data
	extern std::vector<GPUVertex>* gpuVertices;
	extern std::vector<GPUTriangle>* gpuTriangles;
	extern std::vector<GPUMaterial>* gpuMaterials;


	extern GLuint nullTexture;
	extern GLubyte nullData[4];

	Model* tinyLoadObj(const std::string fileName, bool useTinyMats = false);
	Scene* LoadScene(const std::string fileName);
	void LoadGameObjects(const std::string fileName, Scene* scene);
	Material* LoadMaterial(const std::string& fileName);
	Material* tinyLoadMaterial(const tinyobj::material_t& mat, const std::string& name);

	void PostLoadScene();

	void LoadTextureToGPU(const std::string texType, const int vecIndex, const int texIndex, Texture* tex);
	uint64_t LoadBindlessTexture(std::string texType, Texture* tex, int32_t index);

	void AllocateGPUMemory();

};

#endif // ASSET_MANAGER_H_