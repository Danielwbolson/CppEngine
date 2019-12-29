
#include "RendererSystem.h"

#include <chrono>
#include <cmath>
#include "glm/gtc/type_ptr.hpp"

#include "Utility.h"
#include "Configuration.h"
#include "Scene.h"

#include "AssetManager.h"
#include "Camera.h"
#include "Globals.h"

#include "Material.h"
#include "Shader.h"



RendererSystem::RendererSystem(const int& sW, const int& sH) {
	screenWidth = sW;
	screenHeight = sH;

	modelRenderers = std::vector<ModelRenderer*>();
	meshesToDraw = std::vector<MeshToDraw>();
	transparentToDraw = std::vector<MeshToDraw>();

	pointLights = std::vector<PointLight*>();
	pointLightsToDraw = std::vector<PointLightToDraw>();
	pointLightsToGPU = std::vector<PointLightToGPU>();

	// We know that this is a mesh, not a model
	lightVolume = (assetManager->tinyLoadObj("sphere"))->meshes[0];
}

RendererSystem::~RendererSystem() {
	glDeleteProgram(lightVolumeShader);

	glDeleteBuffers(1, &lightVolumeIBO);
	glDeleteBuffers(1, &lightVolumeVBO);
	glDeleteVertexArrays(1, &lightVolumeVAO);
	
	glDeleteFramebuffers(1, &gBuffer.id);
	glDeleteTextures(1, &gBuffer.normals);
	glDeleteTextures(1, &gBuffer.diffuseSpec);
	glDeleteTextures(1, &gBuffer.depth);

	for (int i = 0; i < modelRenderers.size(); i++) {
		modelRenderers[i] = nullptr;
	}
	modelRenderers.clear();

	for (int i = 0; i < pointLights.size(); i++) {
		pointLights[i] = nullptr;
	}
	pointLights.clear();

	sun = nullptr;

	meshesToDraw.clear();
	transparentToDraw.clear();
}

