
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

	pointLightIndicesSSBOToGPU = std::vector<PointLightIndicesSSBO, MemoryAllocator<PointLightIndicesSSBO> >(GRID_SIZE * GRID_SIZE * GRID_SIZE);
	std::vector<LinearBVHNode>& nodes = bvh->GetLinearBVH();

	// Initialize our compute shaders and gpu data
	{
		bakeLightsComputeShader = util::initComputeShader("bakeLights.comp");
		rayTraceComputeShader = util::initComputeShader("rayTrace.comp");

		// Constant memory lights
		glGenBuffers(1, &pointLightsUBO);
		glBindBuffer(GL_UNIFORM_BUFFER, pointLightsUBO);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(PointLightToGPU) * pointLightsToGPU.size(), pointLightsToGPU.data(), GL_STATIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, pointLightsUBO);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		uboMemory += sizeof(PointLightToGPU) * pointLightsToGPU.size();

		// Material buffer
		glGenBuffers(1, &materialsUBO);
		glBindBuffer(GL_UNIFORM_BUFFER, materialsUBO);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(GPUMaterial) * AssetManager::gpuMaterials->size(), AssetManager::gpuMaterials->data(), GL_STATIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, 2, materialsUBO);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		uboMemory += sizeof(GPUMaterial) * AssetManager::gpuMaterials->size();

		// BVH 
		glGenBuffers(1, &bvhSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, bvhSSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(LinearBVHNode) * nodes.size(), nodes.data(), GL_STATIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, bvhSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		ssboMemory += sizeof(LinearBVHNode) * nodes.size();

		// Vertices buffer
		glGenBuffers(1, &verticesSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, verticesSSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GPUVertex) * AssetManager::gpuVertices->size(), AssetManager::gpuVertices->data(), GL_STATIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, verticesSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		ssboMemory += sizeof(GPUVertex) * AssetManager::gpuVertices->size();


		// Triangle index buffer
		glGenBuffers(1, &triangleSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, triangleSSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GPUTriangle) * AssetManager::gpuTriangles->size(), AssetManager::gpuTriangles->data(), GL_STATIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, triangleSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		ssboMemory += sizeof(GPUTriangle) * AssetManager::gpuTriangles->size();

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

	// Cull our lights in a 3D grid for faster look-up in our ray tracer
	{

		// Point light indices as an SSBO to be written to.
		glGenBuffers(1, &pointLightIndicesSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, pointLightIndicesSSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(PointLightIndicesSSBO) * GRID_SIZE* GRID_SIZE* GRID_SIZE, NULL, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, pointLightIndicesSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

#if PROFILING
		glBeginQuery(GL_TIME_ELAPSED, timeQuery);
		LightCull();
		glEndQuery(GL_TIME_ELAPSED);
		glGetQueryObjecti64v(timeQuery, GL_QUERY_RESULT, &bakeLightsTime);
#else
		LightCull();
#endif

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, pointLightIndicesSSBO);
		glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(PointLightIndicesSSBO) * GRID_SIZE * GRID_SIZE* GRID_SIZE, pointLightIndicesSSBOToGPU.data());
		glDeleteBuffers(1, &pointLightIndicesSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		for (uint32_t i = 0; i < pointLightIndicesSSBOToGPU.size(); ++i) {
			PointLightIndicesSSBO p = pointLightIndicesSSBOToGPU[i];
			PointLightIndicesUBO pu;
			glm::uvec4 vec;

			vec.x = (
				(p.indices[0] & 0xFF) << 24 |
				(p.indices[1] & 0xFF) << 16 |
				(p.indices[2] & 0xFF) << 8 |
				(p.indices[3] & 0xFF)
				);
			vec.y = (
				(p.indices[4] & 0xFF) << 24 |
				(p.indices[5] & 0xFF) << 16 |
				(p.indices[6] & 0xFF) << 8 |
				(p.indices[7] & 0xFF)
				);
			vec.z = (
				(p.indices[8] & 0xFF) << 24 |
				(p.indices[9] & 0xFF) << 16 |
				(p.indices[10] & 0xFF) << 8 |
				(p.indices[11] & 0xFF)
				);
			vec.w = (
				(p.indices[12] & 0xFF) << 24 |
				(p.indices[13] & 0xFF) << 16 |
				(p.indices[14] & 0xFF) << 8 |
				(p.numLights & 0xFF)
				);

			pu.indices_and_num_lights = vec;
			pointLightIndicesUBOToGPU.push_back(pu);
		}

		glGenBuffers(1, &pointLightIndicesUBO);
		glBindBuffer(GL_UNIFORM_BUFFER, pointLightIndicesUBO);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(PointLightIndicesUBO) * GRID_SIZE * GRID_SIZE * GRID_SIZE, pointLightIndicesUBOToGPU.data(), GL_STATIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, 1, pointLightIndicesUBO);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		uboMemory += sizeof(PointLightIndicesSSBO) * GRID_SIZE * GRID_SIZE * GRID_SIZE;

	}

	// Set up uniforms for our ray trace compute shader
	{
		glUseProgram(rayTraceComputeShader);

		glBindImageTexture(0, finalQuadRender, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
		uniDestTex = glGetUniformLocation(rayTraceComputeShader, "destTex");
		glUniform1i(uniDestTex, 0);

		uniCamPos = glGetUniformLocation(rayTraceComputeShader, "camPos");

		uniInvProj = glGetUniformLocation(rayTraceComputeShader, "invProj");
		uniInvView = glGetUniformLocation(rayTraceComputeShader, "invView");

		uniMinBounds = glGetUniformLocation(rayTraceComputeShader, "minBounds");
		uniMaxBounds = glGetUniformLocation(rayTraceComputeShader, "maxBounds");
		glm::vec3 minBounds = nodes[0].boundsMin;
		glm::vec3 maxBounds = nodes[0].boundsMax;
		glUniform3fv(uniMinBounds, 1, glm::value_ptr(minBounds));
		glUniform3fv(uniMaxBounds, 1, glm::value_ptr(maxBounds));

		uniDirectionalLightDir = glGetUniformLocation(rayTraceComputeShader, "directionalLightDir");
		uniDirectionalLightCol = glGetUniformLocation(rayTraceComputeShader, "directionalLightCol");

		glUseProgram(0);
	}

	// Memory prints
	{
		// Max of 65kb of constant/shared memory
		assert(uboMemory < 1024 * 64);

		fprintf(stderr, "\nBVH -- Num: %zu\n", nodes.size());
		fprintf(stderr, "BVH -- Bytes: %zu\n", sizeof(LinearBVHNode)* nodes.size());
		uint32_t totalTris = 0;
		for (uint32_t i = 0; i < nodes.size(); i += 1) {
			totalTris += (nodes[i].numPrimitives_and_axis >> 16);
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

	glUseProgram(bakeLightsComputeShader);

	uniNumPointLights = glGetUniformLocation(bakeLightsComputeShader, "numPointLights");
	glUniform1ui(uniNumPointLights, static_cast<unsigned int>(pointLightsToGPU.size()));

	uniMinBounds = glGetUniformLocation(bakeLightsComputeShader, "minBounds");
	uniMaxBounds = glGetUniformLocation(bakeLightsComputeShader, "maxBounds");

	std::vector<LinearBVHNode>& nodes = bvh->GetLinearBVH();
	glm::vec3 minBounds = nodes[0].boundsMin;
	glm::vec3 maxBounds = nodes[0].boundsMax;
	glUniform3fv(uniMinBounds, 1, glm::value_ptr(minBounds));
	glUniform3fv(uniMaxBounds, 1, glm::value_ptr(maxBounds));

	glDispatchCompute(GRID_SIZE, GRID_SIZE, GRID_SIZE);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	glUseProgram(0);
}

void RayTracingSystem::RayTrace() {

	glUseProgram(rayTraceComputeShader);

	//Set up other uniform variables
	glUniform3fv(uniCamPos, 1, glm::value_ptr(mainCamera->transform->position));
	glUniformMatrix4fv(uniInvProj, 1, GL_FALSE, glm::value_ptr(glm::inverse(proj)));
	glUniformMatrix4fv(uniInvView, 1, GL_FALSE, glm::value_ptr(glm::inverse(view)));
	glUniform3fv(uniDirectionalLightDir, 1, glm::value_ptr(glm::vec3(mainScene->directionalLights[0].direction)));
	glUniform3fv(uniDirectionalLightCol, 1, glm::value_ptr(glm::vec3(mainScene->directionalLights[0].color)));

	glDispatchCompute(NUM_GROUPS_X, NUM_GROUPS_Y, 1);
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
