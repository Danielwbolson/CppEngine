
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
		// buffers on heap
		extern std::vector<unsigned char*> memoryArrays;
		extern std::vector<std::list<MemoryChunk> > freeChunks;

		// 1mb
		const unsigned int memBlockSize = 1048576;
		// If you are bigger than 1kb...
		const unsigned int margin = 1024;

		extern int leftIndex;
		extern int rightIndex;

		// Sort and merge possible free memory chunks for later use
		void SortAndMerge();
	}

	void Init();
	void CleanUp();
	// Allocate space for an object of size
	void* Malloc(size_t size);

	// Templated Allocation/Free functions for non-vector types
	// Allocate memory of the specified size and return the pointer (that must be cast)
	// Keeps all memory smaller than 'margin' on the right side and all larger chunks on the left
	// Stores array size right before pointer returned
	// https://eli.thegreenplace.net/2014/perfect-forwarding-and-universal-references-in-c
	template <class T, typename... Args>
	T* Allocate(Args&&... args) {

		// Allocate pointer
		T* ptr = static_cast<T*>(Malloc(sizeof(T)));

		// Construct pointer object and return it
		return new (ptr) T(std::forward<Args>(args)...);
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