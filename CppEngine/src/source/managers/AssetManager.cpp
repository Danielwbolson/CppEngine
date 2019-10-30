
#include "AssetManager.h"

#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <unordered_map>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#define TINYOBJLOADER_IMPLEMENTATION

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include "Component.h"
#include "ModelRenderer.h"
#include "Collider.h"
#include "BoxCollider.h"
#include "SphereCollider.h"
#include "Transform.h"

#include "Model.h"
#include "Mesh.h"
#include "Bounds.h"
#include "Material.h"
#include "Texture.h"

#include "Light.h"
#include "DirectionalLight.h"
#include "SpotLight.h"
#include "PointLight.h"
#include "AmbientLight.h"

#include "Configuration.h"

#include "Globals.h"
#include "Camera.h"
#include "Scene.h"
#include "GameObject.h"

#include "Utility.h"
#include "Shader.h"


std::vector<Model*> AssetManager::models = std::vector<Model*>();
std::vector<Material*> AssetManager::materials = std::vector<Material*>();
std::vector<Texture*> AssetManager::textures = std::vector<Texture*>();
std::vector<Shader*> AssetManager::shaders = std::vector<Shader*>();

std::vector<GLuint> AssetManager::ambientTextures = std::vector<GLuint>();
std::vector<GLuint> AssetManager::diffuseTextures = std::vector<GLuint>();
std::vector<GLuint> AssetManager::specularTextures = std::vector<GLuint>();
std::vector<GLuint> AssetManager::specularHighLightTextures = std::vector<GLuint>();
std::vector<GLuint> AssetManager::bumpTextures = std::vector<GLuint>();
std::vector<GLuint> AssetManager::displacementTextures = std::vector<GLuint>();
std::vector<GLuint> AssetManager::alphaTextures = std::vector<GLuint>();

GLuint AssetManager::nullTexture;
GLubyte AssetManager::nullData[4] = { 255, 255, 255, 255 };

ofbx::IScene* AssetManager::iscene = nullptr;

AssetManager::AssetManager() {

	// Set up our null texture
	glGenTextures(1, &nullTexture);
	glBindTexture(GL_TEXTURE_2D, nullTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullData);

}

AssetManager::~AssetManager() {
	for (int i = 0; i < models.size(); i++) {
		delete models[i];
	}
	models.clear();

	for (int i = 0; i < materials.size(); i++) {
		delete materials[i];
	}
	materials.clear();

	for (int i = 0; i < textures.size(); i++) {
		delete textures[i];
	}
	textures.clear();

	for (int i = 0; i < shaders.size(); i++) {
		glDeleteProgram(shaders[i]->shaderProgram);
	}
	shaders.clear();

	for (int i = 0; i < ambientTextures.size(); i++) {
		glDeleteTextures(1, &ambientTextures[i]);
	}
	ambientTextures.clear();

	for (int i = 0; i < diffuseTextures.size(); i++) {
		glDeleteTextures(1, &diffuseTextures[i]);
	}
	diffuseTextures.clear();

	for (int i = 0; i < specularTextures.size(); i++) {
		glDeleteTextures(1, &specularTextures[i]);
	}
	specularTextures.clear();

	for (int i = 0; i < specularHighLightTextures.size(); i++) {
		glDeleteTextures(1, &specularHighLightTextures[i]);
	}
	specularHighLightTextures.clear();

	for (int i = 0; i < bumpTextures.size(); i++) {
		glDeleteTextures(1, &bumpTextures[i]);
	}
	bumpTextures.clear();

	for (int i = 0; i < displacementTextures.size(); i++) {
		glDeleteTextures(1, &displacementTextures[i]);
	}
	displacementTextures.clear();

	for (int i = 0; i < alphaTextures.size(); i++) {
		glDeleteTextures(1, &alphaTextures[i]);
	}
	alphaTextures.clear();
}



