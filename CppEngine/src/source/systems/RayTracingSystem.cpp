
#include "RayTracingSystem.h"

#include "Configuration.h"
#include "Utility.h"
#include "glm/gtc/type_ptr.hpp"

#include "ModelRenderer.h"
#include "Scene.h"
#include "BVH.h"
#include "Camera.h"

#include <cassert>

#include "BVHTypes.h"

GLfloat boundingBoxVertices[] = {
  -0.5, -0.5, -0.5, 1.0,
   0.5, -0.5, -0.5, 1.0,
   0.5,  0.5, -0.5, 1.0,
  -0.5,  0.5, -0.5, 1.0,
  -0.5, -0.5,  0.5, 1.0,
   0.5, -0.5,  0.5, 1.0,
   0.5,  0.5,  0.5, 1.0,
  -0.5,  0.5,  0.5, 1.0
};
GLushort boundingBoxElements[] = {
  0, 1, 2, 3,
  4, 5, 6, 7,
  0, 4, 1, 5, 2, 6, 3, 7
};

RayTracingSystem::RayTracingSystem() {}

RayTracingSystem::~RayTracingSystem() {
	glDeleteProgram(rayTraceComputeShader);
	glDeleteProgram(finalQuadShader);

	glDeleteFramebuffers(1, &finalQuadFBO);
}

