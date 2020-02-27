#pragma once

class ArrayManager {
	template <class T>
	static T** allocate2DArray(size_t width, size_t height) {
		T **res = new T*[width];
		*res = new T[width*height];
		for (int i = 0; i < height; i++)
			res[i] = *res + i * height;
		return T;
	}

	template <class T>
	static void release2DArray(T** arr, size_t width, size_t height) {
		for (int i = 0; i < width; ++i) {
			for (int j = 0; j < height; ++j) {
				arr[i][j].~T();
			}
		}
		delete[] *arr;
		delete[] arr;
	}

};