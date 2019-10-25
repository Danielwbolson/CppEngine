
#include "AssetManager.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <unordered_map>

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


std::vector<Model*> AssetManager::models = std::vector<Model*>();
std::vector<Material*> AssetManager::materials = std::vector<Material*>();

AssetManager::AssetManager() {}

AssetManager::~AssetManager() {
	for (int i = 0; i < models.size(); i++) {
		delete models[i];
	}
	models.clear();

	for (int i = 0; i < materials.size(); i++) {
		delete materials[i];
	}
	materials.clear();
}

struct vertex {
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec2 uv;

	bool operator==(const vertex& rhs) {
		return pos == rhs.pos && normal == rhs.normal && uv == rhs.uv;
	}
};

Model* AssetManager::tinyLoadObj(const std::string fileName) {

	std::string fullFile = VK_ROOT_DIR"meshes/" + fileName;

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> mats;

	std::string warn;
	std::string err;

	bool ret = tinyobj::LoadObj(&attrib, &shapes, &mats, &warn, &err, fullFile.c_str());

	Model* model = new Model();
	for (const auto& shape : shapes) {

		// For calculating bounds
		float minx = INFINITY;
		float miny = INFINITY;
		float minz = INFINITY;
		float maxx = -INFINITY;
		float maxy = -INFINITY;
		float maxz = -INFINITY;

		// Per shape info		
		std::vector<glm::vec3> positions;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec2> uvs;
		std::vector<unsigned int> indices;

		std::vector<vertex> vertices;
		std::vector<vertex> rawVertices;

		Mesh* mesh = new Mesh();
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
			float uvy = 1.0f - attrib.texcoords[2 * index.texcoord_index + 1];
			v.uv = glm::vec2(uvx, uvy);

			// Find if this vert already exists
			bool exists = false;
			for (int i = static_cast<int>(vertices.size()) - 1; i >= 0; i--) {
				if (v == vertices[i]) {
					exists = true;
					indices.push_back(i);
					break;
				}
			}
			if (!exists) {
				indices.push_back(static_cast<unsigned int>(vertices.size()));
				vertices.push_back(v);
				positions.push_back(v.pos);
				normals.push_back(v.normal);
				uvs.push_back(v.uv);
			}
		}

		mesh->SetIndices(indices);
		mesh->SetPositions(positions);
		mesh->SetNormals(normals);
		mesh->SetUvs(uvs);
		mesh->bounds = new Bounds(minx, miny, minz, maxx, maxy, maxz);
		model->meshes.push_back(mesh);

		vertices.clear();
		rawVertices.clear();

		positions.clear();
		normals.clear();
		uvs.clear();
		indices.clear();
	}

	return model;
}