void AssetManager::LoadFBX(const std::string fileName) {
	std::string fullFile = VK_ROOT_DIR"meshes/" + fileName;

	// open the file containing the scene description	
	FILE *fp = fopen(fullFile.c_str(), "rb");

	// check for errors in opening the file
	if (fp == NULL) {
		fprintf(stderr, "Can't open file '%s'\n", fullFile.c_str());
		exit(1);
	}

	fseek(fp, 0, SEEK_END); // find size of file
	long file_size = ftell(fp);
	fseek(fp, 0, SEEK_SET); // go back to front
	auto* content = new ofbx::u8[file_size];
	fread(content, 1, file_size, fp);
	iscene = ofbx::load((ofbx::u8*)content, file_size, (ofbx::u64)ofbx::LoadFlags::TRIANGULATE);

	if (!iscene) {
		OutputDebugString(ofbx::getError());
	}

	delete[] content;
	fclose(fp);
}



struct vertex {
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec2 uv;

	bool operator==(const vertex& rhs) const {
		return pos == rhs.pos && normal == rhs.normal && uv == rhs.uv;
	}
};

template<> struct std::hash<vertex> {
	size_t operator()(vertex const& v) const {
		return ((hash<glm::vec3>()(v.pos) ^
			(hash<glm::vec3>()(v.normal) << 1)) >> 1) ^
			(hash<glm::vec2>()(v.uv) << 1);
	}
};
Model* AssetManager::tinyLoadObj(const std::string fileName, bool useTinyMats) {

	std::string fullFile = VK_ROOT_DIR"meshes/" + fileName;

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> mats;

	std::string warn;
	std::string err;

	bool ret = tinyobj::LoadObj(&attrib, &shapes, &mats, &warn, &err, fullFile.c_str(), std::string((VK_ROOT_DIR"materials/")).c_str());

	Model* model = new Model();

	// For calculating bounds
	float minx = INFINITY;
	float miny = INFINITY;
	float minz = INFINITY;
	float maxx = -INFINITY;
	float maxy = -INFINITY;
	float maxz = -INFINITY;


	// Meshes are split by material type
	if (useTinyMats) {

		std::vector<std::unordered_map<vertex, unsigned int> > vertices = std::vector<std::unordered_map<vertex, unsigned int> >(mats.size());

		std::vector<Mesh*> tinyMeshes = std::vector<Mesh*>(mats.size());
		for (int i = 0; i < mats.size(); i++) {
			tinyMeshes[i] = new Mesh();
			tinyMeshes[i]->bounds = new Bounds();
			model->meshes.push_back(tinyMeshes[i]);
		}
		std::vector<Material*> tinyMaterials = std::vector<Material*>(mats.size());
		for (int i = 0; i < mats.size(); i++) {
			tinyMaterials[i] = tinyLoadMaterial(mats[i]);
			model->materials.push_back(tinyMaterials[i]);
		}

		for (int i = 0; i < shapes.size(); i++) {

			int indexOffset = 0;
			for (int j = 0; j < shapes[i].mesh.num_face_vertices.size(); j++) {

				int faceVertices = shapes[i].mesh.num_face_vertices[j];
				int matID = shapes[i].mesh.material_ids[j];

				for (int k = 0; k < faceVertices; k++) {
					tinyobj::index_t index = shapes[i].mesh.indices[indexOffset + k];

					vertex v = {};

					float x = attrib.vertices[3 * index.vertex_index + 0];
					float y = attrib.vertices[3 * index.vertex_index + 1];
					float z = attrib.vertices[3 * index.vertex_index + 2];
					v.pos = glm::vec3(x, y, z);

					// Get bounds
					if (v.pos.x > maxx) { maxx = v.pos.x; }
					if (v.pos.y > maxy) { maxy = v.pos.y; }
					if (v.pos.z > maxz) { maxz = v.pos.z; }
					if (v.pos.x < minx) { minx = v.pos.x; }
					if (v.pos.y < miny) { miny = v.pos.y; }
					if (v.pos.z < minz) { minz = v.pos.z; }

					float nx = attrib.normals[3 * index.normal_index + 0];
					float ny = attrib.normals[3 * index.normal_index + 1];
					float nz = attrib.normals[3 * index.normal_index + 2];
					v.normal = glm::vec3(nx, ny, nz);

					float uvx = attrib.texcoords[2 * index.texcoord_index + 0];
					float uvy = attrib.texcoords[2 * index.texcoord_index + 1];
					v.uv = glm::vec2(uvx, uvy);

					// Find if this vert already exists
					if (vertices[matID].count(v) == 0) {
						vertices[matID][v] = static_cast<unsigned int>(vertices[matID].size());
						tinyMeshes[matID]->positions.push_back(v.pos);
						tinyMeshes[matID]->normals.push_back(v.normal);
						tinyMeshes[matID]->uvs.push_back(v.uv);

						tinyMeshes[matID]->bounds->maxX = maxx;
						tinyMeshes[matID]->bounds->maxY = maxy;
						tinyMeshes[matID]->bounds->maxZ = maxz;
						tinyMeshes[matID]->bounds->minX = minx;
						tinyMeshes[matID]->bounds->minY = miny;
						tinyMeshes[matID]->bounds->minZ = minz;
					}

					tinyMeshes[matID]->indices.push_back(vertices[matID][v]);
				}

				indexOffset += faceVertices;
			}
		}

		for (int i = 0; i < tinyMeshes.size(); i++) {
			tinyMeshes[i]->bounds->Init();
			vertices[i].clear();
		}

	// One model with a material given by user
	} else {

		std::unordered_map<vertex, unsigned int> vertices = {};

		Mesh* mesh = new Mesh();

		for (const auto& shape : shapes) {
			for (const auto& index : shape.mesh.indices) {
				vertex v = {};

				float x = attrib.vertices[3 * index.vertex_index + 0];
				float y = attrib.vertices[3 * index.vertex_index + 1];
				float z = attrib.vertices[3 * index.vertex_index + 2];
				v.pos = glm::vec3(x, y, z);

				// Get bounds
				if (v.pos.x > maxx) { maxx = v.pos.x; }
				if (v.pos.y > maxy) { maxy = v.pos.y; }
				if (v.pos.z > maxz) { maxz = v.pos.z; }
				if (v.pos.x < minx) { minx = v.pos.x; }
				if (v.pos.y < miny) { miny = v.pos.y; }
				if (v.pos.z < minz) { minz = v.pos.z; }

				float nx = attrib.normals[3 * index.normal_index + 0];
				float ny = attrib.normals[3 * index.normal_index + 1];
				float nz = attrib.normals[3 * index.normal_index + 2];
				v.normal = glm::vec3(nx, ny, nz);

				float uvx = attrib.texcoords[2 * index.texcoord_index + 0];
				float uvy = attrib.texcoords[2 * index.texcoord_index + 1];
				v.uv = glm::vec2(uvx, uvy);

				// Find if this vert already exists
				if (vertices.count(v) == 0) {
					vertices[v] = static_cast<unsigned int>(vertices.size());
					mesh->positions.push_back(v.pos);
					mesh->normals.push_back(v.normal);
					mesh->uvs.push_back(v.uv);
				}

				mesh->indices.push_back(vertices[v]);

			}
		}

		mesh->bounds = new Bounds(minx, miny, minz, maxx, maxy, maxz);
		model->meshes.push_back(mesh);
		vertices.clear();
	}

	return model;
}

