
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

				if (chunk->size == size + 1) {
					memoryArrays[0][chunk->leftIndex] = size;
					ptr = &memoryArrays[0][chunk->leftIndex + 1];
					freeChunks.erase(chunk);
					return new (ptr) T(std::forward<Args>(args)...);
				} else if (chunk->size > size + 1) {
					memoryArrays[0][chunk->leftIndex] = size;
					ptr = &memoryArrays[0][chunk->leftIndex + 1];
					chunk->leftIndex += size + 1;
					chunk->size -= size + 1;
					return new (ptr) T(std::forward<Args>(args)...);
				}
			}

			fprintf(stderr, "ERROR: No room found for dynamic memory in Memory Manager\n");
			exit(-1);

		} else { // Smaller chunks increment from right side of buffer

			// Run through our block of memory looking for a useable chunk
			for (auto& chunk = freeChunks.rbegin(); chunk != freeChunks.rend(); chunk++) {

				if (chunk->size == size + 1) {
					memoryArrays[0][chunk->rightIndex - size - 1] = size;
					ptr = &memoryArrays[0][chunk->rightIndex - size];
					freeChunks.erase(std::next(chunk).base());
					return new (ptr) T(std::forward<Args>(args)...);
				} else if (chunk->size > size + 1) {
					memoryArrays[0][chunk->rightIndex - size - 1] = size;
					ptr = &memoryArrays[0][chunk->rightIndex - size];
					chunk->rightIndex -= size + 1;
					chunk->size -= size + 1;
					return new (ptr) T(std::forward<Args>(args)...);
				}
			}

			fprintf(stderr, "ERROR: No room found for dynamic memory in Memory Manager\n");
			exit(-1);
		}
	}

	template <class T>
	void* Free(T* t) {
		t->~T();



		freeChunks.sort();
		return nullptr;
	}
};

#endif // MEMORY_MANAGER_H_