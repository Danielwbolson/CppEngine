
#ifndef BVH_TYPES_H_
#define BVH_TYPES_H_

#include "GlobalMacros.h"

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"

#include <cmath>

/***** * * * * * CPU * * * * * *****/

enum class SplitMethod {
	SAH, HLBVH, Middle, EqualCounts
};

enum class SplitAxis : uint32_t {
	X, Y, Z
};

// 24 bytes
struct SlimBounds {
public:
	SlimBounds() {
		min = glm::vec3(INFINITY, INFINITY, INFINITY);
		max = glm::vec3(-INFINITY, -INFINITY, -INFINITY);
	}

	SlimBounds(const glm::vec3& inMin, const glm::vec3& inMax) {
		min = inMin;
		max = inMax;
	};

	SlimBounds(const SlimBounds& b) {
		min = b.min;
		max = b.max;
	}

	bool Intersect(glm::vec3 p) const {
		uint8_t intersect = 0;
		intersect += (p.x < max.x&& p.x > min.x);
		intersect += (p.y < max.y&& p.y > min.y);
		intersect += (p.z < max.z&& p.z > min.z);

		return intersect == 3;
	}

	SplitAxis MaximumExtent() const {
		glm::vec3 extentVector = max - min;

		if (extentVector.x > extentVector.y && extentVector.x > extentVector.z) {
			return SplitAxis::X;
		}
		else if (extentVector.y > extentVector.z) {
			return SplitAxis::Y;
		}
		else {
			return SplitAxis::Z;
		}
	}

	static SlimBounds Union(const SlimBounds& a, const SlimBounds& b) {
		SlimBounds newBounds;

		newBounds.min = glm::vec3(
			std::fmin(a.min.x, b.min.x),
			std::fmin(a.min.y, b.min.y),
			std::fmin(a.min.z, b.min.z)
		);
		newBounds.max = glm::vec3(
			std::fmax(a.max.x, b.max.x),
			std::fmax(a.max.y, b.max.y),
			std::fmax(a.max.z, b.max.z)
		);

		return newBounds;
	}

	static SlimBounds Union(const SlimBounds& a, const glm::vec3& b) {
		SlimBounds newBounds;

		newBounds.min = glm::vec3(
			std::fmin(a.min.x, b.x),
			std::fmin(a.min.y, b.y),
			std::fmin(a.min.z, b.z)
		);
		newBounds.max = glm::vec3(
			std::fmax(a.max.x, b.x),
			std::fmax(a.max.y, b.y),
			std::fmax(a.max.z, b.z)
		);

		return newBounds;
	}

	bool IsEmpty(SplitAxis maxAxis) const {
		float epsilon = 0.00001f;
		float diff;

		switch (maxAxis) {
		case SplitAxis::X:
			diff = max.x - min.x;
			break;
		case SplitAxis::Y:
			diff = max.y - min.y;
			break;
		case SplitAxis::Z:
			diff = max.z - min.z;
			break;
		}

		return diff < epsilon;
	}

	glm::vec3 Offset(const glm::vec3& vec) const {
		glm::vec3 offsetVec = vec - min;
		if (max.x > min.x) offsetVec.x /= (max.x - min.x);
		if (max.y > min.y) offsetVec.y /= (max.y - min.y);
		if (max.z > min.z) offsetVec.z /= (max.z - min.z);
		return offsetVec;
	}

	float SurfaceArea() const {
		glm::vec3 extentVector = max - min;
		return 2 * (
			extentVector.x * extentVector.y +
			extentVector.x * extentVector.z +
			extentVector.y * extentVector.z
		);
	}

	glm::vec3 min;
	glm::vec3 max;
};

struct BVHPrimitiveInfo {
public:
	BVHPrimitiveInfo(int primId, SlimBounds primBounds) {
		primitiveNumber = primId;
		bounds = primBounds;
		center = 0.5f * bounds.min + 0.5f * bounds.max;
	}

	unsigned int primitiveNumber;
	SlimBounds bounds;
	glm::vec3 center;

};

struct BVHNode {
public:
	void InitLeaf(uint32_t first, uint32_t n, const SlimBounds& inBounds) {
		firstPrimOffset = first;
		numPrimitives = n;
		bounds = inBounds;
		for (int i = 0; i < 2; i += 1) {
			children[i] = nullptr;
		}
	}

	void InitInterior(SplitAxis axis, BVHNode* child0, BVHNode* child1) {
		children[0] = child0;
		children[1] = child1;

		bounds = SlimBounds::Union(child0->bounds, child1->bounds);
		splitAxis = axis;
		numPrimitives = 0;
	}

	SlimBounds bounds;
	BVHNode* children[2];
	SplitAxis splitAxis;
	uint32_t firstPrimOffset;
	uint32_t numPrimitives;
};

struct BVHTriangle {
public:
	glm::vec3 positions[3];
};



/***** * * * * * GPU * * * * * *****/

#pragma pack(push, 1)
struct LinearBVHNode {
public:
	glm::vec3 boundsMin;
	uint32_t offset; // primitivesOffset --> leaf, secondChildOffset --> interior

	glm::vec3 boundsMax;
	uint32_t numPrimitives_and_axis; // 16/16
};
#pragma pack(pop)
ASSERT_GPU_ALIGNMENT(LinearBVHNode, 16);
ASSERT_STRUCT_UP_TO_DATE(LinearBVHNode, 32);


#endif