Material* AssetManager::tinyLoadMaterial(const tinyobj::material_t& mat) {
	Material* m = new Material(mat.name);

	m->ambient = glm::vec3(mat.ambient[0], mat.ambient[1], mat.ambient[2]);
	m->diffuse = glm::vec3(mat.diffuse[0], mat.diffuse[1], mat.diffuse[2]);
	m->specular = glm::vec3(mat.specular[0], mat.specular[1], mat.specular[2]);

	m->specularExponent = mat.shininess;
	m->opacity = mat.dissolve;
	m->illum = mat.illum;

	int w;
	int h;
	int numChannels;

	Texture* t;
	if (mat.ambient_texname != "") {
		GLubyte* pixels = stbi_load((VK_ROOT_DIR"textures/" + std::string(mat.ambient_texname)).c_str(), &w, &h, &numChannels, STBI_rgb_alpha);
		t = new Texture(w, h, numChannels, pixels);
		textures.push_back(t);
		ambientTextures.resize(ambientTextures.size() + 1);
		m->ambientTexture = t;
		m->ambientIndex = static_cast<int>(ambientTextures.size() - 1);
	}

	if (mat.diffuse_texname != "") {
		GLubyte* pixels = stbi_load((VK_ROOT_DIR"textures/" + std::string(mat.diffuse_texname)).c_str(), &w, &h, &numChannels, STBI_rgb_alpha);
		t = new Texture(w, h, numChannels, pixels);
		textures.push_back(t);
		diffuseTextures.resize(diffuseTextures.size() + 1);
		m->diffuseTexture = t;
		m->diffuseIndex = static_cast<int>(diffuseTextures.size() - 1);
	}

	if (mat.specular_texname != "") {
		GLubyte* pixels = stbi_load((VK_ROOT_DIR"textures/" + std::string(mat.specular_texname)).c_str(), &w, &h, &numChannels, STBI_rgb_alpha);
		t = new Texture(w, h, numChannels, pixels);
		textures.push_back(t);
		specularTextures.resize(specularTextures.size() + 1);
		m->specularTexture = t;
		m->specularIndex = static_cast<int>(specularTextures.size() - 1);
	}

	if (mat.specular_highlight_texname != "") {
		GLubyte* pixels = stbi_load((VK_ROOT_DIR"textures/" + std::string(mat.specular_highlight_texname)).c_str(), &w, &h, &numChannels, STBI_rgb_alpha);
		t = new Texture(w, h, numChannels, pixels);
		textures.push_back(t);
		specularHighLightTextures.resize(specularHighLightTextures.size() + 1);
		m->specularHighLightTexture = t;
		m->specularHighLightIndex = static_cast<int>(specularHighLightTextures.size() - 1);
	}

	if (mat.bump_texname != "") {
		GLubyte* pixels = stbi_load((VK_ROOT_DIR"textures/" + std::string(mat.bump_texname)).c_str(), &w, &h, &numChannels, STBI_rgb_alpha);
		t = new Texture(w, h, numChannels, pixels);
		textures.push_back(t);
		bumpTextures.resize(bumpTextures.size() + 1);
		m->bumpTexture = t;
		m->bumpIndex = static_cast<int>(bumpTextures.size() - 1);
	}

	if (mat.displacement_texname != "") {
		GLubyte* pixels = stbi_load((VK_ROOT_DIR"textures/" + std::string(mat.displacement_texname)).c_str(), &w, &h, &numChannels, STBI_rgb_alpha);
		t = new Texture(w, h, numChannels, pixels);
		textures.push_back(t);
		displacementTextures.resize(displacementTextures.size() + 1);
		m->displacementTexture = t;
		m->displacementIndex = static_cast<int>(displacementTextures.size() - 1);
	}

	if (mat.alpha_texname != "") {
		GLubyte* pixels = stbi_load((VK_ROOT_DIR"textures/" + std::string(mat.alpha_texname)).c_str(), &w, &h, &numChannels, STBI_rgb_alpha);
		t = new Texture(w, h, numChannels, pixels);
		textures.push_back(t);
		alphaTextures.resize(alphaTextures.size() + 1);
		m->alphaTexture = t;
		m->alphaIndex = static_cast<int>(alphaTextures.size() - 1);
	}

	t = nullptr;
	return m;
}

