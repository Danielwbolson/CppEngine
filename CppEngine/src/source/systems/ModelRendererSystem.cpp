
#include "ModelRendererSystem.h"
#include "Utility.h"
#include "Configuration.h"
#include "Scene.h"

#include "AssetManager.h"
#include "Camera.h"
#include "Globals.h"

#include "Material.h"
#include "Shader.h"

#include <cmath>

#include "glm/gtc/type_ptr.hpp"

void checkGLError(const std::string& s) { 
    GLenum err;
    if ((err = glGetError()) != 0) {
        std::cout << s + " :" << err << std::endl;
    }
}

ModelRendererSystem::ModelRendererSystem(const int& sW, const int& sH) {
    screenWidth = sW;
    screenHeight = sH;

    modelRenderers = std::vector<ModelRenderer*>();
    pointLights = std::vector<PointLight>();

	// We know that this is a mesh, not a model
    lightSphere = (assetManager->tinyLoadObj("sphere.obj"))->meshes[0];
}

ModelRendererSystem::~ModelRendererSystem() {
    glDeleteProgram(combinedShader);

    glDeleteBuffers(1, &quadVbo);
    glDeleteVertexArrays(1, &quadVao);

	glDeleteBuffers(1, &lightVolume_Ibo);
	glDeleteBuffers(1, &lightVolume_Vbo);
	glDeleteVertexArrays(1, &lightVolume_Vao);
	
	glDeleteFramebuffers(1, &gBuffer.id);
	glDeleteTextures(1, &gBuffer.positions);
	glDeleteTextures(1, &gBuffer.normals);
	glDeleteTextures(1, &gBuffer.diffuse);
	glDeleteTextures(1, &gBuffer.specular);

	for (int i = 0; i < modelRenderers.size(); i++) {
		modelRenderers[i] = nullptr;
	}
	modelRenderers.clear();

	pointLights.clear();
	modelRenderers.clear();
	lightPositions.clear();
}