void RendererSystem::Setup() {

	// Get all of our lights
	for (int i = 0; i < mainScene->lights.size(); i++) {

		if (mainScene->lights[i]->GetType() == "pointLight") {
			pointLights.push_back((PointLight*)mainScene->lights[i]);
		} else if (mainScene->lights[i]->GetType() == "directionalLight") {
			sun = (DirectionalLight*)mainScene->lights[i];
		}

	}

	// Get our list of related components, in this case MeshRenderers
	for (int i = 0; i < mainScene->instances.size(); i++) {
		ModelRenderer* mr = (ModelRenderer*)mainScene->instances[i]->GetComponent("modelRenderer");
		if (mr) {
			modelRenderers.push_back(mr);
			Register(mr);
		}
	}

	// Set up debugging support
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(util::DebugMessageCallback, 0);
	GLuint unusedIds = 0;
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, &unusedIds, true);

	glGenQueries(1, &timeQuery);

	// Enable/Disable macros
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);


	// Set up our light volume shader
	{
		lightVolumeShader = util::initShaderFromFiles("deferredLightVolumes.vert", "deferredLightVolumes.frag");


		// Set up light volume mesh
		glGenVertexArrays(1, &lightVolumeVAO);
		glBindVertexArray(lightVolumeVAO);

		glGenBuffers(1, &lightVolumeVBO);
		glBindBuffer(GL_ARRAY_BUFFER, lightVolumeVBO);
		glBufferData(GL_ARRAY_BUFFER, lightVolume->positions.size() * 3 * sizeof(float), &(lightVolume->positions[0]), GL_STATIC_DRAW);
		glEnableVertexAttribArray(glGetAttribLocation(lightVolumeShader, "inPos"));
		glVertexAttribPointer(glGetAttribLocation(lightVolumeShader, "inPos"), 3, GL_FLOAT, GL_FALSE, 0, 0);

		// index buffer for sphere
		glGenBuffers(1, &lightVolumeIBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lightVolumeIBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, lightVolume->indices.size() * sizeof(GL_UNSIGNED_INT), &(lightVolume->indices[0]), GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);


		// Set up textures
		glGenFramebuffers(1, &(gBuffer.id));
		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer.id);

		// normal color buffer
		glGenTextures(1, &(gBuffer.normals));
		glBindTexture(GL_TEXTURE_2D, gBuffer.normals);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, screenWidth, screenHeight, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gBuffer.normals, 0);

		// diffuse color
		glGenTextures(1, &(gBuffer.diffuseSpec));
		glBindTexture(GL_TEXTURE_2D, gBuffer.diffuseSpec);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16UI, screenWidth, screenHeight, 0, GL_RGBA_INTEGER, GL_UNSIGNED_INT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gBuffer.diffuseSpec, 0);

		// depth
		glGenTextures(1, &(gBuffer.depth));
		glBindTexture(GL_TEXTURE_2D, gBuffer.depth);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, screenWidth, screenHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, gBuffer.depth, 0);

		// - tell OpenGL which attachments we'll use (of this framebuffer) for rendering 
		GLuint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(2, attachments);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Set up our directional light shader
	{
		directionalLightShader = util::initShaderFromFiles("deferredDirectionalLight.vert", "deferredDirectionalLight.frag");


		// Set up quad mesh
		glGenVertexArrays(1, &quadVAO);
		glBindVertexArray(quadVAO);

		glGenBuffers(1, &quadVBO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), quadVerts, GL_STATIC_DRAW);

		GLint posAttrib = glGetAttribLocation(directionalLightShader, "inPos");
		glEnableVertexAttribArray(posAttrib);
		glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);

	}

	// Create depth buffer and map for shadow maps from directional light
	{
		shadowMapShader = util::initShaderFromFiles("shadowMap.vert", "shadowMap.frag");

		glGenFramebuffers(1, &depthMapFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);

		// Set up our depth texture
		glGenTextures(1, &depthMap);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowWidth, shadowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

		// Set up our depth framebuffer
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	}

	// Create our framebufferobject and final render texture
	{
		finalQuadShader = util::initShaderFromFiles("finalQuad.vert", "finalQuad.frag");

		glGenFramebuffers(1, &finalQuadFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, finalQuadFBO);

		glGenTextures(1, &finalQuadRender);
		glBindTexture(GL_TEXTURE_2D, finalQuadRender);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8, windowWidth, windowHeight, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float borderColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, finalQuadRender, 0);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);

		GLuint rboDepth;
		glGenRenderbuffers(1, &rboDepth);
		glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, screenWidth, screenHeight);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

		GLint posAttrib = glGetAttribLocation(finalQuadShader, "inPos");
		glEnableVertexAttribArray(posAttrib);
		glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void RendererSystem::Register(const Component* c) {
	// Quick reference
	ModelRenderer* mr = (ModelRenderer*)c;

	// run through every mesh/material for our model
	for (int i = 0; i < mr->numMeshes; i++) {

		// Set up our shader based on if we need transparency or not
		Material* mat = mr->model->materials[i];
		if (!mat->isTransparent) {
			mat->shader = assetManager->shaders[0];
		} else {
			mat->shader = assetManager->shaders[1];
		}

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


		// Tangents
		glGenBuffers(1, &(mr->vbos[i][4]));
		glBindBuffer(GL_ARRAY_BUFFER, mr->vbos[i][4]);
		glBufferData(GL_ARRAY_BUFFER, 3 * mr->model->meshes[i]->tangents.size() * sizeof(float), &(mr->model->meshes[i]->tangents[0]), GL_STATIC_DRAW);

		GLint tanAttrib = glGetAttribLocation(mr->model->materials[i]->shader->shaderProgram, "inTan");
		glEnableVertexAttribArray(tanAttrib);
		glVertexAttribPointer(tanAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);


		// Bitangents
		glGenBuffers(1, &(mr->vbos[i][5]));
		glBindBuffer(GL_ARRAY_BUFFER, mr->vbos[i][5]);
		glBufferData(GL_ARRAY_BUFFER, 3 * mr->model->meshes[i]->bitangents.size() * sizeof(float), &(mr->model->meshes[i]->bitangents[0]), GL_STATIC_DRAW);

		GLint bitanAttrib = glGetAttribLocation(mr->model->materials[i]->shader->shaderProgram, "inBitan");
		glEnableVertexAttribArray(bitanAttrib);
		glVertexAttribPointer(bitanAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);


		glUseProgram(mr->model->materials[i]->shader->shaderProgram);


		// Set up SSBO for forward rendering for our transparent objects
		// Only do this once as our transparent objects are all in same shader
		if (mat->isTransparent && pointLightsSSBO == 0) {
			glGenBuffers(1, &pointLightsSSBO);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, pointLightsSSBO);
			pointLightsToGPU.reserve(pointLights.size());
			glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(PointLightToGPU) * pointLights.size(), pointLightsToGPU.data(), GL_DYNAMIC_DRAW);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, pointLightsSSBO);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		}


		// Textures
		// Load up either a null texture or the wanted one
		if (mat->ambientTexture != nullptr && !mat->ambientTexture->loadedToGPU) {
			assetManager->LoadTextureToGPU("ambient", mat->ambientIndex, 0, mat->ambientTexture);
		} 
		if (mat->diffuseTexture != nullptr && !mat->diffuseTexture->loadedToGPU) {
			assetManager->LoadTextureToGPU("diffuse", mat->diffuseIndex, 1, mat->diffuseTexture);
		} 
		if (mat->specularTexture != nullptr && !mat->specularTexture->loadedToGPU) {
			assetManager->LoadTextureToGPU("specular", mat->specularIndex, 2, mat->specularTexture);
		} 
		if (mat->specularHighLightTexture != nullptr && !mat->specularHighLightTexture->loadedToGPU) {
			assetManager->LoadTextureToGPU("specularHighlight", mat->specularHighLightIndex, 3, mat->specularHighLightTexture);
		} 
		if (mat->bumpTexture != nullptr && !mat->bumpTexture->loadedToGPU) {
			assetManager->LoadTextureToGPU("bump", mat->bumpIndex, 4, mat->bumpTexture);
			mat->usingBump = true;
		}
		if (mat->normalTexture != nullptr && !mat->normalTexture->loadedToGPU) {
			assetManager->LoadTextureToGPU("normal", mat->normalIndex, 5, mat->normalTexture);
			mat->usingNormal = true;
		}
		if (mat->displacementTexture != nullptr && !mat->displacementTexture->loadedToGPU) {
			assetManager->LoadTextureToGPU("displacement", mat->displacementIndex, 6, mat->displacementTexture);
		} 
		if (mat->alphaTexture != nullptr && !mat->alphaTexture->loadedToGPU) {
			assetManager->LoadTextureToGPU("alpha", mat->alphaIndex, 7, mat->alphaTexture);
		}

		mat->InitUniforms();
		mat = nullptr;
	}
}

