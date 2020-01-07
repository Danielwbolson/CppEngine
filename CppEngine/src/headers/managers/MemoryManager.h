
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
template <class T>
class MemoryManager {

	// Allocator information
public:
	// www.josuttis.com/libbook/memory/myalloc.hpp.html
	typedef T value_type;
	typedef T* pointer;
	typedef const T* const_pointer;
	typedef T& reference;
	typedef const T& const_reference;
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;

	template <class U>
	struct rebind { typedef MemoryManager<U> other; };

	pointer address(reference value) const { return &value; }
	const_pointer address(const_reference value) const { return &value; }

	template <class U>
	operator=(const MemoryManager<U>&) {}


	// Constructors/Destructors
	MemoryManager() throw() {
		memoryArrays.push_back(new unsigned char[memBlockSize]());
		freeChunks.push_back(std::list<MemoryChunk>());

		MemoryChunk m = {
			0, memBlockSize - 1, memBlockSize
		};
		freeChunks[0].push_back(m);
	}

	MemoryManager(const MemoryManager&) throw() {}

	template <class U>
	MemoryManager(const MemoryManager<U>&) throw() {}

	~MemoryManager() throw() {
		for (int i = 0; i < memoryArrays.size; i++) {
			delete memoryArrays[];
		}
		memoryArrays.clear();

		for (int i = 0; i < freeChunks.size(); i++) {
			freeChunks[i].clear();
		}
		freeChunks.clear();
	}


	// Functions
	size_type max_size() const throw() {
		return static_cast<size_type>(memBlockSize);
	}

	pointer allocate(size_type num, const void* = 0) {}

	void construct(pointer p, const T& value) {}

	void destroy(pointer p) { 
		p->~T(); 
	}

	void deallocate(pointer p, size_type num) {}


private:
	// Sort and merge possible free memory chunks for later use
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

	// 10mb buffers on heap
	std::vector<unsigned char*> memoryArrays;
	std::vector<std::list<MemoryChunk> > freeChunks;

	// 10mbs
	const unsigned int memBlockSize = 10485760;
	// If you are bigger than 1kb...
	const unsigned int margin = 1024;
	int leftIndex = 0;
	int rightIndex = memBlockSize - 1;




	// Templated Allocation/Free functions for non-vector types
public:
	// Allocate memory of the specified size and return the pointer (that must be cast)
	// Keeps all memory smaller than 'margin' on the right side and all larger chunks on the left
	// Stores array size right before pointer returned
	// https://eli.thegreenplace.net/2014/perfect-forwarding-and-universal-references-in-c
	template <class U, typename... Args>
	U* Allocate(Args&&... args) {

		// Get size and make sure it is 4 byte aligned
		unsigned int size = sizeof(U);
		unsigned int remainder = size % 4;
		size += 4 - remainder;

		void* ptr;

		// Big chunks increment from left side of buffer
		if (size > margin) {

			// Run through our free memory chunk arrays
			for (int i = 0; i < freeChunks.size(); i++) {

				// Run through our blocks of available memory looking for a useable chunk
				for (auto& chunk = freeChunks[i].begin(); chunk != freeChunks[i].end(); chunk++) {

					// Allot 4 bytes for the size
					if (chunk->size == size + 4) {
						*((unsigned int*)(&memoryArrays[i][chunk->leftIndex])) = size;
						ptr = &memoryArrays[i][chunk->leftIndex + 4];
						freeChunks[i].erase(chunk);
						return new (ptr) U(std::forward<Args>(args)...);
					} else if (chunk->size > size + 4) {
						*((unsigned int*)(&memoryArrays[i][chunk->leftIndex])) = size;
						ptr = &memoryArrays[i][chunk->leftIndex + 4];
						chunk->leftIndex += size + 4;
						chunk->size -= size + 4;
						return new (ptr) U(std::forward<Args>(args)...);
					}

				}

				// If we are at the end of our free chunks and had no luck finding space, make a new array, 
				// a new freeChunks array and add our new free chunk
				if (i == freeChunks.size() - 1) {
					fprintf(stderr, "Making another memory block\n");
					memoryArrays.push_back(new unsigned char[memBlockSize]());
					freeChunks.push_back(std::list<MemoryChunk>());

					MemoryChunk m = {
						0, memBlockSize - 1, memBlockSize
					};
					freeChunks[i+1].push_back(m);
				}

			}

			// Impossible to make it here?
			fprintf(stderr, "Failed to allocate memory\n");
			exit(-1);

		} else { // Smaller chunks increment from right side of buffer

			// Run through our free memory chunk arrays
			for (int i = 0; i < freeChunks.size(); i++) {

				// Run through our block of memory looking for a useable chunk
				for (auto& chunk = freeChunks[i].rbegin(); chunk != freeChunks[i].rend(); chunk++) {

					// Allot 4 bytes for the size
					if (chunk->size == size + 4) {
						// We subtract three here instead of 4 because we need to +1 to cancel the fact that the right index
						// is 1 back from leftIndex + size
						*((unsigned int*)(&memoryArrays[i][chunk->rightIndex - size - 3])) = size;
						ptr = &memoryArrays[i][chunk->rightIndex - size];
						freeChunks[i].erase(std::next(chunk).base());
						return new (ptr) U(std::forward<Args>(args)...);
					} else if (chunk->size > size + 4) {
						*((unsigned int*)(&memoryArrays[i][chunk->rightIndex - size - 3])) = size;
						ptr = &memoryArrays[i][chunk->rightIndex - size];
						chunk->rightIndex -= size + 4;
						chunk->size -= size + 4;
						return new (ptr) U(std::forward<Args>(args)...);
					}

				}

				// If we are at the end of our free chunks and have no luck, make a new array, 
				// a new freeChunks array and add our new free chunk
				if (i == freeChunks.size() - 1) {
					fprintf(stderr, "Making another memory block\n");
					memoryArrays.push_back(new unsigned char[memBlockSize]());
					freeChunks.push_back(std::list<MemoryChunk>());

					MemoryChunk m = {
						0, memBlockSize - 1, memBlockSize
					};
					freeChunks[i + 1].push_back(m);
				}

			}

			// Impossible to make it here?
			fprintf(stderr, "Failed to allocate memory\n");
			exit(-1);

		}
	}

	template <class U>
	void* Free(U* u) {
		// Call destructor for recursive deletion
		u->~U();

		// Mark memory used by incoming pointer available for use

		// Memory size is always stored 4 bytes before actual memory
		unsigned int size = *((unsigned char *)u - 4);

		for (int i = 0; i < memoryArrays.size(); i++) {
			int64_t dist = (unsigned char *)u - &memoryArrays[i][0];

			// If we are inside of this memory block
			if (dist < memBlockSize) {
				MemoryChunk m = {
					static_cast<unsigned int>(dist) - 4,
					static_cast<unsigned int>(dist) + size,
					size
				};
				freeChunks[i].push_back(m);

				// No need to look in other chunks if we found the pointer already
				break;
			}
		}

		SortAndMerge();

		return nullptr;
	}
};

#endif // MEMORY_MANAGER_H_