#pragma once

#include <cstdint>

template <typename T>
inline int64_t BinSearch(T* array, T elem, int64_t size)
{
	int64_t start = 0;
	int64_t end = size - 1;

	int64_t mid = -1;
	while (start <= end)
	{
		mid = (start + end) / 2;

#ifdef _DEBUG
		std::cout << "start=" << start << " end=" << end << " mid=" << mid << std::endl;
#endif // DEBUG

		if (array[mid] == elem)
		{
			return mid;
		}

		if (elem > array[mid])
		{
			start = mid + 1;
		}
		else
		{
			end = mid - 1;
		}
	}

	return -1;
}
