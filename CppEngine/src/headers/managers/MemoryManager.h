
#ifndef MEMORY_MANAGER_H_
#define MEMORY_MANAGER_H_

#include <vector>
#include <list>

struct MemoryChunk {
	unsigned int leftIndex;
	unsigned int rightIndex;
	unsigned int size;

	bool operator < (const MemoryChunk& m2) {
		return leftIndex < m2.leftIndex;
	}
};
/*
 * Memory Manager:
 *		The goal of this class is to optimize cache performance and avoid the usage of 'new'
 *		outside of this class. Currently has 100mb buffers and cannot switch to a new buffer
 *		when full.
*/
class MemoryManager {
public:
	MemoryManager();

	// Allocate memory of the specified size and return the pointer (that must be cast)
	// Keeps all memory smaller than 'margin' on the right side and all larger chunks on the left
	// Stores array size right before pointer returned
	void* Allocate(const unsigned int& size);

	// Deallocate memory of the specified size at the location 'ptr' and return nullptr
	void* Deallocate(void* ptr, const unsigned int& size);

private:
	// 100mb buffers on heap
	std::vector<unsigned char*> memoryArrays;
	std::list<MemoryChunk> freeChunks;

	const unsigned int memBlockSize = 104857600;
	const unsigned int margin = 1024;
	int leftIndex = 0;
	int rightIndex = memBlockSize - 1;
};

#endif // MEMORY_MANAGER_H_