void ModelRendererSystem::Setup() {
    for (int i = 0; i < mainScene->lights.size(); i++) {
        PointLight p = *((PointLight*)mainScene->lights[i]);
        pointLights.push_back(p);

        lightPositions.push_back(p.position);
    }

    // Get our list of related components, in this case MeshRenderers
    for (int i = 0; i < mainScene->instances.size(); i++) {
        ModelRenderer* mr = (ModelRenderer*)mainScene->instances[i]->GetComponent("modelRenderer");
        if (mr) {
			modelRenderers.push_back(mr);
            Register(mr);
        }
    }

    glEnable(GL_DEPTH_TEST);

    // Set up our shader for rendering to the texture
    combinedShader = util::initShaderFromFiles("deferredLightVolumes.vert", "deferredLightVolumes.frag");

    glGenVertexArrays(1, &lightVolume_Vao);
    glBindVertexArray(lightVolume_Vao);

    glGenBuffers(1, &lightVolume_Vbo);
    glBindBuffer(GL_ARRAY_BUFFER, lightVolume_Vbo);
    glBufferData(GL_ARRAY_BUFFER, lightSphere->positions.size() * 3 * sizeof(float), &(lightSphere->positions[0]), GL_STATIC_DRAW);
    glEnableVertexAttribArray(glGetAttribLocation(combinedShader, "inPos"));
    glVertexAttribPointer(glGetAttribLocation(combinedShader, "inPos"), 3, GL_FLOAT, GL_FALSE, 0, 0);

    // index buffer for sphere
    glGenBuffers(1, &lightVolume_Ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lightVolume_Ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, lightSphere->indices.size() * sizeof(GL_UNSIGNED_INT), &(lightSphere->indices[0]), GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    /*glGenVertexArrays(1, &quadVao);
    glBindVertexArray(quadVao);

    glGenBuffers(1, &quadVbo);
    glBindBuffer(GL_ARRAY_BUFFER, quadVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);
    glEnableVertexAttribArray(glGetAttribLocation(combinedShader, "inVertex"));
    glVertexAttribPointer(glGetAttribLocation(combinedShader, "inVertex"), 2, GL_FLOAT, GL_FALSE, 0, 0);*/

    glGenFramebuffers(1, &(gBuffer.id));
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer.id);

    // - position color buffer
    glGenTextures(1, &(gBuffer.positions));
    glBindTexture(GL_TEXTURE_2D, gBuffer.positions);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, screenWidth, screenHeight, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gBuffer.positions, 0);

    // - normal color buffer
    glGenTextures(1, &(gBuffer.normals));
    glBindTexture(GL_TEXTURE_2D, gBuffer.normals);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, screenWidth, screenHeight, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gBuffer.normals, 0);

    // diffuse color
    glGenTextures(1, &(gBuffer.diffuse));
    glBindTexture(GL_TEXTURE_2D, gBuffer.diffuse);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, screenWidth, screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gBuffer.diffuse, 0);

    // specular color
    glGenTextures(1, &(gBuffer.specular));
    glBindTexture(GL_TEXTURE_2D, gBuffer.specular);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenWidth, screenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gBuffer.specular, 0);

    // - tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
    GLuint attachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
    glDrawBuffers(4, attachments);

    //// Lights
    //glGenBuffers(1, &lightUBO);
    //glBindBuffer(GL_UNIFORM_BUFFER, lightUBO);
    //glBufferData(GL_UNIFORM_BUFFER, sizeof(PointLight) * numLights, &(pointLights[0]), GL_STATIC_DRAW);
    //glBindBuffer(GL_UNIFORM_BUFFER, 0);

    //glBindBufferRange(GL_UNIFORM_BUFFER, 0, lightUBO, 0, sizeof(PointLight) * numLights);

    //GLuint uniformBlockPointLights = glGetUniformBlockIndex(combinedShader, "pointLightBlock");
    //glBindBufferBase(GL_UNIFORM_BUFFER, 0, lightUBO);
    //glUniformBlockBinding(combinedShader, 0, uniformBlockPointLights);

    //glBindBuffer(GL_UNIFORM_BUFFER, lightUBO);
    //GLvoid* p = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
    //memcpy(p, &(pointLights[0]), sizeof(PointLight) * numLights);
    //glUnmapBuffer(GL_UNIFORM_BUFFER);

    GLuint rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, screenWidth, screenHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "frame buffer incomplete" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void ModelRendererSystem::Register(const Component* c) {
    // Quick reference
    ModelRenderer* mr = (ModelRenderer*)c;

	// run through every mesh/material for our model
	for (int i = 0; i < mr->numMeshes; i++) {
		glGenVertexArrays(1, &(mr->vaos[i]));
		glBindVertexArray(mr->vaos[i]);

		// Position
		glGenBuffers(1, &(mr->vbos[i][0]));
		glBindBuffer(GL_ARRAY_BUFFER, mr->vbos[i][0]);
		glBufferData(GL_ARRAY_BUFFER, 3 * mr->model->meshes[i]->positions.size() * sizeof(float), &(mr->model->meshes[i]->positions[0]), GL_STATIC_DRAW);

		GLint posAttrib = glGetAttribLocation(mr->model->materials[i]->shader->shaderProgram, "inPos");
		glEnableVertexAttribArray(posAttrib);
		glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
		//Attribute, vals/attrib., type, isNormalized, stride, offset

		// Normals
		glGenBuffers(1, &(mr->vbos[i][1]));
		glBindBuffer(GL_ARRAY_BUFFER, mr->vbos[i][1]);
		glBufferData(GL_ARRAY_BUFFER, 3 * mr->model->meshes[i]->normals.size() * sizeof(float), &(mr->model->meshes[i]->normals[0]), GL_STATIC_DRAW);

		GLint normAttrib = glGetAttribLocation(mr->model->materials[i]->shader->shaderProgram, "inNorm");
		glEnableVertexAttribArray(normAttrib);
		glVertexAttribPointer(normAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);

		// UVS
		glGenBuffers(1, &(mr->vbos[i][2]));
		glBindBuffer(GL_ARRAY_BUFFER, mr->vbos[i][2]);
		glBufferData(GL_ARRAY_BUFFER, 2 * mr->model->meshes[i]->uvs.size() * sizeof(float), &(mr->model->meshes[i]->uvs[0]), GL_STATIC_DRAW);

		GLint uvAttrib = glGetAttribLocation(mr->model->materials[i]->shader->shaderProgram, "inUV");
		glEnableVertexAttribArray(uvAttrib);
		glVertexAttribPointer(uvAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);

		// Indices
		glGenBuffers(1, &(mr->vbos[i][3]));
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mr->vbos[i][3]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, mr->model->meshes[i]->indices.size() * sizeof(GL_UNSIGNED_INT), &(mr->model->meshes[i]->indices[0]), GL_STATIC_DRAW);


		// Textures
		// Load up either a null texture or the wanted one
		glUseProgram(mr->model->materials[i]->shader->shaderProgram);
		Material* mat = mr->model->materials[i];

		if (mat->ambientTexture != nullptr && !mat->ambientTexture->loadedToGPU) {
			assetManager->LoadTextureToGPU("ambient", mat->ambientIndex, 0, mat->ambientTexture);
		} else if (mat->diffuseTexture != nullptr && !mat->diffuseTexture->loadedToGPU) {
			assetManager->LoadTextureToGPU("diffuse", mat->diffuseIndex, 1, mat->diffuseTexture);
		} else if (mat->specularTexture != nullptr && !mat->specularTexture->loadedToGPU) {
			assetManager->LoadTextureToGPU("specular", mat->specularIndex, 2, mat->specularTexture);
		} else if (mat->specularHighLightTexture != nullptr && !mat->specularHighLightTexture->loadedToGPU) {
			assetManager->LoadTextureToGPU("specularHighlight", mat->specularHighLightIndex, 3, mat->specularHighLightTexture);
		} else if (mat->bumpTexture != nullptr && !mat->bumpTexture->loadedToGPU) {
			assetManager->LoadTextureToGPU("bump", mat->bumpIndex, 4, mat->bumpTexture);
		} else if (mat->displacementTexture != nullptr && !mat->displacementTexture->loadedToGPU) {
			assetManager->LoadTextureToGPU("displacement", mat->displacementIndex, 5, mat->displacementTexture);
		} else if (mat->alphaTexture != nullptr && !mat->alphaTexture->loadedToGPU) {
			assetManager->LoadTextureToGPU("alpha", mat->alphaIndex, 6, mat->alphaTexture);
		}

		mat = nullptr;
	}
}

