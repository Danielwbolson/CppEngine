
#include "MeshRendererSystem.h"
#include "Utility.h"
#include "Parse.h"
#include "Configuration.h"

#include <algorithm>

#include "glm/gtc/type_ptr.hpp"

void checkGLError(const std::string& s) { 
    GLenum err;
    if ((err = glGetError()) != 0) {
        std::cout << s + " :" << err << std::endl;
    }
}

MeshRendererSystem::MeshRendererSystem(const int& sW, const int& sH) {
    screenWidth = sW;
    screenHeight = sH;
    numLights = 1000;

    gameObjects = std::vector<GameObject*>();
    components = std::vector<Component*>();
    pointLights = std::vector<PointLight>();

    ObjParse(lightSphere, "sphere.obj");
}

MeshRendererSystem::~MeshRendererSystem() {
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

	for (int i = 0; i < meshRenderers.size(); i++) {
		delete meshRenderers[i];
	}
	for (int i = 0; i < lights.size(); i++) {
		delete lights[i];
	}
}

void MeshRendererSystem::Setup(const std::vector<GameObject*>& g, const std::vector<Light*>& l) {
    // Get our list of gameObjects
    gameObjects = g;
    lights = l;
    for (int i = 0; i < lights.size(); i++) {
        PointLight p = *((PointLight*)lights[i]);
        pointLights.push_back(p);

        lightPositions.push_back(p.position);
    }

    // Get our list of related components, in this case MeshRenderers
    for (int i = 0; i < gameObjects.size(); i++) {
        MeshRenderer* mr = (MeshRenderer*)gameObjects[i]->GetComponent("meshRenderer");
        if (mr) {
            components.push_back(mr);
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
    glBufferData(GL_ARRAY_BUFFER, lightSphere.NumPositions() * 3 * sizeof(float), &(lightSphere.pos[0]), GL_STATIC_DRAW);
    glEnableVertexAttribArray(glGetAttribLocation(combinedShader, "inPos"));
    glVertexAttribPointer(glGetAttribLocation(combinedShader, "inPos"), 3, GL_FLOAT, GL_FALSE, 0, 0);

    // index buffer for sphere
    glGenBuffers(1, &lightVolume_Ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lightVolume_Ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, lightSphere.NumIndices() * sizeof(GL_UNSIGNED_INT), &(lightSphere.indices[0]), GL_STATIC_DRAW);
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, screenWidth, screenHeight, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gBuffer.positions, 0);

    // - normal color buffer
    glGenTextures(1, &(gBuffer.normals));
    glBindTexture(GL_TEXTURE_2D, gBuffer.normals);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, screenWidth, screenHeight, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gBuffer.normals, 0);

    // diffuse color
    glGenTextures(1, &(gBuffer.diffuse));
    glBindTexture(GL_TEXTURE_2D, gBuffer.diffuse);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screenWidth, screenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gBuffer.diffuse, 0);

    // specular color
    glGenTextures(1, &(gBuffer.specular));
    glBindTexture(GL_TEXTURE_2D, gBuffer.specular);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screenWidth, screenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
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

    glEnable(GL_DEPTH_TEST);
}

void MeshRendererSystem::ComponentType(const std::string&) const {

}

void MeshRendererSystem::Register(const Component* c) {
    // Quick reference
    MeshRenderer* mr = (MeshRenderer*)c;
    meshRenderers.push_back(mr);

    glGenVertexArrays(1, &(mr->vao));
    glBindVertexArray(mr->vao);

    // Position
    glGenBuffers(1, &(mr->vbo[0]));
    glBindBuffer(GL_ARRAY_BUFFER, mr->vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, 3 * mr->mesh->NumPositions() * sizeof(float), &(mr->mesh->pos[0]), GL_STATIC_DRAW);

    GLint posAttrib = glGetAttribLocation(mr->material->shaderProgram, "inPosition");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
    //Attribute, vals/attrib., type, isNormalized, stride, offset

    // Normals
    glGenBuffers(1, &(mr->vbo[1]));
    glBindBuffer(GL_ARRAY_BUFFER, mr->vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, 3 * mr->mesh->NumNorms() * sizeof(float), &(mr->mesh->normals[0]), GL_STATIC_DRAW);

    GLint normAttrib = glGetAttribLocation(mr->material->shaderProgram, "inNormal");
    glEnableVertexAttribArray(normAttrib);
    glVertexAttribPointer(normAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);

    //// UVS
    //glGenBuffers(1, &(mr->vbo[2]));
    //glBindBuffer(GL_ARRAY_BUFFER, mr->vbo[2]);
    //glBufferData(GL_ARRAY_BUFFER, 2 * mr->mesh.NumPositions() * sizeof(float), &(mr->mesh.uvs[0]), GL_STATIC_DRAW);
    //if ((err = glGetError()) != GL_NO_ERROR) {
    //    std::cerr << "inbetween OpenGL error: " << err << std::endl;
    //}

    //GLint uvAttrib = glGetAttribLocation(mr->material.shaderProgram, "inTexcoord");
    //glEnableVertexAttribArray(uvAttrib);
    //glVertexAttribPointer(uvAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);

    // Indices
    glGenBuffers(1, &(mr->vbo[3]));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mr->vbo[3]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mr->mesh->NumIndices() * sizeof(GL_UNSIGNED_INT), &(mr->mesh->indices[0]), GL_STATIC_DRAW);

    glBindVertexArray(0);
}

