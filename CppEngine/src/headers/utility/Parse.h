
#ifndef PARSE_H_
#define PARSE_H_

#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>

#include "Scene.h"
#include "Map.h"

// Global components
#include "Component.h"
#include "MeshRenderer.h"
#include "Collider.h"
#include "BoxCollider.h"
#include "SphereCollider.h"
#include "Transform.h"
#include "Camera.h"

// Local Components
#include "PlayerMovement.h"
#include "Key.h"
#include "Door.h"
#include "InteractableObject.h"
#include "Grab.h"

#include "Mesh.h"
#include "Bounds.h"
#include "GameObject.h"
#include "Material.h"

#include "Light.h"
#include "DirectionalLight.h"
#include "SpotLight.h"
#include "PointLight.h"
#include "AmbientLight.h"

#include "Globals.h"
#include "Configuration.h"

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

static void ObjParse(Mesh& mesh, const std::string fileName) {
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
    std::vector<Vec2> rawUvs;

    std::vector<glm::vec3> verts;
    std::vector<glm::vec3> normals;
    std::vector<Vec2> uvs;
    std::vector<unsigned int> indices;

    std::vector<vertData> vertMap;
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
            Vec2 uv;

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
        }
        else { continue; }
    }

    mesh.SetPositions(verts);
    mesh.SetNormals(normals);
    mesh.SetUvs(uvs);
    mesh.SetIndices(indices);

    rawVerts.clear();
    rawNormals.clear();
    rawUvs.clear();

    verts.clear();
    normals.clear();
    uvs.clear();
    indices.clear();

	mesh.bounds = new Bounds(minx, miny, minz, maxx, maxy, maxz);
}

static void SceneParse(Scene* scene, std::string fileName) {
    FILE *fp;
    char line[1024]; //Assumes no line is longer than 1024 characters!

	std::string fullFile = VK_ROOT_DIR"scenes/" + fileName;

    // open the file containing the scene description
    fp = fopen(fullFile.c_str(), "r");

    // check for errors in opening the file
    if (fp == NULL) {
        fprintf(stderr, "Can't open file '%s'\n", fullFile.c_str());
		exit(1);
    }

	GameObject* currGameObject = nullptr;
    Mesh* currMesh = nullptr;
	Material* currMaterial = nullptr;
    int window_width, window_height;

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

        if (strcmp(command, "aspect_ratio") == 0) {
            sscanf(line, "aspect_ratio %d %d", &window_width, &window_height);

            scene->window_width = window_width;
            scene->window_height = window_height;
        }
        else if (strcmp(command, "gameObject") == 0) {
			if (currGameObject != nullptr) {
				scene->gameObjects.push_back(currGameObject);
			}

            currGameObject = new GameObject();
            char name[1024];

            sscanf(line, "gameObject %s", name);
            currGameObject->name = name;
        }
        else if (strcmp(command, "camera") == 0) {
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
                window_width,
                window_height,
                fov, np, fp);
        }
        else if (strcmp(command, "component") == 0) {
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
            }
            else if (strcmp(type, "sphereCollider") == 0) {
                glm::vec3 pos;
                float radius;
                int dynamic;

                sscanf(line, "component sphereCollider %f %f %f %f %d",
                    &pos.x, &pos.y, &pos.z, &radius, &dynamic);

                if (currGameObject)
                    currGameObject->AddComponent(new SphereCollider(pos, radius, dynamic));
            }
            else if (strcmp(type, "meshRenderer") == 0) {
                if (currGameObject)
                    currGameObject->AddComponent(new MeshRenderer(currMesh, currMaterial));
            }
            else if (strcmp(type, "playerMovement") == 0) {
                float speed;

                sscanf(line, "component playerMovement %f", &speed);

                if (currGameObject)
                    currGameObject->AddComponent(new PlayerMovement(speed));
            }
            else if (strcmp(type, "key") == 0) {
                char p[1024];

                sscanf(line, "component key %s", p);

                if (currGameObject)
                    currGameObject->AddComponent(new Key(p));
            }
            else if (strcmp(type, "door") == 0) {
                char p[1024];

                sscanf(line, "component door %s", p);

                if (currGameObject)
                    currGameObject->AddComponent(new Door(p));
            }
            else if (strcmp(type, "interactableObject") == 0) {
                if (currGameObject)
                    currGameObject->AddComponent(new InteractableObject());
            }
            else if (strcmp(type, "grab") == 0) {
                if (currGameObject)
                    currGameObject->AddComponent(new Grab());
            }
            else { continue; }
        }
        else if (strcmp(command, "mesh") == 0) {
            char filename[1024];

            sscanf(line, "mesh %s", &filename);

			currMesh = new Mesh();
            ObjParse(*currMesh, filename);
            currMesh->name = currGameObject->name;
        }
        else if (strcmp(command, "material") == 0) { // If the command is a material
            float cr, cg, cb; // color
            float ar, ag, ab; // ambient coefficients
            float dr, dg, db; // diffuse coefficients
            float sr, sg, sb; // specular coefficients
            char vert[32];
            char frag[32];

            sscanf(line, "material %f %f %f %f %f %f %f %f %f %f %f %f %s %s",
                &cr, &cg, &cb, &ar, &ag, &ab, &dr, &dg, &db, &sr, &sg, &sb, &vert, &frag);

            currMaterial = new Material(glm::vec3(cr, cg, cb), glm::vec3(ar, ag, ab), glm::vec3(dr, dg, db), glm::vec3(sr, sg, sb), 
                vert, frag);
        }
        else if (strcmp(command, "directional_light") == 0) { // If the command is a directional light
            float r, g, b, a, dx, dy, dz, dw;
            sscanf(line, "directional_light %f %f %f %f %f %f %f %f",
                &r, &g, &b, &a, &dx, &dy, &dz, &dw);

            DirectionalLight* d = new DirectionalLight {
                glm::vec4(r, g, b, a),
                glm::normalize(glm::vec4(dx, dy, dz, dw))
            };

            scene->lights.push_back(d);
        }
        else if (strcmp(command, "spot_light") == 0) { // If the command is a point_light
            float r, g, b, a;
            float px, py, pz, pw;
            float dx, dy, dz, dw;
            float angle1, angle2;
            sscanf(line, "spot_light %f %f %f %f %f %f %f %f %f %f %f %f %f %f",
                &r, &g, &b, &a, &px, &py, &pz, &pw, &dx, &dy, &dz, &dw, &angle1, &angle2);

            SpotLight* s = new SpotLight {
                glm::vec4(r, g, b, a),
                glm::vec4(px, py, pz, pw),
                glm::normalize(glm::vec4(dx, dy, dz, dw)),
                angle1,
                angle2
            };

            scene->lights.push_back(s);
        }
        else if (strcmp(command, "point_light") == 0) { // If the command is a spot_light
            float r, g, b, a, x, y, z, w;
            sscanf(line, "point_light %f %f %f %f %f %f %f %f",
                &r, &g, &b, &a, &x, &y, &z, &w);
            
            PointLight* p = new PointLight {
                glm::vec4(r, g, b, a),
                glm::vec4(x, y, z, w)
            };

            scene->lights.push_back(p);
        }
        else if (strcmp(command, "ambient_light") == 0) { // If the command is a ambient_light
            float r, g, b, a;
            sscanf(line, "ambient_light %f %f %f %f", &r, &g, &b, &a);

            AmbientLight* amb = new AmbientLight {
                glm::vec4(r, g, b, a)
            };

            scene->lights.push_back(amb);
        }
        else {
            fprintf(stderr, "WARNING. Do not know command: %s\n", command);
        }
    }

	// Push in our last gameobject
	if (currGameObject != nullptr) {
		scene->gameObjects.push_back(currGameObject);
	}

    GameObject* g = scene->FindGameObject("floor");
    Transform* t = g->transform;
    t->SetPosition(glm::vec3(0, -1, 0));
    
    for (int i = 0; i < 1000; i++) {
        PointLight* p = new PointLight{
            glm::vec4(
                (float)rand() / (float)RAND_MAX * 4,
                (float)rand() / (float)RAND_MAX * 4,
                (float)rand() / (float)RAND_MAX * 4, 1),
            glm::vec4(
                (float)rand() / (float)RAND_MAX * 40,
                0.5,
                (float)rand() / (float)RAND_MAX * 40, 1)
        };
        scene->lights.push_back(p);
    }

	g = nullptr;
	t = nullptr;
	currMaterial = nullptr;
	currMesh = nullptr;
	currGameObject = nullptr;
}

