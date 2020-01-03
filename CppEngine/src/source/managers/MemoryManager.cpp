
#include "MemoryManager.h"

MemoryManager::MemoryManager() {
	memoryArrays.push_back(new unsigned char[memBlockSize]());

	MemoryChunk m = {
		0, memBlockSize - 1, memBlockSize
	};
	freeChunks.push_back(m);
}