void MeshRendererSystem::Render() {
	totalTriangles = 0;

    // Bind our deferred texture buffer
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer.id);

    // Clear the screen to default color
    glClearColor(0, 0, 0, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 view = mainCamera->view;
    glm::mat4 proj = mainCamera->proj;

    // Get all of our meshRenderers to draw to textures
    for (int i = 0; i < meshRenderers.size(); i++) {

		glm::mat4 model = meshRenderers[i]->gameObject->transform->model;

		// Pre-emptively frustum cull unnecessary meshes
		if (FrustumCull(meshRenderers[i]->mesh, model, proj * view)) { continue; }


        glUseProgram(meshRenderers[i]->material->shaderProgram);
        glBindVertexArray(meshRenderers[i]->vao);

        GLint uniColor = glGetUniformLocation(meshRenderers[i]->material->shaderProgram, "inColor");
        glUniform3f(uniColor, meshRenderers[i]->material->Color().x, meshRenderers[i]->material->Color().y, meshRenderers[i]->material->Color().z);
        GLint uniModel = glGetUniformLocation(meshRenderers[i]->material->shaderProgram, "model");
        glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
        GLint uniView = glGetUniformLocation(meshRenderers[i]->material->shaderProgram, "view");
        glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));
        GLint uniProj = glGetUniformLocation(meshRenderers[i]->material->shaderProgram, "proj");
        glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));

        // Indices
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshRenderers[i]->vbo[3]);

		totalTriangles += meshRenderers[i]->mesh->NumIndices() / 3;

        // Use our shader and draw our program
        glDrawElements(GL_TRIANGLES, meshRenderers[i]->mesh->NumIndices(), GL_UNSIGNED_INT, 0); //Number of vertices
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, screenWidth, screenHeight);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

	glDepthMask(GL_FALSE);
	glDisable(GL_DEPTH_TEST);

    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    // Indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lightVolume_Ibo);

	glm::vec3 camPos = mainCamera->transform->position;

    // instead of drawing arrays, draw spheres at each light position
    for (int i = 0; i < numLights; i++) {
        glm::vec4 pos = pointLights[i].position;
        glm::vec4 color = pointLights[i].color;

        float lum = .6f * color.g + .3f * color.r + .1f * color.b;
        float radScal = sqrt(lum * 10);
		float radius = lum * radScal;

        glm::mat4 model;
        model = glm::translate(model, glm::vec3(pos.x, pos.y, pos.z));
        model = glm::scale(model, lum * glm::vec3(radScal, radScal, radScal));


		// Pre-emptively frustum cull unnecessary meshes
		if (FrustumCull(&lightSphere, model, proj * view)) { continue; }


		glm::mat4 pvmMatrix = proj * view * model;

		// Swith culling if inside light volume
		if (glm::length(camPos - glm::vec3(pos.x, pos.y, pos.z)) < radius) {
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

		totalTriangles += lightSphere.NumIndices() / 3;

        // User our shader and draw our program
        glDrawElements(GL_TRIANGLES, lightSphere.NumIndices(), GL_UNSIGNED_INT, 0); //Number of vertices
    }

	glCullFace(GL_BACK);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
}

bool MeshRendererSystem::FrustumCull(const Mesh* mesh, const glm::mat4& model, const glm::mat4& projViewMat) const {
	
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
		float val = std::max(minPoint.x * planes[j].x, maxPoint.x * planes[j].x)
			+ std::max(minPoint.y * planes[j].y, maxPoint.y * planes[j].y)
			+ std::max(minPoint.z * planes[j].z, maxPoint.z * planes[j].z)
			+ planes[j].w;
		success += (val > 0);
	}

	if (success == 6) { 
		return false;
	}

	return true;
}