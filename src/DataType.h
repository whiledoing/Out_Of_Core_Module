#ifndef _DATA_TYPE_H
#define _DATA_TYPE_H

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef unsigned long long ulonglong;
typedef __int64  int64;

template<typename T>
struct PixelElement
{
	union
	{
		struct
		{
			T r, g, b;
		};
		T data[3];
	};
};

typedef PixelElement<uchar> Vec3b;
typedef PixelElement<ushort> Vec3s;
typedef PixelElement<uint> Vec3i;

struct Size
{
	Size(size_t _rows = 0, size_t _cols = 0)
		: rows(_rows), cols(_cols)
	{
	}

	size_t cols;
	size_t rows;
};

struct RowMajorPoint
{
	RowMajorPoint(size_t _row, size_t _col) : row(_row), col(_col) {}
	size_t row;
	size_t col;
};

#endif