Material* AssetManager::LoadMaterial(const std::string& fileName) {

	//TODO: Only works for 1 material per .mtl file


	// Check for existing material
	for (int i = 0; i < materials.size(); i++) {
		if (materials[i]->filename == fileName) {
			return materials[i];
		}
	}

	Material* m = new Material(fileName);

	FILE *fp;
	char line[1024]; //Assumes no line is longer than 1024 characters!

	std::string fullFile = VK_ROOT_DIR"materials/" + fileName;

	// open the file containing the scene description
	fp = fopen(fullFile.c_str(), "r");

	// check for errors in opening the file
	if (fp == NULL) {
		fprintf(stderr, "Can't open file '%s'\n", fullFile.c_str());
		exit(1);
	}

	stbi_set_flip_vertically_on_load(true);

	while (fgets(line, 1024, fp)) { //Assumes no line is longer than 1024 characters!
		if (line[0] == '#') {
			//fprintf(stderr, "Skipping comment: %s", line);
			continue;
		}

		char command[1024];
		int fieldsRead = sscanf(line, "%s ", command); //Read first word in the line (i.e., the command type)

		if (fieldsRead < 1) { //No command read
			//Blank line
			continue;
		}

		if (strcmp(command, "Ka") == 0) {
			float r, g, b;

			sscanf(line, "Ka %f %f %f",
				&r, &g, &b);

			m->ambient = glm::vec3(r, g, b);
		} else if (strcmp(command, "Kd") == 0) {
			float r, g, b;

			sscanf(line, "Kd %f %f %f",
				&r, &g, &b);

			m->diffuse = glm::vec3(r, g, b);
		} else if (strcmp(command, "Ks") == 0) {
			float r, g, b;

			sscanf(line, "Ks %f %f %f",
				&r, &g, &b);

			m->specular = glm::vec3(r, g, b);
		} else if (strcmp(command, "Ns") == 0) {
			float spec;

			sscanf(line, "Ns %f",
				&spec);

			m->specularExponent = spec;
		} else if (strcmp(command, "Tr") == 0) {
			float tr;

			sscanf(line, "Tr %f",
				&tr);

			m->opacity = 1.0f - tr;
		} else if (strcmp(command, "d") == 0) {
			float d;

			sscanf(line, "d %f",
				&d);

			m->opacity = d;
		} else if (strcmp(command, "illum") == 0) {
			int illum;

			sscanf(line, "illum %d",
				&illum);

			m->illum = illum;
		} else if (strcmp(command, "map_Ka") == 0) {
			char filename[1024];

			sscanf(line, "map_Ka %s",
				&filename);

			int w;
			int h;
			int numChannels;
			GLubyte* pixels = stbi_load((VK_ROOT_DIR"textures/" + std::string(filename)).c_str(), &w, &h, &numChannels, STBI_rgb_alpha);

			Texture* t = new Texture(w, h, numChannels, pixels);

			textures.push_back(t);
			ambientTextures.resize(ambientTextures.size() + 1);

			m->ambientTexture = t;
			m->ambientIndex = static_cast<int>(ambientTextures.size() - 1);
		} else if (strcmp(command, "map_Kd") == 0) {
			char filename[1024];

			sscanf(line, "map_Kd %s",
				&filename);

			int w;
			int h;
			int numChannels;
			GLubyte* pixels = stbi_load((VK_ROOT_DIR"textures/" + std::string(filename)).c_str(), &w, &h, &numChannels, STBI_rgb_alpha);

			Texture* t = new Texture(w, h, numChannels, pixels);

			textures.push_back(t);
			diffuseTextures.resize(diffuseTextures.size() + 1);

			m->diffuseTexture = t;
			m->diffuseIndex = static_cast<int>(diffuseTextures.size() - 1);
		} else if (strcmp(command, "map_Ks") == 0) {
			char filename[1024];

			sscanf(line, "map_Ks %s",
				&filename);

			int w;
			int h;
			int numChannels;
			GLubyte* pixels = stbi_load((VK_ROOT_DIR"textures/" + std::string(filename)).c_str(), &w, &h, &numChannels, STBI_rgb_alpha);

			Texture* t = new Texture(w, h, numChannels, pixels);

			textures.push_back(t);
			specularTextures.resize(specularTextures.size() + 1);

			m->specularTexture = t;
			m->specularIndex = static_cast<int>(specularTextures.size() - 1);
		} else if (strcmp(command, "map_Ns") == 0) {
			char filename[1024];

			sscanf(line, "map_Ns %s",
				&filename);

			int w;
			int h;
			int numChannels;
			GLubyte* pixels = stbi_load((VK_ROOT_DIR"textures/" + std::string(filename)).c_str(), &w, &h, &numChannels, STBI_rgb_alpha);

			Texture* t = new Texture(w, h, numChannels, pixels);

			textures.push_back(t);
			specularHighLightTextures.resize(specularHighLightTextures.size() + 1);

			m->specularHighLightTexture = t;
			m->specularHighLightIndex = static_cast<int>(specularHighLightTextures.size() - 1);
		} else if (strcmp(command, "map_d") == 0) {
			char filename[1024];

			sscanf(line, "map_d %s",
				&filename);

			int w;
			int h;
			int numChannels;
			GLubyte* pixels = stbi_load((VK_ROOT_DIR"textures/" + std::string(filename)).c_str(), &w, &h, &numChannels, STBI_rgb_alpha);

			Texture* t = new Texture(w, h, numChannels, pixels);

			textures.push_back(t);
			alphaTextures.resize(alphaTextures.size() + 1);

			m->alphaTexture = t;
			m->alphaIndex = static_cast<int>(alphaTextures.size() - 1);
		} else if (strcmp(command, "map_bump") == 0) {
			char filename[1024];

			sscanf(line, "map_bump %s",
				&filename);

			int w;
			int h;
			int numChannels;
			GLubyte* pixels = stbi_load((VK_ROOT_DIR"textures/" + std::string(filename)).c_str(), &w, &h, &numChannels, STBI_rgb_alpha);

			Texture* t = new Texture(w, h, numChannels, pixels);

			textures.push_back(t);
			bumpTextures.resize(bumpTextures.size() + 1);

			m->bumpTexture = t;
			m->bumpIndex = static_cast<int>(bumpTextures.size() - 1);
		} else if (strcmp(command, "bump") == 0) {
			char filename[1024];

			sscanf(line, "bump %s",
				&filename);

			int w;
			int h;
			int numChannels;
			GLubyte* pixels = stbi_load((VK_ROOT_DIR"textures/" + std::string(filename)).c_str(), &w, &h, &numChannels, STBI_rgb_alpha);

			Texture* t = new Texture(w, h, numChannels, pixels);

			textures.push_back(t);
			bumpTextures.resize(bumpTextures.size() + 1);

			m->bumpTexture = t;
			m->bumpIndex = static_cast<int>(bumpTextures.size() - 1);
		} else if (strcmp(command, "disp") == 0) {
			char filename[1024];

			sscanf(line, "disp %s",
				&filename);

			int w;
			int h;
			int numChannels;
			GLubyte* pixels = stbi_load((VK_ROOT_DIR"textures/" + std::string(filename)).c_str(), &w, &h, &numChannels, STBI_rgb_alpha);

			Texture* t = new Texture(w, h, numChannels, pixels);

			textures.push_back(t);
			displacementTextures.resize(displacementTextures.size() + 1);

			m->displacementTexture = t;
			m->displacementIndex = static_cast<int>(displacementTextures.size() - 1);
		} else {
			continue;
		}
	}

	materials.push_back(m);

	return m;
}

