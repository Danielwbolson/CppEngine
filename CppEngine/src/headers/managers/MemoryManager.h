
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
	 *		outside of this class. Currently has 10mb buffers and cannot switch to a new buffer
	 *		when full.
	*/
namespace MemoryManager {

	// Functions/Variables only useable by functions within namespace
	namespace detail {
		// 10mb buffers on heap
		extern std::vector<unsigned char*> memoryArrays;
		extern std::vector<std::list<MemoryChunk> > freeChunks;

		// 10mbs
		const unsigned int memBlockSize = 10485760;
		// If you are bigger than 1kb...
		const unsigned int margin = 1024;

		extern int leftIndex;
		extern int rightIndex;

		// Sort and merge possible free memory chunks for later use
		void SortAndMerge();
	}

	void Init();
	void CleanUp();

	// Templated Allocation/Free functions for non-vector types
	// Allocate memory of the specified size and return the pointer (that must be cast)
	// Keeps all memory smaller than 'margin' on the right side and all larger chunks on the left
	// Stores array size right before pointer returned
	// https://eli.thegreenplace.net/2014/perfect-forwarding-and-universal-references-in-c
	template <class T, typename... Args>
	T* Allocate(Args&&... args) {

		// Allocate pointer
		T* ptr = Malloc<T>(sizeof(T));

		// Construct pointer object and return it
		return new (ptr) T(std::forward<Args>(args)...);
	}


	// Allocate space for an object of type T
	template <class T>
	T* Malloc(size_t size) {

		// Make sure size is 4 byte aligned
		size_t remainder = size % 4;
		if (remainder != 0)
			size += 4 - remainder;

		T* ptr;

		// Big chunks increment from left side of buffer
		if (size > detail::margin) {

			// Run through our free memory chunk arrays
			for (int i = 0; i < detail::freeChunks.size(); i++) {

				// Run through our blocks of available memory looking for a useable chunk
				for (auto& chunk = detail::freeChunks[i].begin(); chunk != detail::freeChunks[i].end(); chunk++) {

					// Allot 4 bytes for the size
					if (chunk->size == (unsigned int)size + 4) {
						*((unsigned int*)(&detail::memoryArrays[i][chunk->leftIndex])) = (unsigned int)size;
						ptr = (T*)(&detail::memoryArrays[i][chunk->leftIndex + 4]);
						detail::freeChunks[i].erase(chunk);
						return ptr;
					} else if (chunk->size > (unsigned int)size + 4) {
						*((unsigned int*)(&detail::memoryArrays[i][chunk->leftIndex])) = (unsigned int)size;
						ptr = (T*)(&detail::memoryArrays[i][chunk->leftIndex + 4]);
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
						ptr = (T*)(&detail::memoryArrays[i][chunk->rightIndex - size]);
						detail::freeChunks[i].erase(std::next(chunk).base());
						return ptr;
					} else if (chunk->size > (unsigned int)size + 4) {
						*((unsigned int*)(&detail::memoryArrays[i][chunk->rightIndex - size - 3])) = (unsigned int)size;
						ptr = (T*)(&detail::memoryArrays[i][chunk->rightIndex - size]);
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

	template <class T>
	void Free(T* t) {
		// Call destructor for recursive deletion
		t->~T();

		// Mark memory used by incoming pointer available for use

		// Memory size is always stored 4 bytes before actual memory
		unsigned int size = *((unsigned char *)t - 4);

		for (int i = 0; i < detail::memoryArrays.size(); i++) {
			int64_t dist = (unsigned char *)t - &detail::memoryArrays[i][0];

			// If we are inside of this memory block
			if (dist < detail::memBlockSize) {
				MemoryChunk m = {
					static_cast<unsigned int>(dist) - 4,
					static_cast<unsigned int>(dist) + size,
					size
				};
				detail::freeChunks[i].push_back(m);

				// No need to look in other chunks if we found the pointer already
				break;
			}
		}

		detail::SortAndMerge();

	}
}

#endif // MEMORY_MANAGER_H_