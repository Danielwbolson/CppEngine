
#include "MemoryManager.h"

MemoryManager::MemoryManager() {
	memoryArrays.push_back(new unsigned char[memBlockSize]());

	MemoryChunk m = {
		0, memBlockSize - 1, memBlockSize
	};
	freeChunks.push_back(m);
}

void MemoryManager::SortAndMerge() {

	freeChunks.sort();

	// chunk2's initialization fails with an empty list
	// https://stackoverflow.com/questions/37016845/c-iterator-access-next-element-for-comparison
	if (freeChunks.empty()) return;

	auto chunk1 = freeChunks.begin();
	auto chunk2 = ++freeChunks.begin();

	while (chunk2 != freeChunks.end()) {

		// If we are merging an element
		if (chunk1->rightIndex >= chunk2->leftIndex - 1) {
			unsigned int newSize = chunk2->rightIndex - chunk1->leftIndex + 1;

			chunk2->leftIndex = chunk1->leftIndex;
			chunk2->size = newSize;

			chunk1 = freeChunks.erase(chunk1);
			chunk2++;

		} else {
			chunk1++;
			chunk2++;
		}

	}
}