Scene* AssetManager::LoadScene(const std::string fileName) {
	FILE *fp;
	char line[1024]; //Assumes no line is longer than 1024 characters!

	std::string fullFile = VK_ROOT_DIR + fileName;

	// open the file containing the scene description
	fp = fopen(fullFile.c_str(), "r");

	// check for errors in opening the file
	if (fp == NULL) {
		fprintf(stderr, "Can't open file '%s'\n", fullFile.c_str());
		exit(1);
	}

	Scene* scene = new Scene();

	//Loop through reading each line
	while (fgets(line, 1024, fp)) { //Assumes no line is longer than 1024 characters!
		if (line[0] == '#') {
			//fprintf(stderr, "Skipping comment: %s", line);
			continue;
		}

		char command[1024];
		int fieldsRead = sscanf(line, "%s ", command); //Read first word in the line (i.e., the command type)

		if (fieldsRead < 1) { //No command read
			//Blank line
			continue;
		}

		if (strcmp(command, "camera") == 0) {
			float px, py, pz;
			float fx, fy, fz;
			float ux, uy, uz;
			float fov, np, fp;

			sscanf(line, "camera %f %f %f %f %f %f %f %f %f %f %f %f",
				&px, &py, &pz, &fx, &fy, &fz, &ux, &uy, &uz, &fov, &np, &fp);

			mainCamera = new Camera(
				glm::vec3(px, py, pz),
				glm::vec3(fx, fy, fz),
				glm::vec3(ux, uy, uz),
				fov, np, fp);
		} else if (strcmp(command, "directional_light") == 0) { // If the command is a directional light
			float r, g, b, a, dx, dy, dz, dw;
			sscanf(line, "directional_light %f %f %f %f %f %f %f %f",
				&r, &g, &b, &a, &dx, &dy, &dz, &dw);

			DirectionalLight* d = new DirectionalLight{
				glm::vec4(r, g, b, a),
				glm::normalize(glm::vec4(dx, dy, dz, dw))
			};

			scene->lights.push_back(d);
		} else if (strcmp(command, "spot_light") == 0) { // If the command is a point_light
			float r, g, b, a;
			float px, py, pz, pw;
			float dx, dy, dz, dw;
			float angle1, angle2;
			sscanf(line, "spot_light %f %f %f %f %f %f %f %f %f %f %f %f %f %f",
				&r, &g, &b, &a, &px, &py, &pz, &pw, &dx, &dy, &dz, &dw, &angle1, &angle2);

			SpotLight* s = new SpotLight{
				glm::vec4(r, g, b, a),
				glm::vec4(px, py, pz, pw),
				glm::normalize(glm::vec4(dx, dy, dz, dw)),
				angle1,
				angle2
			};

			scene->lights.push_back(s);
		} else if (strcmp(command, "point_light") == 0) { // If the command is a spot_light
			float r, g, b, a, x, y, z, w;
			sscanf(line, "point_light %f %f %f %f %f %f %f %f",
				&r, &g, &b, &a, &x, &y, &z, &w);

			PointLight* p = new PointLight{
				glm::vec4(r, g, b, a),
				glm::vec4(x, y, z, w)
			};

			scene->lights.push_back(p);
		} else if (strcmp(command, "ambient_light") == 0) { // If the command is a ambient_light
			float r, g, b, a;
			sscanf(line, "ambient_light %f %f %f %f", &r, &g, &b, &a);

			AmbientLight* amb = new AmbientLight{
				glm::vec4(r, g, b, a)
			};

			scene->lights.push_back(amb);
		} else {
			fprintf(stderr, "WARNING. Do not know command: %s\n", command);
		}
	}

	return scene;
}

