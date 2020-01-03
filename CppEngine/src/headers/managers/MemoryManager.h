
#ifndef MEMORY_MANAGER_H_
#define MEMORY_MANAGER_H_

#include <vector>
#include <list>
#include <iostream>

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


private:
	// Sort and merge possible free memory chunks for later use
	void SortAndMerge();

	// 100mb buffers on heap
	std::vector<unsigned char*> memoryArrays;
	std::list<MemoryChunk> freeChunks;

	const unsigned int memBlockSize = 104857600;
	const unsigned int margin = 1024;
	int leftIndex = 0;
	int rightIndex = memBlockSize - 1;


	// Template Allocation function
public:
	// Allocate memory of the specified size and return the pointer (that must be cast)
	// Keeps all memory smaller than 'margin' on the right side and all larger chunks on the left
	// Stores array size right before pointer returned
	// https://eli.thegreenplace.net/2014/perfect-forwarding-and-universal-references-in-c
	template <class T, typename... Args>
	T* Allocate(Args&&... args) {

		unsigned int size = sizeof(T);
		void* ptr;

		// Big chunks increment from left side of buffer
		if (size > margin) {

			// Run through our block of memory looking for a useable chunk
			for (auto& chunk = freeChunks.begin(); chunk != freeChunks.end(); chunk++) {

				// Allot 4 bytes for the size
				if (chunk->size == size + 4) {
					*((unsigned int*)(&memoryArrays[0][chunk->leftIndex])) = size;
					ptr = &memoryArrays[0][chunk->leftIndex + 4];
					freeChunks.erase(chunk);
					return new (ptr) T(std::forward<Args>(args)...);
				} else if (chunk->size > size + 4) {
					*((unsigned int*)(&memoryArrays[0][chunk->leftIndex])) = size;
					ptr = &memoryArrays[0][chunk->leftIndex + 4];
					chunk->leftIndex += size + 4;
					chunk->size -= size + 4;
					return new (ptr) T(std::forward<Args>(args)...);
				}
			}

			fprintf(stderr, "ERROR: No room found for dynamic memory in Memory Manager\n");
			exit(-1);

		} else { // Smaller chunks increment from right side of buffer

			// Run through our block of memory looking for a useable chunk
			for (auto& chunk = freeChunks.rbegin(); chunk != freeChunks.rend(); chunk++) {

				// Allot 4 bytes for the size
				if (chunk->size == size + 4) {
					*((unsigned int*)(&memoryArrays[0][chunk->rightIndex - size - 4])) = size;
					ptr = &memoryArrays[0][chunk->rightIndex - size];
					freeChunks.erase(std::next(chunk).base());
					return new (ptr) T(std::forward<Args>(args)...);
				} else if (chunk->size > size + 4) {
					*((unsigned int*)(&memoryArrays[0][chunk->rightIndex - size - 4])) = size;
					ptr = &memoryArrays[0][chunk->rightIndex - size];
					chunk->rightIndex -= size + 4;
					chunk->size -= size + 4;
					return new (ptr) T(std::forward<Args>(args)...);
				}
			}

			fprintf(stderr, "ERROR: No room found for dynamic memory in Memory Manager\n");
			exit(-1);
		}
	}

	template <class T>
	void* Free(T* t) {
		// Call destructor for recursive deletion
		t->~T();

		// Mark memory used by incoming pointer available for use

		// Memory size is always stored 4 bytes before actual memory
		unsigned int size = *((unsigned char *)t - 4);

		for (int i = 0; i < memoryArrays.size(); i++) {
			int64_t dist = (unsigned char *)t - &memoryArrays[i][0];

			// If we are inside of this memory block
			if (dist < memBlockSize) {
				MemoryChunk m = {
					static_cast<unsigned int>(dist) - 4,
					static_cast<unsigned int>(dist) + size,
					size
				};
				freeChunks.push_back(m);
			}
		}

		SortAndMerge();

		return nullptr;
	}
};

#endif // MEMORY_MANAGER_H_