
#include "MemoryManager.h"

MemoryManager::MemoryManager() {
	memoryArrays.push_back(new unsigned char[memBlockSize]());
	freeChunks.push_back(std::list<MemoryChunk>());

	MemoryChunk m = {
		0, memBlockSize - 1, memBlockSize
	};
	freeChunks[0].push_back(m);
}