void AssetManager::LoadGameObjects(const std::string fileName, Scene* scene) {
	FILE *fp;
	char line[1024]; //Assumes no line is longer than 1024 characters!

	std::string fullFile = VK_ROOT_DIR + fileName;

	// open the file containing the scene description
	fp = fopen(fullFile.c_str(), "r");

	// check for errors in opening the file
	if (fp == NULL) {
		fprintf(stderr, "Can't open file '%s'\n", fullFile.c_str());
		exit(1);
	}

	GameObject* currGameObject = nullptr;
	Model* currModel = nullptr;
	Material* currMaterial = nullptr;
	Shader* currShader = nullptr;

	//Loop through reading each line
	while (fgets(line, 1024, fp)) { //Assumes no line is longer than 1024 characters!
		if (line[0] == '#') {
			//fprintf(stderr, "Skipping comment: %s", line);
			continue;
		}

		char command[1024];
		int fieldsRead = sscanf(line, "%s ", command); //Read first word in the line (i.e., the command type)

		if (fieldsRead < 1) { //No command read
			//Blank line
			continue;
		}

		if (strcmp(command, "gameObject") == 0) {
			if (currGameObject != nullptr) {
				scene->gameObjects.push_back(currGameObject);
			}

			currGameObject = new GameObject();
			char name[1024];

			sscanf(line, "gameObject %s", name);
			currGameObject->name = name;

			currModel = nullptr;
			currMaterial = nullptr;

		} else if (strcmp(command, "component") == 0) {
			char type[1024];

			sscanf(line, "component %s", &type);

			if (strcmp(type, "boxCollider") == 0) {
				glm::vec3 pos;
				float width, height;
				int dynamic;
				sscanf(line, "component boxCollider %f %f %f %f %f %d",
					&pos.x, &pos.y, &pos.z, &width, &height, &dynamic);
				if (currGameObject)
					currGameObject->AddComponent(new BoxCollider(pos, width, height, dynamic));
			} else if (strcmp(type, "sphereCollider") == 0) {
				glm::vec3 pos;
				float radius;
				int dynamic;

				sscanf(line, "component sphereCollider %f %f %f %f %d",
					&pos.x, &pos.y, &pos.z, &radius, &dynamic);

				if (currGameObject)
					currGameObject->AddComponent(new SphereCollider(pos, radius, dynamic));
			} else if (strcmp(type, "modelRenderer") == 0) {
				if (currGameObject)
					currGameObject->AddComponent(new ModelRenderer(currModel));
			} else { continue; }
		} else if (strcmp(command, "model") == 0) {
			char filename[1024];

			sscanf(line, "model %s", &filename);

			currModel = tinyLoadObj(filename, currMaterial == nullptr);
			currModel->name = currGameObject->name;

			// TODO: Hack for now, but eventually need to support materials in obj
			if (currMaterial != nullptr) {
				currModel->materials.push_back(currMaterial);
			}

			for (int i = 0; i < currModel->materials.size(); i++) {
				currModel->materials[i]->shader = shaders[shaders.size() - 1];
			}

		} else if (strcmp(command, "fbx") == 0) {
			char filename[1024];

			sscanf(line, "fbx %s", &filename);

			LoadFBX(filename);
		} else if (strcmp(command, "material") == 0) { // If the command is a material
			char filename[1024];

			sscanf(line, "material  %s",
				&filename);

			currMaterial = LoadMaterial(filename);
		} else if (strcmp(command, "shader") == 0) {
			char vert[1024];
			char frag[1024];

			sscanf(line, "shader %s %s",
				&vert, &frag);

			shaders.push_back(new Shader(vert, frag));
		} else {
			fprintf(stderr, "WARNING. Do not know command: %s\n", command);
		}
	}

	// Push in our last gameobject
	if (currGameObject != nullptr) {
		scene->gameObjects.push_back(currGameObject);
	}

	currMaterial = nullptr;
	currModel = nullptr;
	currGameObject = nullptr;
	currShader = nullptr;

}

