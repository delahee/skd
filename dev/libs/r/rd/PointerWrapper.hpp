#pragma once

template <typename T>
class PointerWrapper {
	T* ptr;
	int maxSize = 1;
public:
	PointerWrapper() { ptr = new T(); maxSize = 1; }
	PointerWrapper(int size) { ptr = new T[size]; maxSize = size; }
	~PointerWrapper() { delete ptr; }

	T* GetPtr() { return ptr; }
	void* GetVoidPtr() { return ptr; }
	int GetSize() { return maxSize; }

	T Get() { return *ptr; }
	T GetAt(int index) { return ptr[index]; }
	void Set(T value) { *ptr = value; }
	void SetAt(int index, T value) { ptr[index] = value; }

	void MemSet(T value) { memset(ptr, value, maxSize * sizeof(T)); }
	void MemSet(T value, int size) { memset(ptr, value, size * sizeof(T)); }
	void MemCpy(const T* src, int size) { memcpy(ptr, src, size * sizeof(T)); }
	void StrCpy(const T* src) { strcpy(ptr, src); }
};

typedef PointerWrapper<char> PointerWrapperChar;