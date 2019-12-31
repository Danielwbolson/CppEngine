
#include "MemoryManager.h"
#include <iostream>

MemoryManager::MemoryManager() {
	memoryArrays.push_back(new unsigned char[memBlockSize]());

	MemoryChunk m = {
		0, memBlockSize - 1, memBlockSize
	};
	freeChunks.push_back(m);
}

void* MemoryManager::Allocate(const unsigned int& size) {

	void* ptr;

	// Big chunks increment from left side of buffer
	if (size > margin) {

		// Run through our block of memory looking for a useable chunk
		for (auto& chunk = freeChunks.begin(); chunk != freeChunks.end(); chunk++) {
			
			if (chunk->size == size + 1) {
				memoryArrays[0][chunk->leftIndex] = size;
				ptr = &memoryArrays[0][chunk->leftIndex + 1];
				freeChunks.erase(chunk);
				return ptr;
			} else if (chunk->size > size + 1) {
				memoryArrays[0][chunk->leftIndex] = size;
				ptr = &memoryArrays[0][chunk->leftIndex + 1];
				chunk->leftIndex += size + 1;
				chunk->size -= size + 1;
				return ptr;
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
				return ptr;
			} else if (chunk->size > size + 1) {
				memoryArrays[0][chunk->rightIndex - size - 1] = size;
				ptr = &memoryArrays[0][chunk->rightIndex - size];
				chunk->rightIndex -= size + 1;
				chunk->size -= size + 1;
				return ptr;
			}
		}

		fprintf(stderr, "ERROR: No room found for dynamic memory in Memory Manager\n");
		exit(-1);
	}

	freeChunks.sort();
}

void* MemoryManager::Deallocate(void* ptr, const unsigned int& size) {
	freeChunks.sort();
	return nullptr;
}