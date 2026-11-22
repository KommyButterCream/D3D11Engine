#pragma once

#include <stdint.h>
#include <memory.h>

template<typename T>
struct PtrArray
{
public:
	PtrArray() = default;
	~PtrArray() = default;

	PtrArray(const PtrArray&) = delete;
	PtrArray& operator=(const PtrArray&) = delete;

public:
	T** data = nullptr;
	uint32_t size = 0;
	uint32_t capacity = 0;

public:
	void Init(uint32_t initial = 8)
	{
		if (data)
			return;

		capacity = initial;
		data = (T**)malloc(sizeof(T*) * capacity);
	}

	void Clear()
	{
		if (data)
		{
			free(data);
			data = nullptr;
		}

		size = 0;
		capacity = 0;
	}

	void Push(T* ptr)
	{
		if (!data)
			Init();

		if (size == capacity)
			Grow();

		data[size++] = ptr;
	}

	T* Get(uint32_t index) const
	{
		return index < size ? data[index] : nullptr;
	}

private:
	void Grow()
	{
		uint32_t newCapacity = capacity * 2;
		T** newData = (T**)malloc(sizeof(T*) * newCapacity);

		if (!newData)
			return;

		memcpy(newData, data, sizeof(T*) * size);
		free(data);
		data = newData;
		capacity = newCapacity;
	}
};