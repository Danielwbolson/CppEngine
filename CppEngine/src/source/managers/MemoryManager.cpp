
#include "MemoryManager.h"

namespace MemoryManager {

	void Init() {
		detail::memoryArrays.push_back(new unsigned char[detail::memBlockSize]());
		detail::freeChunks.push_back(std::list<MemoryChunk>());

		MemoryChunk m = {
			0, detail::memBlockSize - 1, detail::memBlockSize
		};
		detail::freeChunks[0].push_back(m);
	}

	void CleanUp() {
		for (int i = 0; i < detail::memoryArrays.size(); i++) {
			delete detail::memoryArrays[i];
		}
		detail::memoryArrays.clear();

		for (int i = 0; i < detail::freeChunks.size(); i++) {
			detail::freeChunks[i].clear();
		}
		detail::freeChunks.clear();
	}

	namespace detail {

		std::vector<unsigned char*> memoryArrays;
		std::vector<std::list<MemoryChunk> > freeChunks;
		int leftIndex = 0;
		int rightIndex = memBlockSize - 1;

		void SortAndMerge() {
			for (int i = 0; i < freeChunks.size(); i++) {
				freeChunks[i].sort();

				// chunk2's initialization fails with an empty list
				// https://stackoverflow.com/questions/37016845/c-iterator-access-next-element-for-comparison
				if (freeChunks[i].empty()) return;

				auto chunk1 = freeChunks[i].begin();
				auto chunk2 = ++freeChunks[i].begin();

				while (chunk2 != freeChunks[i].end()) {

					// If we are merging an element
					if (chunk1->rightIndex >= chunk2->leftIndex - 1) {
						unsigned int newSize = chunk2->rightIndex - chunk1->leftIndex + 1;

						chunk2->leftIndex = chunk1->leftIndex;
						chunk2->size = newSize;

						chunk1 = freeChunks[i].erase(chunk1);
						chunk2++;

					} else {
						chunk1++;
						chunk2++;
					}

				}
			}
		}

	}

}