
#ifndef BVH_H_
#define BVH_H_

#include "BVHTypes.h"
#include "RenderTypes.h"

#include <vector>
#include "MemoryAllocator.h"

class Model;

// This is a BVH class based off of the BVH chapter in the PBRT textbook
// http://www.pbr-book.org/3ed-2018/Primitives_and_Intersection_Acceleration/Bounding_Volume_Hierarchies.html

class BVH {
public:

	BVH(
		const std::vector<GPUVertex>& gpuVertices,
		std::vector<GPUTriangle>& gpuTriangles,
		int maxPrimsPerNode,
		SplitMethod splitMethod
	);

	~BVH() {}

	BVHNode* RecursiveBuild(
		std::vector<BVHPrimitiveInfo>& primitiveInfo,
		int start,
		int end,
		int* totalNodes,
		const std::vector<GPUTriangle>& originalGPUTriangles,
		std::vector<GPUTriangle>& orderedGPUTriangles
	);

	int32_t FlattenBVHTree(BVHNode* node, int32_t* offset);

	void CreateBVHLeafNode(
		BVHNode* node,
		const int32_t start,
		const int32_t end,
		const int32_t numPrimitives,
		const SlimBounds& bounds,
		const std::vector<BVHPrimitiveInfo>& primitiveInfo,
		const std::vector<GPUTriangle>& originalGPUTriangles,
		std::vector<GPUTriangle>& orderedGPUTriangles
	);

	static float SplitAxisToVectorElement(const glm::vec3& vec, SplitAxis splitAxis);

	std::vector<LinearBVHNode> GetLinearBVH() const;
	int32_t GetBVHSize() const;

private:
	const int _maxPrimsPerNode;
	const SplitMethod _splitMethod;
	std::vector<LinearBVHNode> _nodes;
};

#endif // BVH_H_