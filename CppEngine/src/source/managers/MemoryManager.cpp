
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

	void* Malloc(size_t size) {

		// Make sure size is 4 byte aligned
		size_t remainder = size % 4;
		if (remainder != 0)
			size += 4 - remainder;

		void* ptr;

		// Big chunks increment from left side of buffer
		if (size > detail::margin) {

			// Run through our free memory chunk arrays
			for (int i = 0; i < detail::freeChunks.size(); i++) {

				// Run through our blocks of available memory looking for a useable chunk
				for (auto& chunk = detail::freeChunks[i].begin(); chunk != detail::freeChunks[i].end(); chunk++) {

					// Allot 4 bytes for the size
					if (chunk->size == (unsigned int)size + 4) {
						*((unsigned int*)(&detail::memoryArrays[i][chunk->leftIndex])) = (unsigned int)size;
						ptr = &detail::memoryArrays[i][chunk->leftIndex + 4];
						detail::freeChunks[i].erase(chunk);
						return ptr;
					} else if (chunk->size > (unsigned int)size + 4) {
						*((unsigned int*)(&detail::memoryArrays[i][chunk->leftIndex])) = (unsigned int)size;
						ptr = &detail::memoryArrays[i][chunk->leftIndex + 4];
						chunk->leftIndex += (unsigned int)size + 4;
						chunk->size -= (unsigned int)size + 4;
						return ptr;
					}

				}

				// If we are at the end of our free chunks and had no luck finding space, make a new array, 
				// a new freeChunks array and add our new free chunk
				if (i == detail::freeChunks.size() - 1) {
					fprintf(stderr, "Making another memory block\n");
					detail::memoryArrays.push_back(new unsigned char[detail::memBlockSize]());
					detail::freeChunks.push_back(std::list<MemoryChunk>());

					MemoryChunk m = {
						0, detail::memBlockSize - 1, detail::memBlockSize
					};
					detail::freeChunks[i + 1].push_back(m);
				}

			}

			// Impossible to make it here?
			fprintf(stderr, "Failed to allocate memory\n");
			exit(-1);

		} else { // Smaller chunks increment from right side of buffer

			// Run through our free memory chunk arrays
			for (int i = 0; i < detail::freeChunks.size(); i++) {

				// Run through our block of memory looking for a useable chunk
				for (auto& chunk = detail::freeChunks[i].rbegin(); chunk != detail::freeChunks[i].rend(); chunk++) {

					// Allot 4 bytes for the size
					if (chunk->size == (unsigned int)size + 4) {
						// We subtract three here instead of 4 because we need to +1 to cancel the fact that the right index
						// is 1 back from leftIndex + size
						*((unsigned int*)(&detail::memoryArrays[i][chunk->rightIndex - size - 3])) = (unsigned int)size;
						ptr = &detail::memoryArrays[i][chunk->rightIndex - size];
						detail::freeChunks[i].erase(std::next(chunk).base());
						return ptr;
					} else if (chunk->size > (unsigned int)size + 4) {
						*((unsigned int*)(&detail::memoryArrays[i][chunk->rightIndex - size - 3])) = (unsigned int)size;
						ptr = &detail::memoryArrays[i][chunk->rightIndex - size];
						chunk->rightIndex -= (unsigned int)size + 4;
						chunk->size -= (unsigned int)size + 4;
						return ptr;
					}

				}

				// If we are at the end of our free chunks and have no luck, make a new array, 
				// a new freeChunks array and add our new free chunk
				if (i == detail::freeChunks.size() - 1) {
					fprintf(stderr, "Making another memory block\n");
					detail::memoryArrays.push_back(new unsigned char[detail::memBlockSize]());
					detail::freeChunks.push_back(std::list<MemoryChunk>());

					MemoryChunk m = {
						0, detail::memBlockSize - 1, detail::memBlockSize
					};
					detail::freeChunks[i + 1].push_back(m);
				}

			}

			// Impossible to make it here?
			fprintf(stderr, "Failed to allocate memory\n");
			exit(-1);

		}
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