void RayTracingSystem::Setup() {

	// Set up debugging support
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(util::DebugMessageCallback, 0);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_FALSE);
	glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_ERROR, GL_DONT_CARE, 0, NULL, GL_TRUE);

	glGenQueries(1, &timeQuery);

	// Set up our vectors of gpu lights
	pointLightsToGPU.reserve(mainScene->pointLights.size());
	for (int i = 0; i < mainScene->pointLights.size(); i++) {
		PointLightToGPU pToGPU = PointLightToGPU{
			pToGPU.position_and_radius = glm::vec4(glm::vec3(mainScene->pointLights[i].position), mainScene->pointLights[i].radius),
			pToGPU.color_and_luminance = glm::vec4(mainScene->pointLights[i].color, mainScene->pointLights[i].lum)
		};
		pointLightsToGPU.push_back(pToGPU);
	}

	directionalLightsToGPU.reserve(mainScene->directionalLights.size());
	for (int i = 0; i < mainScene->directionalLights.size(); i++) {
		DirectionalLightToGPU d;
		d.direction = mainScene->directionalLights[i].direction;

		glm::vec3 color = mainScene->directionalLights[i].color;
		d.color_and_luminance =
			glm::vec4(
				color,
				mainScene->directionalLights[i].lum
			);
		directionalLightsToGPU.push_back(d);
	}

	// Initialize our bounding box debug shader and data
	{
		boundingBoxShader = util::initVertFragShader("simpleDraw.vert", "simpleDraw.frag");

		glGenVertexArrays(1, &bbVAO);
		glBindVertexArray(bbVAO);

		glGenBuffers(1, &bbVertices);
		glBindBuffer(GL_ARRAY_BUFFER, bbVertices);
		glBufferData(GL_ARRAY_BUFFER, sizeof(boundingBoxVertices), boundingBoxVertices, GL_STATIC_DRAW);

		GLint posAttrib = glGetAttribLocation(boundingBoxShader, "inPos");
		glEnableVertexAttribArray(posAttrib);
		glVertexAttribPointer(
			posAttrib,  // attribute
			4,                  // number of elements per vertex, here (x,y,z,w)
			GL_FLOAT,           // the type of each element
			GL_FALSE,           // take our values as-is
			0,                  // no extra data between each position
			0                   // offset of first element
		);

		glGenBuffers(1, &bbIndices);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bbIndices);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(boundingBoxElements), boundingBoxElements, GL_STATIC_DRAW);

		glBindVertexArray(0);
	}

	// Initialize our compute shaders and gpu data
	{
		//bakeLightsComputeShader = util::initComputeShader("bakeLights.comp");
		rayTraceComputeShader = util::initComputeShader("rayTrace.comp");
		unsigned long long uboMemory = 0;
		unsigned long long ssboMemory = 0;

		// Constant memory lights
		// Max of 65k bytes
		assert(sizeof(PointLightToGPU) * pointLightsToGPU.size() <= 1024 * 64);
		glGenBuffers(1, &pointLightsUBO);
		glBindBuffer(GL_UNIFORM_BUFFER, pointLightsUBO);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(PointLightToGPU) * pointLightsToGPU.size(), pointLightsToGPU.data(), GL_STATIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, pointLightsUBO);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		uboMemory += sizeof(PointLightToGPU) * pointLightsToGPU.size();

		// Material buffer
		//glGenBuffers(1, &materialsSSBO);
		//glBindBuffer(GL_UNIFORM_BUFFER, materialsSSBO);
		//glBufferData(GL_UNIFORM_BUFFER, sizeof(GPUMaterial) * AssetManager::gpuMaterials->size(), AssetManager::gpuMaterials->data(), GL_STATIC_DRAW);
		//glBindBufferBase(GL_UNIFORM_BUFFER, 1, materialsSSBO);
		//glBindBuffer(GL_UNIFORM_BUFFER, 0);
		//uboMemory += sizeof(GPUMaterial) * AssetManager::gpuMaterials->size();

		// BVH 
		std::vector<LinearBVHNode>& nodes = bvh->GetLinearBVH();
		glGenBuffers(1, &bvhSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, bvhSSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(LinearBVHNode) * nodes.size(), nodes.data(), GL_STATIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, bvhSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		ssboMemory += sizeof(LinearBVHNode) * nodes.size();


		// Vertices buffer
		glGenBuffers(1, &verticesSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, verticesSSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GPUVertex) * AssetManager::gpuVertices->size(), AssetManager::gpuVertices->data(), GL_STATIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, verticesSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		ssboMemory += sizeof(GPUVertex) * AssetManager::gpuVertices->size();


		// Triangle index buffer
		glGenBuffers(1, &triangleSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, triangleSSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GPUTriangle) * AssetManager::gpuTriangles->size(), AssetManager::gpuTriangles->data(), GL_STATIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, triangleSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		ssboMemory += sizeof(GPUTriangle) * AssetManager::gpuTriangles->size();


		fprintf(stderr, "\nBVH -- Num: %zu\n", nodes.size());
		fprintf(stderr, "BVH -- Bytes: %zu\n", sizeof(LinearBVHNode)* nodes.size());
		uint32_t totalTris = 0;
		for (uint32_t i = 0; i < nodes.size(); i += 1) {
			totalTris += nodes[i].numPrimitives;
		}
		fprintf(stderr, "BVH -- Contained Tris: %u\n", totalTris);

		fprintf(stderr, "\nVertices -- Num: %zu\n", AssetManager::gpuVertices->size());
		fprintf(stderr, "Vertices -- Bytes: %zu\n", sizeof(GPUVertex) * AssetManager::gpuVertices->size());

		fprintf(stderr, "\nTriangles -- Num: %zu\n", AssetManager::gpuTriangles->size());
		fprintf(stderr, "Triangles -- Bytes: %zu\n", sizeof(GPUTriangle) * AssetManager::gpuTriangles->size());

		fprintf(stderr, "\nUBO -- Constant/Shared Memory (b): %llu\n", uboMemory);
		fprintf(stderr, "UBO -- Constant/Shared Memory (kb): %llu\n", uboMemory / 1024);
		fprintf(stderr, "UBO -- Constant/Shared Memory (mb): %llu\n", uboMemory / 1024 / 1024);

		fprintf(stderr, "\nSSBO -- Global Memory (b): %llu\n", ssboMemory);
		fprintf(stderr, "SSBO -- Global Memory (kb): %llu\n", ssboMemory / 1024);
		fprintf(stderr, "SSBO -- Global Memory (mb): %llu\n", ssboMemory / 1024 / 1024);

	}
	
	// Create our final framebuffer and accompanying geometry and shaders for our final render
	{
		finalQuadShader = util::initVertFragShader("finalQuad.vert", "finalQuad.frag");

		glGenFramebuffers(1, &finalQuadFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, finalQuadFBO);

		glGenTextures(1, &finalQuadRender);
		glBindTexture(GL_TEXTURE_2D, finalQuadRender);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float blackBorderColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, blackBorderColor);

		glGenTextures(1, &finalQuadDepth);
		glBindTexture(GL_TEXTURE_2D, finalQuadDepth);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, windowWidth, windowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float whiteBorderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, whiteBorderColor);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, finalQuadRender, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, finalQuadDepth, 0);

		glGenVertexArrays(1, &quadVAO);
		glBindVertexArray(quadVAO);

		glGenBuffers(1, &quadVBO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), quadVerts, GL_STATIC_DRAW);

		GLint posAttrib = glGetAttribLocation(finalQuadShader, "inPos");
		glEnableVertexAttribArray(posAttrib);
		glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);

		glBindVertexArray(0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	// Set up uniforms
	{
		glUseProgram(rayTraceComputeShader);

		glBindImageTexture(0, finalQuadRender, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
		uniDestTex = glGetUniformLocation(rayTraceComputeShader, "destTex");
		glUniform1i(uniDestTex, 0);

		uniCamPos = glGetUniformLocation(rayTraceComputeShader, "camPos");

		uniProj = glGetUniformLocation(rayTraceComputeShader, "proj");
		uniView = glGetUniformLocation(rayTraceComputeShader, "view");

		uniInvProj = glGetUniformLocation(rayTraceComputeShader, "invProj");
		uniInvView = glGetUniformLocation(rayTraceComputeShader, "invView");

		uniNumPointLights = glGetUniformLocation(rayTraceComputeShader, "numPointLights");
		glUniform1ui(uniNumPointLights, static_cast<unsigned int>(pointLightsToGPU.size()));

		glUseProgram(0);
	}

}

void RayTracingSystem::Render() {

	view = mainCamera->view;
	proj = mainCamera->proj;

	// Clear our framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(0, 0, 0, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindFramebuffer(GL_FRAMEBUFFER, finalQuadFBO);
	glClearColor(0, 0, 0, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Cull our lights in a 3D grid for faster look-up in our ray tracer
#if PROFILING
	glBeginQuery(GL_TIME_ELAPSED, timeQuery);
	//LightCull();
	glEndQuery(GL_TIME_ELAPSED);
	glGetQueryObjecti64v(timeQuery, GL_QUERY_RESULT, &bakeLightsTime);
#else
	LightCull();
#endif

	// Debug draw our bounding boxes.
#if PROFILING

	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//glDrawBuffer(GL_FRONT_AND_BACK);
	//glClearColor(0, 0, 0, 1.0f);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//glUseProgram(boundingBoxShader);
	//glUniformMatrix4fv(glGetUniformLocation(boundingBoxShader, "projView"), 1, GL_FALSE, glm::value_ptr(proj * view));

	//glBindVertexArray(bbVAO);
	//glLineWidth(5);

	//std::vector<LinearBVHNode> nodes = bvh->GetLinearBVH();
	//for (int32_t i = 0; i < 5000;/*nodes.size();*/ i += 1)
	//{
	//	glm::vec3 size = nodes[i].boundsMax - nodes[i].boundsMin;
	//	glm::vec3 center = (nodes[i].boundsMax + nodes[i].boundsMax) / 2.0f;
	//	glm::mat4 transform = glm::translate(glm::mat4(1), center) * glm::scale(glm::mat4(1), size);

	//	glUniformMatrix4fv(glGetUniformLocation(boundingBoxShader, "model"), 1, GL_FALSE, glm::value_ptr(transform));

	//	glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_SHORT, 0);
	//	glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_SHORT, (GLvoid*)(4 * sizeof(GLushort)));
	//	glDrawElements(GL_LINES, 8, GL_UNSIGNED_SHORT, (GLvoid*)(8 * sizeof(GLushort)));
	//}

	//glBindVertexArray(0);

#else
	// Empty
#endif

	// Next, ray trace our scene
#if PROFILING
	glBeginQuery(GL_TIME_ELAPSED, timeQuery);
	RayTrace();
	glEndQuery(GL_TIME_ELAPSED);
	glGetQueryObjecti64v(timeQuery, GL_QUERY_RESULT, &rayTraceTime);
#else
	RayTrace();
#endif

	// Finally, do post processing effects on final image.
#if PROFILING
	glBeginQuery(GL_TIME_ELAPSED, timeQuery);
	PostProcess();
	glEndQuery(GL_TIME_ELAPSED);
	glGetQueryObjecti64v(timeQuery, GL_QUERY_RESULT, &postFXTime);
#else
	PostProcess();
#endif
}

void RayTracingSystem::LightCull() {

}

void RayTracingSystem::RayTrace() {

	glUseProgram(rayTraceComputeShader);

	//Set up other uniform variables
	glm::vec3 camPos = mainCamera->transform->position;
	glUniform3f(uniCamPos, camPos.x, camPos.y, camPos.z);

	glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));
	glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(uniInvProj, 1, GL_FALSE, glm::value_ptr(glm::inverse(proj)));
	glUniformMatrix4fv(uniInvView, 1, GL_FALSE, glm::value_ptr(glm::inverse(view)));

	glDispatchCompute(NUM_GROUPS_X, NUM_GROUPS_Y, 1);

	glBindVertexArray(0);
}

void RayTracingSystem::PostProcess() {

	glBindFramebuffer(GL_READ_FRAMEBUFFER, finalQuadFBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
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

	glBindVertexArray(0);
}