void RendererSystem::Render() {

	totalTriangles = 0;

	// First, cull out unwanted geometry. Currently, only Frustum Culling is supported
	auto startTime = std::chrono::high_resolution_clock::now();
	CullScene();
	auto endTime = std::chrono::high_resolution_clock::now();
	cullTime = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();

	// Next, calculate our shadow map using our directional light only
	// We do this every frame because we are assuming the directional light will move
	glBeginQuery(GL_TIME_ELAPSED, timeQuery);
	DrawShadows();
	glEndQuery(GL_TIME_ELAPSED);
	glGetQueryObjecti64v(timeQuery, GL_QUERY_RESULT, &shadowTime);

	view = mainCamera->view;
	proj = mainCamera->proj;

	// Next, draw our opaque geometry to our buffers for deferred shading and then calculate lighting for our deferred objects
	glBeginQuery(GL_TIME_ELAPSED, timeQuery);
	DeferredToTexture();
	glEndQuery(GL_TIME_ELAPSED);
	glGetQueryObjecti64v(timeQuery, GL_QUERY_RESULT, &deferredToTexTime);

	glBeginQuery(GL_TIME_ELAPSED, timeQuery);
	DeferredLighting();
	glEndQuery(GL_TIME_ELAPSED);
	glGetQueryObjecti64v(timeQuery, GL_QUERY_RESULT, &deferredLightsTime);

	// Next, draw our transparent items in a forward rendering pass
	glBeginQuery(GL_TIME_ELAPSED, timeQuery);
	DrawTransparent();
	glEndQuery(GL_TIME_ELAPSED);
	glGetQueryObjecti64v(timeQuery, GL_QUERY_RESULT, &transparentTime);

	// Next, do post processing effects on final image.
	glBeginQuery(GL_TIME_ELAPSED, timeQuery);
	PostProcess();
	glEndQuery(GL_TIME_ELAPSED);
	glGetQueryObjecti64v(timeQuery, GL_QUERY_RESULT, &postFXXTime);

	// Finally, render final image to 2D quad that is the size of the screen
	//DrawQuad();
}