void ModelRendererSystem::Render() {
	checkGLError("Top of render");
	totalTriangles = 0;

    // Bind our deferred texture buffer
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer.id);

    // Clear the screen to default color
    glClearColor(0, 0, 0, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_CULL_FACE);

    glm::mat4 view = mainCamera->view;
    glm::mat4 proj = mainCamera->proj;

    // Get all of our meshRenderers to draw to textures
    for (int i = 0; i < modelRenderers.size(); i++) {

		glm::mat4 model = modelRenderers[i]->gameObject->transform->model;

		for (int j = 0; j < modelRenderers[i]->numMeshes; j++) {

			// Pre-emptively frustum cull unnecessary meshes
			if (FrustumCull(modelRenderers[i]->model->meshes[j], model, proj * view)) { continue; }

			glUseProgram(modelRenderers[i]->model->materials[j]->shader->shaderProgram);
			glBindVertexArray(modelRenderers[i]->vaos[j]);

			Material* m = modelRenderers[i]->model->materials[j];

			GLint uniModel = glGetUniformLocation(m->shader->shaderProgram, "model");
			glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
			GLint uniView = glGetUniformLocation(m->shader->shaderProgram, "view");
			glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));
			GLint uniProj = glGetUniformLocation(m->shader->shaderProgram, "proj");
			glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));


			// Material variables
			GLint uniAmbient = glGetUniformLocation(m->shader->shaderProgram, "ambient");
			glUniform3f(uniAmbient, m->ambient.r, m->ambient.g, m->ambient.b);
			GLint uniDiffuse = glGetUniformLocation(m->shader->shaderProgram, "diffuse");
			glUniform3f(uniDiffuse, m->diffuse.r, m->diffuse.g, m->diffuse.b);
			GLint uniSpecular = glGetUniformLocation(m->shader->shaderProgram, "specular");
			glUniform3f(uniSpecular, m->specular.r, m->specular.g, m->specular.b);
			GLint uniSpecularExp = glGetUniformLocation(m->shader->shaderProgram, "specularExp");
			glUniform1f(uniSpecularExp, m->specularExponent);
			GLint uniOpacity = glGetUniformLocation(m->shader->shaderProgram, "opacity");
			glUniform1f(uniOpacity, m->opacity);

			// Textures and booleans
			glActiveTexture(GL_TEXTURE0);
			if (m->ambientTexture != nullptr) {
				glBindTexture(GL_TEXTURE_2D, assetManager->ambientTextures[m->ambientIndex]);
			} else {
				glBindTexture(GL_TEXTURE_2D, assetManager->nullTexture);
			}
			glUniform1i(glGetUniformLocation(m->shader->shaderProgram, "ambientTex"), 0);

			glActiveTexture(GL_TEXTURE0 + 1);
			if (m->diffuseTexture != nullptr) {
				glBindTexture(GL_TEXTURE_2D, assetManager->diffuseTextures[m->diffuseIndex]);
			} else {
				glBindTexture(GL_TEXTURE_2D, assetManager->nullTexture);
			}
			glUniform1i(glGetUniformLocation(m->shader->shaderProgram, "diffuseTex"), 1);

			glActiveTexture(GL_TEXTURE0 + 2);
			if (m->specularTexture != nullptr) {
				glBindTexture(GL_TEXTURE_2D, assetManager->specularTextures[m->specularIndex]);
			} else {
				glBindTexture(GL_TEXTURE_2D, assetManager->nullTexture);
			}
			glUniform1i(glGetUniformLocation(m->shader->shaderProgram, "specularTex"), 2);

			glActiveTexture(GL_TEXTURE0 + 3);
			if (m->specularHighLightTexture != nullptr) {
				glBindTexture(GL_TEXTURE_2D, assetManager->specularHighLightTextures[m->specularHighLightIndex]);
			} else {
				glBindTexture(GL_TEXTURE_2D, assetManager->nullTexture);
			}
			glUniform1i(glGetUniformLocation(m->shader->shaderProgram, "specularHighLightTex"), 3);


			glActiveTexture(GL_TEXTURE0 + 4);
			if (m->bumpTexture != nullptr) {
				glBindTexture(GL_TEXTURE_2D, assetManager->bumpTextures[m->bumpIndex]);
			} else {
				glBindTexture(GL_TEXTURE_2D, assetManager->nullTexture);
			}
			glUniform1i(glGetUniformLocation(m->shader->shaderProgram, "bumpTex"), 4);


			glActiveTexture(GL_TEXTURE0 + 5);
			if (m->displacementTexture != nullptr) {
				glBindTexture(GL_TEXTURE_2D, assetManager->displacementTextures[m->displacementIndex]);
			} else {
				glBindTexture(GL_TEXTURE_2D, assetManager->nullTexture);
			}
			glUniform1i(glGetUniformLocation(m->shader->shaderProgram, "dispTex"), 5);


			glActiveTexture(GL_TEXTURE0 + 6);
			if (m->alphaTexture != nullptr) {
				glBindTexture(GL_TEXTURE_2D, assetManager->alphaTextures[m->alphaIndex]);
			} else {
				glBindTexture(GL_TEXTURE_2D, assetManager->nullTexture);
			}
			glUniform1i(glGetUniformLocation(m->shader->shaderProgram, "alphaTex"), 6);

			checkGLError("After texture bind");

			// Indices
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelRenderers[i]->vbos[j][3]);

			totalTriangles += static_cast<int>(modelRenderers[i]->model->meshes[j]->indices.size()) / 3;

			// Use our shader and draw our program
			glDrawElements(GL_TRIANGLES, static_cast<int>(modelRenderers[i]->model->meshes[j]->indices.size()), GL_UNSIGNED_INT, 0); //Number of vertices
		}
    }

	checkGLError("After draw to textures");
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, screenWidth, screenHeight);

    glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer.id);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, screenWidth, screenHeight, 0, 0, screenWidth, screenHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

    glUseProgram(combinedShader);
    glBindVertexArray(lightVolume_Vao);

    // View
    GLint uniView = glGetUniformLocation(combinedShader, "view");
    glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gBuffer.positions);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gBuffer.normals);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, gBuffer.diffuse);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, gBuffer.specular);
    glUniform1i(glGetUniformLocation(combinedShader, "gPosition"), 0);
    glUniform1i(glGetUniformLocation(combinedShader, "gNormal"), 1);
    glUniform1i(glGetUniformLocation(combinedShader, "gDiffuse"), 2);
    glUniform1i(glGetUniformLocation(combinedShader, "gSpecularExp"), 3);

	glm::vec3 camPos = mainCamera->transform->position;

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lightVolume_Ibo);

    // instead of drawing arrays, draw spheres at each light position
    for (int i = 0; i < pointLights.size(); i++) {
        glm::vec4 pos = pointLights[i].position;
        glm::vec4 color = pointLights[i].color;

        float lum = .6f * color.g + .3f * color.r + .1f * color.b;
		float radius = 14; // falloff where edge of radius is 1/10000th the color

        glm::mat4 model = glm::mat4(1);
        model = glm::translate(model, glm::vec3(pos.x, pos.y, pos.z));
        model = glm::scale(model, lum * glm::vec3(radius, radius, radius));

		// Pre-emptively frustum cull unnecessary meshes
		if (FrustumCull(lightSphere, model, proj * view)) { continue; }

		glm::mat4 pvmMatrix = proj * view * model;

		// Switch culling if inside light volume
		if (glm::length(camPos - glm::vec3(pos.x, pos.y, pos.z)) < lum * radius) {
			glCullFace(GL_FRONT);
		} else {
			glCullFace(GL_BACK);
		}

        GLint pvm = glGetUniformLocation(combinedShader, "pvm");
        glUniformMatrix4fv(pvm, 1, GL_FALSE, glm::value_ptr(pvmMatrix));
        GLint lightPos = glGetUniformLocation(combinedShader, "lightPos");
        glUniform4f(lightPos, pos.x, pos.y, pos.z, pos.w);
        GLint lightCol = glGetUniformLocation(combinedShader, "lightCol");
        glUniform4f(lightCol, color.r, color.g, color.b, color.a);

		totalTriangles += static_cast<int>(lightSphere->indices.size()) / 3;

        // User our shader and draw our program
        glDrawElements(GL_TRIANGLES, static_cast<int>(lightSphere->indices.size()), GL_UNSIGNED_INT, 0); //Number of vertices
    }

	glCullFace(GL_BACK);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

