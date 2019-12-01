
#include "RendererSystem.h"
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

RendererSystem::RendererSystem(const int& sW, const int& sH) {
    screenWidth = sW;
    screenHeight = sH;

    modelRenderers = std::vector<ModelRenderer*>();
	meshesToDraw = std::vector<MeshToDraw>();
	transparentToDraw = std::vector<MeshToDraw>();

    pointLights = std::vector<PointLight*>();
	pointLightsToDraw = std::vector<PointLightToDraw>();

	// We know that this is a mesh, not a model
    lightVolume = (assetManager->tinyLoadObj("cube"))->meshes[0];
}

RendererSystem::~RendererSystem() {
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

	for (int i = 0; i < pointLights.size(); i++) {
		pointLights[i] = nullptr;
	}
	pointLights.clear();

	meshesToDraw.clear();
	transparentToDraw.clear();
}

void RendererSystem::Setup() {
    for (int i = 0; i < mainScene->lights.size(); i++) {
        PointLight* p = (PointLight*)mainScene->lights[i];
        pointLights.push_back(p);
    }

    // Get our list of related components, in this case MeshRenderers
    for (int i = 0; i < mainScene->instances.size(); i++) {
        ModelRenderer* mr = (ModelRenderer*)mainScene->instances[i]->GetComponent("modelRenderer");
        if (mr) {
			modelRenderers.push_back(mr);
            Register(mr);
        }
    }

	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(util::DebugMessageCallback, 0);

    glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glBlendFunc(GL_ONE, GL_ONE);

    // Set up our shader for rendering to the texture
    combinedShader = util::initShaderFromFiles("deferredLightVolumes.vert", "deferredLightVolumes.frag");

    glGenVertexArrays(1, &lightVolume_Vao);
    glBindVertexArray(lightVolume_Vao);

    glGenBuffers(1, &lightVolume_Vbo);
    glBindBuffer(GL_ARRAY_BUFFER, lightVolume_Vbo);
    glBufferData(GL_ARRAY_BUFFER, lightVolume->positions.size() * 3 * sizeof(float), &(lightVolume->positions[0]), GL_STATIC_DRAW);
    glEnableVertexAttribArray(glGetAttribLocation(combinedShader, "inPos"));
    glVertexAttribPointer(glGetAttribLocation(combinedShader, "inPos"), 3, GL_FLOAT, GL_FALSE, 0, 0);

    // index buffer for sphere
    glGenBuffers(1, &lightVolume_Ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lightVolume_Ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, lightVolume->indices.size() * sizeof(GL_UNSIGNED_INT), &(lightVolume->indices[0]), GL_STATIC_DRAW);
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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gBuffer.positions, 0);

    // - normal color buffer
    glGenTextures(1, &(gBuffer.normals));
    glBindTexture(GL_TEXTURE_2D, gBuffer.normals);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, screenWidth, screenHeight, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gBuffer.normals, 0);

    // diffuse color
    glGenTextures(1, &(gBuffer.diffuse));
    glBindTexture(GL_TEXTURE_2D, gBuffer.diffuse);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, screenWidth, screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gBuffer.diffuse, 0);

    // specular color
    glGenTextures(1, &(gBuffer.specular));
    glBindTexture(GL_TEXTURE_2D, gBuffer.specular);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, screenWidth, screenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
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
			mat->shader = assetManager->shaders[0];
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

		// Textures
		// Load up either a null texture or the wanted one
		glUseProgram(mr->model->materials[i]->shader->shaderProgram);

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