void RendererSystem::CullScene() {

	// Get all of our meshes that are not frustum culled. These will be used for later drawing
	meshesToDraw.clear();
	transparentToDraw.clear();
	for (int i = 0; i < modelRenderers.size(); i++) {
		glm::mat4 model = modelRenderers[i]->gameObject->transform->model;
		for (int j = 0; j < modelRenderers[i]->numMeshes; j++) {

			if (!ShouldFrustumCull(modelRenderers[i]->model->meshes[j], model)) {

				MeshToDraw m = MeshToDraw{
					m.mesh = modelRenderers[i]->model->meshes[j],
					m.material = modelRenderers[i]->model->materials[j],
					m.model = model,
					m.vao = modelRenderers[i]->vaos[j],
					m.shaderProgram = modelRenderers[i]->model->materials[j]->shader->shaderProgram,
					m.position = modelRenderers[i]->gameObject->transform->position
				};

				if (m.material->isTransparent) {
					transparentToDraw.push_back(m);
				} else {
					meshesToDraw.push_back(m);
				}

			}

		}
	}

	// Get all of our lights that will be used for drawing.
	pointLightsToDraw.clear();
	pointLightsToGPU.clear();
	for (int i = 0; i < pointLights.size(); i++) {
		glm::vec4 pos = pointLights[i]->position;
		glm::vec3 color = pointLights[i]->color;
		float radius = pointLights[i]->radius;
		float lum = pointLights[i]->lum;

		glm::mat4 model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(pos.x, pos.y, pos.z));
		model = glm::scale(model, glm::vec3(radius, radius, radius));

		if (!ShouldFrustumCull(lightVolume, model)) {

			PointLightToDraw pToDraw = PointLightToDraw{
				pToDraw.model = model,
				pToDraw.position = pos,
				pToDraw.luminance = lum,
				pToDraw.color = color,
				pToDraw.radius = radius
			};
			pointLightsToDraw.push_back(pToDraw);

			PointLightToGPU pToGPU = PointLightToGPU{
				pToGPU.position = pos,
				pToGPU.color = color,
				pToGPU.luminance = lum
			};
			pointLightsToGPU.push_back(pToGPU);
		}
	}

}

