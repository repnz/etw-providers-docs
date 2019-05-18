#pragma once

template <typename T>
struct heap_deleter {
	void operator()(T * b) { 
		if (b != NULL) {
			free(b);
		}
	}
};

template <typename T>
using heap_ptr = std::unique_ptr<T, heap_deleter<T>>;

template <typename T>
heap_ptr<T> heap_malloc(size_t size) {
	void* mem = malloc(size);

	if (mem == NULL) {
		throw std::bad_alloc();
	}

	return heap_ptr<T>((T*)mem);
}
