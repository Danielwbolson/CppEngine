
#ifndef MEMORY_ALLOCATOR_H_
#define MEMORY_ALLOCATOR_H_

#include "MemoryManager.h"

#include <memory>

// resource: www.josuttis.com/libbook/memory/myalloc.hpp.html

// Custom Allocator class to allow us to store vectors using our MemoryManager
template <class T>
struct MemoryAllocator {
	typedef T value_type;
	typedef T* pointer;
	typedef const T* const_pointer;
	typedef T& reference;
	typedef const T& const_reference;
	typedef std::size_t size_type;
	typedef std::ptrdiff_t difference_type;
	typedef std::true_type propagate_on_container_move_assignment;
	typedef std::true_type is_always_equal;

	template <class U> 
	struct rebind { 
		typedef MemoryAllocator<U> other;
	};

	pointer address(reference x) const { return &x; }
	const_pointer address(const_reference x) const { return &x; }

	// Constuctors/Destructors
	MemoryAllocator() noexcept {}
	MemoryAllocator(const MemoryAllocator&) noexcept {}
	template <class U>
	MemoryAllocator(const MemoryAllocator<U>&) noexcept {}
	~MemoryAllocator() {}

	// Functions

	size_type max_size() const throw() {
		return (size_t)MemoryManager::detail::memBlockSize;
	}

	// Allocate a number of size T elements
	pointer allocate(size_type num) {
		pointer t = MemoryManager::Malloc<T>(num * sizeof(T));
		return t;
	}

	// Initialie our memory
	template< class U, typename... Args >
	void construct(U* ptr, Args&&... args) {
		new (ptr) U(std::forward<Args>(args)...);
	}

	// Deallocate our pointer
	void deallocate(T* ptr, size_type num) {
		MemoryManager::Free(ptr);
	}

	// Destroy our pointer via destructor
	template< class U >
	void destroy(U* ptr) {
		ptr->~U();
	}

};

template <class T1, class T2>
bool operator== (const MemoryAllocator<T1>&, const MemoryAllocator<T2>&) noexcept {
	return true;
}
template <class T1, class T2>
bool operator!= (const MemoryAllocator<T1>&, const MemoryAllocator<T2>&) noexcept {
	return false;
}

#endif // MEMORY_ALLOCATOR_H_