void RendererSystem::DrawShadows() {

	// Set our viewport for our shadow map and link our depth FBO
	glViewport(0, 0, shadowWidth, shadowHeight);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);

	// Clear our depth bit as we are going to overwrite it with our new shadow map
	glClear(GL_DEPTH_BUFFER_BIT);

	// Actually draw to our depth buffer
	glUseProgram(shadowMapShader);

	// Create matrices to simulate drawing from our directional light's perspective
	proj = glm::ortho(-30.0f, 10.0f, -15.0f, 40.0f, 0.01f, 1000.0f);
	glm::vec3 pos = glm::vec3(sun->direction) * -100.0f;
	glm::vec3 up = glm::cross(glm::vec3(sun->direction), glm::vec3(0, 0, -1));
	view = glm::lookAt(pos, pos + 100.0f * glm::vec3(sun->direction), glm::normalize(up));
	lightProjView = proj * view;
	glUniformMatrix4fv(glGetUniformLocation(shadowMapShader, "lightProjView"), 1, GL_FALSE, glm::value_ptr(lightProjView));

	GLint uniModel = glGetUniformLocation(shadowMapShader, "model");

	for (int i = 0; i < meshesToDraw.size(); i++) {
		glBindVertexArray(meshesToDraw[i].vao);

		glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(meshesToDraw[i].model));

		totalTriangles += static_cast<int>(meshesToDraw[i].mesh->indices.size()) / 3;

		// Use our shader and draw our program
		glDrawElements(GL_TRIANGLES, static_cast<int>(meshesToDraw[i].mesh->indices.size()), GL_UNSIGNED_INT, 0); //Number of vertices
	}

	for (int i = 0; i < transparentToDraw.size(); i++) {
		glBindVertexArray(transparentToDraw[i].vao);

		glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(transparentToDraw[i].model));

		totalTriangles += static_cast<int>(transparentToDraw[i].mesh->indices.size()) / 3;

		// Use our shader and draw our program
		glDrawElements(GL_TRIANGLES, static_cast<int>(transparentToDraw[i].mesh->indices.size()), GL_UNSIGNED_INT, 0); //Number of vertices
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void RendererSystem::DeferredToTexture() {

	glViewport(0, 0, windowWidth, windowHeight);

	// Bind the output framebuffer from our deferred shading
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer.id);
	// Clear the buffer to default color
	glClearColor(0, 0, 0, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// All opaque meshes use same shader
	if (meshesToDraw.size() > 0) {
		glUseProgram(meshesToDraw[0].shaderProgram);
	}

	// Draw all of our wanted meshRendereres
	for (int i = 0; i < meshesToDraw.size(); i++) {

		glBindVertexArray(meshesToDraw[i].vao);

		Material* m = meshesToDraw[i].material;

		glUniformMatrix4fv(m->uniModel, 1, GL_FALSE, glm::value_ptr(meshesToDraw[i].model));
		glUniformMatrix4fv(m->uniView, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(m->uniProj, 1, GL_FALSE, glm::value_ptr(proj));

		glUniform3f(m->uniAmbient, m->ambient.r, m->ambient.g, m->ambient.b);
		glUniform3f(m->uniDiffuse, m->diffuse.r, m->diffuse.g, m->diffuse.b);
		glUniform3f(m->uniSpecular, m->specular.r, m->specular.g, m->specular.b);
		glUniform1f(m->uniSpecularExp, m->specularExponent);

		glUniform1i(m->uniUsingNormal, m->usingNormal);


		// How could I change the following IF statements to avoid last second state changes?
		// Could I call specific functions?

		glActiveTexture(GL_TEXTURE0 + 0);
		if (m->diffuseTexture != nullptr) {
			glBindTexture(GL_TEXTURE_2D, assetManager->diffuseTextures[m->diffuseIndex]);
		} else {
			glBindTexture(GL_TEXTURE_2D, assetManager->nullTexture);
		}
		glUniform1i(m->uniDiffuseTex, 0);

		glActiveTexture(GL_TEXTURE0 + 1);
		if (m->specularTexture != nullptr) {
			glBindTexture(GL_TEXTURE_2D, assetManager->specularTextures[m->specularIndex]);
		} else {
			glBindTexture(GL_TEXTURE_2D, assetManager->nullTexture);
		}
		glUniform1i(m->uniSpecularTex, 1);

		glActiveTexture(GL_TEXTURE0 + 2);
		if (m->specularHighLightTexture != nullptr) {
			glBindTexture(GL_TEXTURE_2D, assetManager->specularHighLightTextures[m->specularHighLightIndex]);
		} else {
			glBindTexture(GL_TEXTURE_2D, assetManager->nullTexture);
		}
		glUniform1i(m->uniSpecularHighLightTex, 2);

		glActiveTexture(GL_TEXTURE0 + 3);
		if (m->bumpTexture != nullptr) {
			glBindTexture(GL_TEXTURE_2D, assetManager->bumpTextures[m->bumpIndex]);
		} else {
			glBindTexture(GL_TEXTURE_2D, assetManager->nullTexture);
		}
		glUniform1i(m->uniBumpTex, 3);

		glActiveTexture(GL_TEXTURE0 + 4);
		if (m->normalTexture != nullptr) {
			glBindTexture(GL_TEXTURE_2D, assetManager->normalTextures[m->normalIndex]);
		} else {
			glBindTexture(GL_TEXTURE_2D, assetManager->nullTexture);
		}
		glUniform1i(m->uniNormalTex, 4);

		glActiveTexture(GL_TEXTURE0 + 5);
		if (m->displacementTexture != nullptr) {
			glBindTexture(GL_TEXTURE_2D, assetManager->displacementTextures[m->displacementIndex]);
		} else {
			glBindTexture(GL_TEXTURE_2D, assetManager->nullTexture);
		}
		glUniform1i(m->uniDisplacementTex, 5);


		totalTriangles += static_cast<int>(meshesToDraw[i].mesh->indices.size()) / 3;

		// Use our shader and draw our program
		glDrawElements(GL_TRIANGLES, static_cast<int>(meshesToDraw[i].mesh->indices.size()), GL_UNSIGNED_INT, 0); //Number of vertices
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void RendererSystem::DeferredLighting() {

	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	glm::vec3 camPos = mainCamera->transform->position;

	// Bind the output framebuffer from our deferred shading
	glBindFramebuffer(GL_FRAMEBUFFER, finalQuadFBO);
	// Clear the buffer to default color
	glClearColor(0, 0, 0, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// Point lights
	{
		glUseProgram(lightVolumeShader);
		glBindVertexArray(lightVolumeVAO);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gBuffer.normals);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gBuffer.diffuseSpec);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, gBuffer.depth);
		glUniform1i(glGetUniformLocation(lightVolumeShader, "gNormal"), 0);
		glUniform1i(glGetUniformLocation(lightVolumeShader, "gDiffuseSpec"), 1);
		glUniform1i(glGetUniformLocation(lightVolumeShader, "gDepth"), 2);

		GLint uniCamPos = glGetUniformLocation(lightVolumeShader, "camPos");
		glUniform3f(uniCamPos, camPos.x, camPos.y, camPos.z);

		GLint uniProj = glGetUniformLocation(lightVolumeShader, "invProj");
		glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(glm::inverse(proj)));
		GLint uniView = glGetUniformLocation(lightVolumeShader, "invView");
		glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(glm::inverse(view)));

		GLint pvm = glGetUniformLocation(lightVolumeShader, "pvm");
		GLint lightPos = glGetUniformLocation(lightVolumeShader, "lightPos");
		GLint lightCol = glGetUniformLocation(lightVolumeShader, "lightCol");
		GLint lightLum = glGetUniformLocation(lightVolumeShader, "lightLum");

		// instead of drawing arrays, draw spheres at each light position
		// Tiled deferred rendering has a huge boost over deferred with light volumes in densely
		// populated lights (like my demo). However, it performs (slightly?) worse with sparsely
		// populated lights
		// Assuming I want to keep deferred/light-volumes due to performing better in more realistic
		// scenarios, how can I boost performance?

		// Right now we loop through every single pixel for every single light --> WASTE
		// We want to get that to looping through every pixel for every meaningful light
		// - BVH --> Would be similar to frustum planes. Would need to figure out bounds of object and which bvh sections it fits in
		// - 
		// - Tiled-deferred


		// Point Lights
		for (int i = 0; i < pointLightsToDraw.size(); i++) {

			glm::mat4 pvmMatrix = proj * view * pointLightsToDraw[i].model;

			// Switch culling if inside light volume
			// float cubeRadius = sqrt(pointLightsToDraw[i].radius * pointLightsToDraw[i].radius + pointLightsToDraw[i].radius * pointLightsToDraw[i].radius);
			if (glm::length(camPos - glm::vec3(pointLightsToDraw[i].position)) < pointLightsToDraw[i].radius) {
				glCullFace(GL_FRONT);
			} else {
				glCullFace(GL_BACK);
			}


			glUniformMatrix4fv(pvm, 1, GL_FALSE, glm::value_ptr(pvmMatrix));
			glUniform1f(lightLum, pointLightsToDraw[i].luminance);
			glUniform3f(lightPos, pointLightsToDraw[i].position.x, pointLightsToDraw[i].position.y, pointLightsToDraw[i].position.z);
			glUniform3f(lightCol, pointLightsToDraw[i].color.r, pointLightsToDraw[i].color.g, pointLightsToDraw[i].color.b);


			totalTriangles += static_cast<int>(lightVolume->indices.size()) / 3;

			// User our shader and draw our program
			glDrawElements(GL_TRIANGLES, static_cast<int>(lightVolume->indices.size()), GL_UNSIGNED_INT, 0); //Number of vertices
		}
	}

	// Switch back culling after point lights
	glCullFace(GL_BACK);

	// Directional Light (Sun)
	{
		glUseProgram(directionalLightShader);
		glBindVertexArray(quadVAO);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gBuffer.normals);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gBuffer.diffuseSpec);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, gBuffer.depth);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		glUniform1i(glGetUniformLocation(directionalLightShader, "gNormal"), 0);
		glUniform1i(glGetUniformLocation(directionalLightShader, "gDiffuseSpec"), 1);
		glUniform1i(glGetUniformLocation(directionalLightShader, "gDepth"), 2);
		glUniform1i(glGetUniformLocation(directionalLightShader, "depthMap"), 3);

		GLint uniLightMat = glGetUniformLocation(directionalLightShader, "lightProjView");
		glUniformMatrix4fv(uniLightMat, 1, GL_FALSE, glm::value_ptr(lightProjView));

		GLint uniProj = glGetUniformLocation(directionalLightShader, "invProj");
		glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(glm::inverse(proj)));
		GLint uniView = glGetUniformLocation(directionalLightShader, "invView");
		glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(glm::inverse(view)));

		GLint uniCamPos = glGetUniformLocation(directionalLightShader, "camPos");
		glUniform3f(uniCamPos, camPos.x, camPos.y, camPos.z);

		GLint lightDir = glGetUniformLocation(directionalLightShader, "lightDir");
		glUniform3f(lightDir, sun->direction.x, sun->direction.y, sun->direction.z);
		GLint lightCol = glGetUniformLocation(directionalLightShader, "lightCol");
		glUniform3f(lightCol, sun->color.r, sun->color.g, sun->color.b);

		glDrawArrays(GL_TRIANGLES, 0, 6); //Number of vertices
	}

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
}