bool RendererSystem::ShouldFrustumCull(const Mesh* mesh, const glm::mat4& model, const glm::mat4& projViewMat) const {

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

void RendererSystem::Render() {

	// C++ here is actually incredibly small proportion of actual code runtime
	// Only 8-9% of actual bottleneck. Almost everything is in SwapBuffers with queued OpenGL commands
	// Which means, the place to optimize is still in this function and it is how I am queueing up OpenGL
	// commands or the amount of them and amount of context switches

	totalTriangles = 0;

    // Bind our deferred texture buffer
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer.id);

    // Clear the screen to default color
    glClearColor(0, 0, 0, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 view = mainCamera->view;
    glm::mat4 proj = mainCamera->proj;
	glm::mat4 projView = proj * view;



	// Get all of our meshes that are not frustum culled. These will be used for later drawing
	meshesToDraw.clear();
	transparentToDraw.clear();
	for (int i = 0; i < modelRenderers.size(); i++) {
		glm::mat4 model = modelRenderers[i]->gameObject->transform->model;
		for (int j = 0; j < modelRenderers[i]->numMeshes; j++) {

			if (!ShouldFrustumCull(modelRenderers[i]->model->meshes[j], model, projView)) {
				MeshToDraw m = MeshToDraw{
					m.mesh = modelRenderers[i]->model->meshes[j],
					m.material = modelRenderers[i]->model->materials[j],
					m.model = model,
					m.vao = modelRenderers[i]->vaos[j],
					m.indexVbo = modelRenderers[i]->vbos[j][3],
					m.shaderProgram = modelRenderers[i]->model->materials[j]->shader->shaderProgram					
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
	for (int i = 0; i < pointLights.size(); i++) {
		glm::vec4 pos = pointLights[i]->position;
		glm::vec4 color = pointLights[i]->color;

		float lum = .6f * color.g + .3f * color.r + .1f * color.b;
		float radius = 14;

		glm::mat4 model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(pos.x, pos.y, pos.z));
		model = glm::scale(model, lum * glm::vec3(radius, radius, radius));

		if (!ShouldFrustumCull(lightVolume, model, projView)) {

			PointLightToDraw p = PointLightToDraw{
				p.luminance = lum,
				p.radius = radius,
				p.position = pos,
				p.color = color,
				p.model = model
			};
			pointLightsToDraw.push_back(p);
		}
	}


	// First deferred rendering pass with all of our non-transparent model renderers
	DrawModels(proj, view, projView);

	// Next, draw our transparent items in a forward rendering pass
	DrawTransparency(proj, view, projView);
}

void RendererSystem::DrawModels(const glm::mat4& proj, const glm::mat4& view, const glm::mat4& projView) {

	// Draw all of our wanted meshRendereres
	for (int i = 0; i < meshesToDraw.size(); i++) {

		glUseProgram(meshesToDraw[i].shaderProgram);
		glBindVertexArray(meshesToDraw[i].vao);

		Material* m = meshesToDraw[i].material;

		glUniformMatrix4fv(m->uniModel, 1, GL_FALSE, glm::value_ptr(meshesToDraw[i].model));
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

		if (m->useTextures) {
			// Textures and booleans
			glActiveTexture(GL_TEXTURE0);
			if (m->ambientTexture != nullptr) {
				glBindTexture(GL_TEXTURE_2D, assetManager->ambientTextures[m->ambientIndex]);
			} else {
				glBindTexture(GL_TEXTURE_2D, assetManager->nullTexture);
			}
			glUniform1i(m->uniAmbientTex, 0);

			glActiveTexture(GL_TEXTURE0 + 1);
			if (m->diffuseTexture != nullptr) {
				glBindTexture(GL_TEXTURE_2D, assetManager->diffuseTextures[m->diffuseIndex]);
			} else {
				glBindTexture(GL_TEXTURE_2D, assetManager->nullTexture);
			}
			glUniform1i(m->uniDiffuseTex, 1);

			glActiveTexture(GL_TEXTURE0 + 2);
			if (m->specularTexture != nullptr) {
				glBindTexture(GL_TEXTURE_2D, assetManager->specularTextures[m->specularIndex]);
			} else {
				glBindTexture(GL_TEXTURE_2D, assetManager->nullTexture);
			}
			glUniform1i(m->uniSpecularTex, 2);

			glActiveTexture(GL_TEXTURE0 + 3);
			if (m->specularHighLightTexture != nullptr) {
				glBindTexture(GL_TEXTURE_2D, assetManager->specularHighLightTextures[m->specularHighLightIndex]);
			} else {
				glBindTexture(GL_TEXTURE_2D, assetManager->nullTexture);
			}
			glUniform1i(m->uniSpecularHighLightTex, 3);

			glActiveTexture(GL_TEXTURE0 + 4);
			if (m->bumpTexture != nullptr) {
				glBindTexture(GL_TEXTURE_2D, assetManager->bumpTextures[m->bumpIndex]);
			} else {
				glBindTexture(GL_TEXTURE_2D, assetManager->nullTexture);
			}
			glUniform1i(m->uniBumpTex, 4);

			glActiveTexture(GL_TEXTURE0 + 5);
			if (m->normalTexture != nullptr) {
				glBindTexture(GL_TEXTURE_2D, assetManager->normalTextures[m->normalIndex]);
			} else {
				glBindTexture(GL_TEXTURE_2D, assetManager->nullTexture);
			}
			glUniform1i(m->uniNormalTex, 5);

			glActiveTexture(GL_TEXTURE0 + 6);
			if (m->displacementTexture != nullptr) {
				glBindTexture(GL_TEXTURE_2D, assetManager->displacementTextures[m->displacementIndex]);
			} else {
				glBindTexture(GL_TEXTURE_2D, assetManager->nullTexture);
			}
			glUniform1i(m->uniDisplacementTex, 6);

			glActiveTexture(GL_TEXTURE0 + 7);
			if (m->alphaTexture != nullptr) {
				glBindTexture(GL_TEXTURE_2D, assetManager->alphaTextures[m->alphaIndex]);
			} else {
				glBindTexture(GL_TEXTURE_2D, assetManager->nullTexture);
			}
			glUniform1i(m->uniAlphaTex, 7);
		}

		// Indices
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshesToDraw[i].indexVbo);

		totalTriangles += static_cast<int>(meshesToDraw[i].mesh->indices.size()) / 3;

		// Use our shader and draw our program
		glDrawElements(GL_TRIANGLES, static_cast<int>(meshesToDraw[i].mesh->indices.size()), GL_UNSIGNED_INT, 0); //Number of vertices
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, screenWidth, screenHeight);

	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer.id);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, 0, screenWidth, screenHeight, 0, 0, screenWidth, screenHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

	glUseProgram(combinedShader);
	glBindVertexArray(lightVolume_Vao);

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
	GLint uniCamPos = glGetUniformLocation(combinedShader, "camPos");
	glUniform3f(uniCamPos, camPos.x, camPos.y, camPos.z);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lightVolume_Ibo);

	GLint pvm = glGetUniformLocation(combinedShader, "pvm");
	GLint lightPos = glGetUniformLocation(combinedShader, "lightPos");
	GLint lightCol = glGetUniformLocation(combinedShader, "lightCol");

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
	for (int i = 0; i < pointLightsToDraw.size(); i++) {

		glm::mat4 pvmMatrix = projView * pointLightsToDraw[i].model;

		// Switch culling if inside light volume
		float diagRad = sqrt(pointLightsToDraw[i].luminance * pointLightsToDraw[i].radius * pointLightsToDraw[i].radius + 
								pointLightsToDraw[i].luminance * pointLightsToDraw[i].radius * pointLightsToDraw[i].radius);
		if (glm::length(camPos - glm::vec3(pointLightsToDraw[i].position)) < diagRad) {
			glCullFace(GL_FRONT);
		} else {
			glCullFace(GL_BACK);
		}


		glUniformMatrix4fv(pvm, 1, GL_FALSE, glm::value_ptr(pvmMatrix));
		glUniform4f(lightPos, pointLightsToDraw[i].position.x, pointLightsToDraw[i].position.y, pointLightsToDraw[i].position.z, pointLightsToDraw[i].position.w);
		glUniform4f(lightCol, pointLightsToDraw[i].color.r, pointLightsToDraw[i].color.g, pointLightsToDraw[i].color.b, pointLightsToDraw[i].color.a);

		totalTriangles += static_cast<int>(lightVolume->indices.size()) / 3;

		// User our shader and draw our program
		glDrawElements(GL_TRIANGLES, static_cast<int>(lightVolume->indices.size()), GL_UNSIGNED_INT, 0); //Number of vertices
	}

	glCullFace(GL_BACK);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}

void RendererSystem::DrawTransparency(const glm::mat4& proj, const glm::mat4& view, const glm::mat4& projView) {

}

void RendererSystem::DrawShadows() {}

void RendererSystem::PostProcess() {}