Model* AssetManager::LoadObj(const std::string fileName) {
	FILE *fp;
	char line[1024]; //Assumes no line is longer than 1024 characters!

	std::string fullFile = VK_ROOT_DIR"meshes/" + fileName;

	// open the file containing the scene description
	fp = fopen(fullFile.c_str(), "r");

	// check for errors in opening the file
	if (fp == NULL) {
		fprintf(stderr, "Can't open file '%s'\n", fullFile.c_str());
		exit(1);
	}

	// For calculating bounds
	float minx = INFINITY;
	float miny = INFINITY;
	float minz = INFINITY;
	float maxx = -INFINITY;
	float maxy = -INFINITY;
	float maxz = -INFINITY;

	std::vector<glm::vec3> rawVerts;
	std::vector<glm::vec3> rawNormals;
	std::vector<glm::vec2> rawUvs;

	std::vector<glm::vec3> verts;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> uvs;
	std::vector<unsigned int> indices;


	struct vertData {
		int v;
		int uv;
		int n;

		bool operator==(const vertData& rhs) {
			if (v != rhs.v) { return false; }
			if (uv != rhs.uv) { return false; }
			if (n != rhs.n) { return false; }

			return true;
		}
	};
	std::vector<vertData> vertMap;


	Model* model = nullptr;
	Mesh* mesh = nullptr;
	int nextIndex = 0;
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

		if (line[0] == '#') {
			//fprintf(stderr, "Skipping comment: %s", line);
			continue;
		}

		// vertex
		if (strcmp(command, "v") == 0) {
			glm::vec3 v;

			sscanf(line, "v %f %f %f", &v.x, &v.y, &v.z);
			rawVerts.push_back(v);

			if (v.x > maxx) { maxx = v.x; }
			if (v.y > maxy) { maxy = v.y; }
			if (v.z > maxz) { maxz = v.z; }
			if (v.x < minx) { minx = v.x; }
			if (v.y < miny) { miny = v.y; }
			if (v.z < minz) { minz = v.z; }
		}
		// uvs
		else if (strcmp(command, "vt") == 0) {
			glm::vec2 uv;

			sscanf(line, "vt %f %f", &uv.x, &uv.y);
			rawUvs.push_back(uv);
		}
		// normals
		else if (strcmp(command, "vn") == 0) {
			glm::vec3 n;

			sscanf(line, "vn %f %f %f", &n.x, &n.y, &n.z);
			rawNormals.push_back(glm::normalize(n));
		}
		// face
		else if (strcmp(command, "f") == 0) {
			int vert_info[3][3];

			sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d",
				&vert_info[0][0], &vert_info[0][1], &vert_info[0][2],
				&vert_info[1][0], &vert_info[1][1], &vert_info[1][2],
				&vert_info[2][0], &vert_info[2][1], &vert_info[2][2]);
			// https://stackoverflow.com/questions/23349080/opengl-index-buffers-difficulties/23356738#23356738

			for (int i = 0; i < 3; i++) {
				vertData temp_vert = vertData{
					temp_vert.v = vert_info[i][0],
					temp_vert.uv = vert_info[i][1],
					temp_vert.n = vert_info[i][2]
				};

				bool exists = false;
				for (int j = 0; j < vertMap.size(); j++) {
					if (temp_vert == vertMap[j]) {
						exists = true;
						indices.push_back(j);
						break;
					}
				}

				if (!exists) {
					indices.push_back(nextIndex);
					vertMap.push_back(temp_vert);
					nextIndex++;
					verts.push_back(rawVerts[temp_vert.v - 1]);
					uvs.push_back(rawUvs[temp_vert.uv - 1]);
					normals.push_back(rawNormals[temp_vert.n - 1]);
				}
			}
		// We have reached a new obj here, time to store our data and start again
		} else if (strcmp(command, "o") == 0) {

			// Init our model and mesh for our first obj
			if (model == nullptr) {
				model = new Model();
				mesh = new Mesh();
				continue;
			}

			// This is not our first obj, thus we must save data
			mesh->SetPositions(verts);
			mesh->SetNormals(normals);
			mesh->SetUvs(uvs);
			mesh->SetIndices(indices);

			rawVerts.clear();
			rawNormals.clear();
			rawUvs.clear();

			verts.clear();
			normals.clear();
			uvs.clear();
			indices.clear();

			mesh->bounds = new Bounds(minx, miny, minz, maxx, maxy, maxz);
			minx = INFINITY;
			miny = INFINITY;
			minz = INFINITY;
			maxx = -INFINITY;
			maxy = -INFINITY;
			maxz = -INFINITY;

			model->meshes.push_back(mesh);
			mesh = new Mesh();
		} else { continue; }
	}

	mesh->SetPositions(verts);
	mesh->SetNormals(normals);
	mesh->SetUvs(uvs);
	mesh->SetIndices(indices);

	rawVerts.clear();
	rawNormals.clear();
	rawUvs.clear();

	verts.clear();
	normals.clear();
	uvs.clear();
	indices.clear();

	mesh->bounds = new Bounds(minx, miny, minz, maxx, maxy, maxz);

	model->meshes.push_back(mesh);
	AssetManager::models.push_back(model);
	return model;
}

Material* AssetManager::LoadMaterial(const glm::vec3& c, const glm::vec3& a, const glm::vec3& d, const glm::vec3& s,
	const std::string& vert, const std::string& frag) {

	Material* m = new Material(c, a, d, s, vert, frag);

	for (int i = 0; i < materials.size(); i++) {
		if (*(materials[i]) == *m) {
			delete m;
			return materials[i];
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
	int windowWidth, windowHeight;

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
				windowWidth,
				windowHeight,
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

			currModel = tinyLoadObj(filename);
			currModel->name = currGameObject->name;

			// TODO: Hack for now, but eventually need to support materials in obj
			currModel->materials.push_back(currMaterial);
		} else if (strcmp(command, "material") == 0) { // If the command is a material
			float cr, cg, cb; // color
			float ar, ag, ab; // ambient coefficients
			float dr, dg, db; // diffuse coefficients
			float sr, sg, sb; // specular coefficients
			char vert[32];
			char frag[32];

			sscanf(line, "material %f %f %f %f %f %f %f %f %f %f %f %f %s %s",
				&cr, &cg, &cb, &ar, &ag, &ab, &dr, &dg, &db, &sr, &sg, &sb, &vert, &frag);

			currMaterial = LoadMaterial(glm::vec3(cr, cg, cb), glm::vec3(ar, ag, ab), glm::vec3(dr, dg, db), glm::vec3(sr, sg, sb),
				vert, frag);
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

}