void AssetManager::LoadTextureToGPU(const std::string texType, const int vecIndex, const int texIndex, Texture* tex) {

	if (texType == "ambient") {
		glGenTextures(1, &ambientTextures[vecIndex]);
		glActiveTexture(GL_TEXTURE0 + texIndex);
		glBindTexture(GL_TEXTURE_2D, ambientTextures[vecIndex]);
	} else if (texType == "diffuse") {
		glGenTextures(1, &diffuseTextures[vecIndex]);
		glActiveTexture(GL_TEXTURE0 + texIndex);
		glBindTexture(GL_TEXTURE_2D, diffuseTextures[vecIndex]);
	} else if (texType == "specular") {
		glGenTextures(1, &specularTextures[vecIndex]);
		glActiveTexture(GL_TEXTURE0 + texIndex);
		glBindTexture(GL_TEXTURE_2D, specularTextures[vecIndex]);
	} else if (texType == "specularHighlight") {
		glGenTextures(1, &specularHighLightTextures[vecIndex]);
		glActiveTexture(GL_TEXTURE0 + texIndex);
		glBindTexture(GL_TEXTURE_2D, specularHighLightTextures[vecIndex]);
	} else if (texType == "bump") {
		glGenTextures(1, &bumpTextures[vecIndex]);
		glActiveTexture(GL_TEXTURE0 + texIndex);
		glBindTexture(GL_TEXTURE_2D, bumpTextures[vecIndex]);
	} else if (texType == "displacement") {
		glGenTextures(1, &displacementTextures[vecIndex]);
		glActiveTexture(GL_TEXTURE0 + texIndex);
		glBindTexture(GL_TEXTURE_2D, displacementTextures[vecIndex]);
	} else if (texType == "alpha") {
		glGenTextures(1, &alphaTextures[vecIndex]);
		glActiveTexture(GL_TEXTURE0 + texIndex);
		glBindTexture(GL_TEXTURE_2D, alphaTextures[vecIndex]);
	}

	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex->width, tex->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex->pixels);

	tex->loadedToGPU = true;

	stbi_image_free(tex->pixels);
}

