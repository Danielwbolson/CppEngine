
#include "BVH.h"

#include "Model.h"
#include "Mesh.h"

#include <cmath>
#include <algorithm>

BVH::BVH(
	const std::vector<GPUVertex>& gpuVertices,
	std::vector<GPUTriangle>& gpuTriangles,
	uint32_t maxPrimsPerNode,
	SplitMethod splitMethod
) : _maxPrimsPerNode(std::min((uint32_t)255, maxPrimsPerNode)), _splitMethod(splitMethod) {

	if (gpuTriangles.size() == 0) {
		return;
	}

	// Build Triangle based BVH from our models


	uint32_t numPrimitives = (uint32_t)gpuTriangles.size();;

	// Get our necessary information for each primitive triangle
	std::vector<BVHPrimitiveInfo> primitiveInfo;
	primitiveInfo.reserve(numPrimitives);
	for (uint32_t i = 0; i < numPrimitives; i++) {

		SlimBounds slimBounds;

		glm::fvec3 positions[3];
		for (int32_t j = 0; j < 3; j += 1) {
			positions[j] = gpuVertices[gpuTriangles[i].indices[j]].position;
		}

		slimBounds.min = glm::vec3(
			std::fmin(positions[0].x, std::fmin(positions[1].x, positions[2].x)),
			std::fmin(positions[0].y, std::fmin(positions[1].y, positions[2].y)),
			std::fmin(positions[0].z, std::fmin(positions[1].z, positions[2].z))
		);
		slimBounds.max = glm::vec3(
			std::fmax(positions[0].x, std::fmax(positions[1].x, positions[2].x)),
			std::fmax(positions[0].y, std::fmax(positions[1].y, positions[2].y)),
			std::fmax(positions[0].z, std::fmax(positions[1].z, positions[2].z))
		);

		primitiveInfo.push_back(BVHPrimitiveInfo(i, slimBounds));
	}

	// Now, build our bvh
	uint32_t totalNodes = 0;
	std::vector<GPUTriangle> orderedGPUTriangles;
	BVHNode* root = nullptr;
	if (splitMethod == SplitMethod::SAH) {
		root = RecursiveBuild(primitiveInfo, 0, numPrimitives, &totalNodes, gpuTriangles, orderedGPUTriangles);
	}

	gpuTriangles.swap(orderedGPUTriangles);

	// Flatten our BVH hierarchy for sending to the GPU
	if (root) {
		_nodes = std::vector<LinearBVHNode>(totalNodes);
		uint32_t offset = 0;
		FlattenBVHTree(root, &offset);
	}

}