bool ModelRendererSystem::FrustumCull(const Mesh* mesh, const glm::mat4& model, const glm::mat4& projViewMat) const {
	
	Bounds b = *(mesh->bounds);
	glm::vec3 maxPoint = b.Max(model);
	glm::vec3 minPoint = b.Min(model);

	/* https://fgiesen.wordpress.com/2010/10/17/view-frustum-culling/ 
	* Method 4 
		Switched to this method instead of checking each bounding box point
		because I ran into issues with large meshes. I would be looking 
		directly at the mesh and all of its bounds points are ouside the 
		frustum. So even though it is right in front of me, it is culled
		due to only checking it's extremes.

		Checks if every plane can "see" the most likely point. If every plane can see
		at least one point that means our mesh is at least partially visible.
	*/
	std::vector<glm::vec4> planes = mainCamera->frustumPlanes;

	int success = 0;
	for (int j = 0; j < 6; j++) {
		float val = fmax(minPoint.x * planes[j].x, maxPoint.x * planes[j].x)
			+ fmax(minPoint.y * planes[j].y, maxPoint.y * planes[j].y)
			+ fmax(minPoint.z * planes[j].z, maxPoint.z * planes[j].z)
			+ planes[j].w;
		success += (val > 0);
	}

	if (success == 6) { 
		return false;
	}

	return true;
}