void RendererSystem::DrawTransparent() {

	// Save all of our depth information from deferred pass for use in forward pass
	glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer.id);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, finalQuadFBO);
	glBlitFramebuffer(0, 0, screenWidth, screenHeight, 0, 0, screenWidth, screenHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

	glBindFramebuffer(GL_FRAMEBUFFER, finalQuadFBO);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glm::vec3 camPos = mainCamera->transform->position;

	// Not worth sorting transparent objects for sponza as each transparent model is not separate. 1 obj has transparent models in multiple locations
	// But will sort anyways to deal with other models
	std::sort(transparentToDraw.begin(), transparentToDraw.end(), [camPos](const MeshToDraw& a, const MeshToDraw& b) {
		return glm::length(a.position - camPos) > glm::length(b.position - camPos);
	});

	// all transparent objects use same shader
	if (transparentToDraw.size() > 0) {
		glUseProgram(transparentToDraw[0].shaderProgram);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, pointLightsSSBO);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(PointLightToGPU) * pointLightsToGPU.size(), pointLightsToGPU.data());

		GLint uniNumLights = glGetUniformLocation(transparentToDraw[0].shaderProgram, "numLights");
		glUniform1i(uniNumLights, static_cast<int>(pointLightsToGPU.size()));

		GLint uniCamPos = glGetUniformLocation(transparentToDraw[0].shaderProgram, "camPos");
		glUniform3f(uniCamPos, camPos.x, camPos.y, camPos.z);

		// Directional light
		GLint lightDir = glGetUniformLocation(transparentToDraw[0].shaderProgram, "lightDir");
		glUniform3f(lightDir, sun->direction.x, sun->direction.y, sun->direction.z);
		GLint lightCol = glGetUniformLocation(transparentToDraw[0].shaderProgram, "lightCol");
		glUniform3f(lightCol, sun->color.r, sun->color.g, sun->color.b);

		// Shadow map stuff
		glActiveTexture(GL_TEXTURE0 + 8);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		glUniform1i(glGetUniformLocation(transparentToDraw[0].shaderProgram, "depthMap"), 8);
		glUniformMatrix4fv(glGetUniformLocation(transparentToDraw[0].shaderProgram, "lightProjView"), 1, GL_FALSE, glm::value_ptr(lightProjView));
	}

	// For each of our transparent objects, set up its shader
	for (int i = 0; i < static_cast<int>(transparentToDraw.size()); i++) {

		glBindVertexArray(transparentToDraw[i].vao);

		Material* m = transparentToDraw[i].material;

		glUniformMatrix4fv(m->uniModel, 1, GL_FALSE, glm::value_ptr(transparentToDraw[i].model));
		glUniformMatrix4fv(m->uniView, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(m->uniProj, 1, GL_FALSE, glm::value_ptr(proj));

		glUniform3f(m->uniAmbient, m->ambient.r, m->ambient.g, m->ambient.b);
		glUniform3f(m->uniDiffuse, m->diffuse.r, m->diffuse.g, m->diffuse.b);
		glUniform3f(m->uniSpecular, m->specular.r, m->specular.g, m->specular.b);
		glUniform1f(m->uniSpecularExp, m->specularExponent);
		glUniform1f(m->uniOpacity, m->opacity);

		glUniform1i(m->uniUsingBump, m->usingBump);
		glUniform1i(m->uniUsingNormal, m->usingNormal);


		// How could I change the following IF statements to avoid last second state changes?
		// Could I call specific functions?

		glActiveTexture(GL_TEXTURE0 + 0);
		if (m->diffuseTexture != nullptr) {
			glBindTexture(GL_TEXTURE_2D, assetManager->diffuseTextures[m->diffuseIndex]);
		} else {
			glBindTexture(GL_TEXTURE_2D, assetManager->nullTexture);
		}
		glUniform1i(m->uniDiffuseTex, 0);

		glActiveTexture(GL_TEXTURE0 + 1);
		if (m->specularTexture != nullptr) {
			glBindTexture(GL_TEXTURE_2D, assetManager->specularTextures[m->specularIndex]);
		} else {
			glBindTexture(GL_TEXTURE_2D, assetManager->nullTexture);
		}
		glUniform1i(m->uniSpecularTex, 1);

		glActiveTexture(GL_TEXTURE0 + 2);
		if (m->specularHighLightTexture != nullptr) {
			glBindTexture(GL_TEXTURE_2D, assetManager->specularHighLightTextures[m->specularHighLightIndex]);
		} else {
			glBindTexture(GL_TEXTURE_2D, assetManager->nullTexture);
		}
		glUniform1i(m->uniSpecularHighLightTex, 2);

		glActiveTexture(GL_TEXTURE0 + 3);
		if (m->bumpTexture != nullptr) {
			glBindTexture(GL_TEXTURE_2D, assetManager->bumpTextures[m->bumpIndex]);
		} else {
			glBindTexture(GL_TEXTURE_2D, assetManager->nullTexture);
		}
		glUniform1i(m->uniBumpTex, 3);

		glActiveTexture(GL_TEXTURE0 + 4);
		if (m->normalTexture != nullptr) {
			glBindTexture(GL_TEXTURE_2D, assetManager->normalTextures[m->normalIndex]);
		} else {
			glBindTexture(GL_TEXTURE_2D, assetManager->nullTexture);
		}
		glUniform1i(m->uniNormalTex, 4);

		glActiveTexture(GL_TEXTURE0 + 5);
		if (m->displacementTexture != nullptr) {
			glBindTexture(GL_TEXTURE_2D, assetManager->displacementTextures[m->displacementIndex]);
		} else {
			glBindTexture(GL_TEXTURE_2D, assetManager->nullTexture);
		}
		glUniform1i(m->uniDisplacementTex, 5);

		glActiveTexture(GL_TEXTURE0 + 6);
		if (m->alphaTexture != nullptr) {
			glBindTexture(GL_TEXTURE_2D, assetManager->alphaTextures[m->alphaIndex]);
		} else {
			glBindTexture(GL_TEXTURE_2D, assetManager->nullTexture);
		}
		glUniform1i(m->uniAlphaTex, 6);


		totalTriangles += static_cast<int>(transparentToDraw[i].mesh->indices.size()) / 3;

		// Use our shader and draw our program
		glDrawElements(GL_TRIANGLES, static_cast<int>(transparentToDraw[i].mesh->indices.size()), GL_UNSIGNED_INT, 0); //Number of vertices
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDisable(GL_BLEND);

}

void RendererSystem::PostProcess() {

	glBindFramebuffer(GL_READ_FRAMEBUFFER, finalQuadFBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, 0, screenWidth, screenHeight, 0, 0, screenWidth, screenHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glUseProgram(finalQuadShader);
	glBindVertexArray(quadVAO);

	// Final quad
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, finalQuadRender);
	glUniform1i(glGetUniformLocation(finalQuadShader, "finalQuadRender"), 0);

	glUniform1f(glGetUniformLocation(finalQuadShader, "xSpan"), 1.0f / windowWidth);
	glUniform1f(glGetUniformLocation(finalQuadShader, "ySpan"), 1.0f / windowHeight);

	glDrawArrays(GL_TRIANGLES, 0, 6); //Number of vertices
}

bool RendererSystem::ShouldFrustumCull(const Mesh* mesh, const glm::mat4& model) const {

	// Not dereferencing and caching bounds moved my ModelRendererSystem::Render call from 10.11% 
	// of total time used to 8.66%. Insignificant change, could be randomness or difference in view
	glm::vec3 maxPoint = mesh->bounds->Max(model);
	glm::vec3 minPoint = mesh->bounds->Min(model);

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