BVHNode* BVH::RecursiveBuild(
	std::vector<BVHPrimitiveInfo>& primitiveInfo,
	uint32_t start,
	uint32_t end,
	uint32_t* totalNodes,
	const std::vector<GPUTriangle>& originalGPUTriangles,
	std::vector<GPUTriangle>& orderedGPUTriangles
) {
	BVHNode* node = MemoryManager::Allocate<BVHNode>();
	(*totalNodes) += 1;

	// Calculate total bounds for everything within this box
	SlimBounds bounds;
	for (uint32_t i = start; i < end; i += 1) {
		bounds = SlimBounds::Union(bounds, primitiveInfo[i].bounds);
	}

	uint32_t numPrimitives = end - start;

	// leaf node
	if (numPrimitives == 1) {
		CreateBVHLeafNode(node, start, end, numPrimitives, bounds, primitiveInfo, originalGPUTriangles, orderedGPUTriangles);
		return node;
	}
	// interior node
	else {

		// Get a bounds of all the centers of our _primitives to determine the axis to split on
		SlimBounds centroidBounds;
		for (uint32_t i = start; i < end; i += 1) {
			centroidBounds = SlimBounds::Union(centroidBounds, primitiveInfo[i].center);
		}
		SplitAxis splitAxis = centroidBounds.MaximumExtent();

		uint32_t mid = (start + end) / 2;

		// If our bounds has no volume, create a leaf node with all _primitives.
		// Very unusual case.
		if (centroidBounds.IsEmpty(splitAxis)) {
			// leaf node
			CreateBVHLeafNode(node, start, end, numPrimitives, bounds, primitiveInfo, originalGPUTriangles, orderedGPUTriangles);
			return node;
		}
		else {

			if (numPrimitives <= 4) {
				// Divide into equal groups. SAH doesn't provide much value here anymore.

				std::nth_element(
					&primitiveInfo[start], &primitiveInfo[mid], &primitiveInfo[end - 1] + 1,
					[splitAxis](const BVHPrimitiveInfo& a, const BVHPrimitiveInfo& b) {
						float aValue = BVH::SplitAxisToVectorElement(a.center, splitAxis);
						float bValue = BVH::SplitAxisToVectorElement(b.center, splitAxis);
						return aValue < bValue;
					});
			}
			else {

				// Initialize our bucket info for our surface area heuristic partitions
				constexpr int numBuckets = 12;
				struct BucketInfo {
					int count = 0;
					SlimBounds bounds;
				};
				BucketInfo buckets[numBuckets];

				for (uint32_t i = start; i < end; i += 1) {
					float value =
						BVH::SplitAxisToVectorElement(
							centroidBounds.Offset(primitiveInfo[i].center),
							splitAxis
						);

					uint32_t b = (uint32_t)(numBuckets * value);
					if (b == numBuckets) {
						b = numBuckets - 1;
					}

					buckets[b].count += 1;
					buckets[b].bounds = SlimBounds::Union(buckets[b].bounds, primitiveInfo[i].bounds);
				}

				// Compute costs for splitting after each bucket
				float cost[numBuckets - 1];
				for (uint32_t i = 0; i < numBuckets - 1; i += 1) {

					SlimBounds b0;
					SlimBounds b1;
					uint32_t count0 = 0;
					uint32_t count1 = 0;

					for (uint32_t j = 0; j <= i; j += 1) {
						b0 = SlimBounds::Union(b0, buckets[j].bounds);
						count0 += buckets[j].count;
					}

					for (uint32_t j = i + 1; j < numBuckets; j += 1) {
						b1 = SlimBounds::Union(b1, buckets[j].bounds);
						count1 += buckets[j].count;
					}

					cost[i] = (1.0f / 8.0f) + (count0 * b0.SurfaceArea() + count1 * b1.SurfaceArea()) / bounds.SurfaceArea();
				}

				// find bucket to split at the the minimal surface area heuristic metric
				float minCost = cost[0];
				uint32_t minCostSplitBucket = 0;
				for (uint32_t i = 1; i < numBuckets - 1; i += 1) {
					if (cost[i] < minCost) {
						minCost = cost[i];
						minCostSplitBucket = i;
					}
				}

				// Either create a leaf node or split _primitives based on the SAH buckets
				float leafCost = (float)(numPrimitives);
				if (numPrimitives > _maxPrimsPerNode || minCost < leafCost) {

					BVHPrimitiveInfo* primMid = std::partition(
						&primitiveInfo[start], &primitiveInfo[end - 1] + 1,
						[=](const BVHPrimitiveInfo& primInfo) {
							float value = BVH::SplitAxisToVectorElement(centroidBounds.Offset(primInfo.center), splitAxis);
							uint32_t b = (uint32_t)(numBuckets * value);
							if (b == numBuckets) {
								b = numBuckets - 1;
							}
							return b <= minCostSplitBucket;
						});
					mid = (uint32_t)(primMid - &primitiveInfo[0]);
				}
				else {
					CreateBVHLeafNode(node, start, end, numPrimitives, bounds, primitiveInfo, originalGPUTriangles, orderedGPUTriangles);
					return node;
				}
			}

			node->InitInterior(splitAxis,
				RecursiveBuild(primitiveInfo, start, mid, totalNodes, originalGPUTriangles, orderedGPUTriangles),
				RecursiveBuild(primitiveInfo, mid, end, totalNodes, originalGPUTriangles, orderedGPUTriangles)
			);
		}
	}

	return node;
}

// TODO: Handle child nodes being null
uint32_t BVH::FlattenBVHTree(BVHNode* node, uint32_t* offset) {
	LinearBVHNode* linearNode = &_nodes[*offset];
	linearNode->boundsMin = node->bounds.min;
	linearNode->boundsMax = node->bounds.max;
	uint32_t myOffset = (*offset) += 1;
	if (node->numPrimitives > 0) {
		linearNode->offset = node->firstPrimOffset;
		linearNode->numPrimitives = node->numPrimitives;
	}
	else {
		linearNode->axis = node->splitAxis;
		linearNode->numPrimitives = 0;
		FlattenBVHTree(node->children[0], offset);
		linearNode->offset =
			FlattenBVHTree(node->children[1], offset);
	}

	return myOffset;
}

void BVH::CreateBVHLeafNode(
	BVHNode* node,
	const uint32_t start,
	const uint32_t end,
	const uint32_t numPrimitives,
	const SlimBounds& bounds,
	const std::vector<BVHPrimitiveInfo>& primitiveInfo,
	const std::vector<GPUTriangle>& originalGPUTriangles,
	std::vector<GPUTriangle>& orderedGPUTriangles
) {
	uint32_t firstPrimOffset = (uint32_t)orderedGPUTriangles.size();

	for (uint32_t i = start; i < end; i += 1) {
		uint32_t primNum = primitiveInfo[i].primitiveNumber;
		orderedGPUTriangles.push_back(originalGPUTriangles[primNum]);
	}

	node->InitLeaf(firstPrimOffset, numPrimitives, bounds);
}

float BVH::SplitAxisToVectorElement(const glm::vec3& vec, SplitAxis splitAxis) {

	float value = 0;

	switch (splitAxis) {
	case SplitAxis::X:
		value = vec.x;
		break;
	case SplitAxis::Y:
		value = vec.y;
		break;
	case SplitAxis::Z:
		value = vec.z;
		break;
	}

	return value;
}

std::vector<LinearBVHNode> BVH::GetLinearBVH() const {
	return _nodes;
}

uint32_t BVH::GetBVHSize() const {
	return (int32_t)_nodes.size();
}