static void MapParse(Map* map, std::string fileName, Scene* s) {
    FILE *fp;
    char line[1024]; //Assumes no line is longer than 1024 characters!

	std::string fullFile = VK_ROOT_DIR"maps/" + fileName;

    // open the file containing the scene description
    fp = fopen(fullFile.c_str(), "r");

    // check for errors in opening the file
    if (fp == NULL) {
        fprintf(stderr, "Can't open file '%s'\n", fullFile.c_str());
		exit(1);
    }

    int i = 0;
    int width, height;
    //Loop through reading each line
    while (fgets(line, 1024, fp)) { //Assumes no line is longer than 1024 characters!
        if (line[0] == '#') {
            //fprintf(stderr, "Skipping comment: %s", line);
            continue;
        }

        int fieldsRead; char row[100];

        // Bounds of grid
        int temp_width, temp_height;
        if ((fieldsRead = sscanf(line, "%d %d", &temp_width, &temp_height)) == 2) {
            width = temp_width;
            height = temp_height;
            map->Setup(width, height);
        }
        // Grid entries
        else if ((fieldsRead = sscanf(line, "%s", row)) == 1) {
            for (int j = 0; j < width; j++) {
                switch (row[j]) {
                case 's' :
                    map->layout.push_back(door1);
                    break;
                case '0' :
                    map->layout.push_back(empty);
                    break;
                default:
                    break;
                }
            }
            i++;
        }
        else { continue; }

    }

    s->instances.reserve(map->layout.size());
    // Create actual grid
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            bool flag = true;
            GameObject* g = new GameObject();
            Transform* t = new Transform();
            switch (map->layout[map->index(i, j)]) {
            case empty: {
                flag = false;
                break;
            }
            case door1: {
                g = new GameObject(*(s->FindGameObject("Suzzane")));
                break;
            }
            default:
                flag = false;
                break;
            }

            if (flag) {
                t = g->transform;
                t->SetPosition(glm::vec3((float)s->cube_width * i, 0, (float)s->cube_width * j));
                for (int k = 0; k < g->components.size(); k++) {
                    g->components[k]->gameObject = g;
                }
                s->instances.push_back(g);
            }
        }
    }

    GameObject* g = new GameObject(*(s->FindGameObject("floor")));
    Transform* t = g->transform;
    t->SetPosition(glm::vec3(0, -1, 0));
    for (int i = 0; i < g->components.size(); i++) {
        g->components[i]->gameObject = g;
    }
    s->instances.push_back(g);
}

#endif