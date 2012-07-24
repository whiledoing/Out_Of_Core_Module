#ifndef _INDEX_METHOD_H
#define _INDEX_METHOD_H

#include "IndexMethodInterface.h"
#include <cmath>
#include <boost/assert.hpp>

class Block2DIndex : public IndexMethodInterface
{
private:

	//the size of the array
	IndexType m_row, m_col;

	//the size of the block area (the shift distance)
	IndexType m_blockRowSize, m_blockColSize;

	//the count of block along with each axis of the 2D array
	IndexType m_blockRowCount, m_blockColCount;

	//the total size of the block (the shift distance)
	IndexType m_blockTotalSize;

private:
	typedef IndexType ValueType;
	const IndexType ONE;

public:

	/*
	 * @para row_size, col_size : the size of the image
	 * @para block_row_size , block_col_size : the size of the block in the unit of 2^n,
	 * thus n = 2, then the block size is 4*4
	 */
	Block2DIndex(IndexType row_size, IndexType col_size, IndexType block_row_size, IndexType block_col_size = -1)
		: m_row(row_size), m_col(col_size), m_blockRowSize(block_row_size),
			m_blockColSize((block_col_size == -1) ? block_row_size : block_col_size), ONE(1)
	{
		BOOST_ASSERT_MSG(block_row_size <= (IndexType)(std::log((double)row_size) / std::log(2.0)),
			"block row size must less than log2(row_size)");

		BOOST_ASSERT_MSG(block_col_size <= (IndexType)(std::log((double)col_size) / std::log(2.0)),
			"block col size must less than log2(col_size)");

		ValueType result, mod;

		result = (m_row >> block_row_size); mod = m_row - (result << block_row_size);
		m_blockRowCount = result + ((mod == 0) ? 0 : 1);

		result = (m_col >> block_col_size); mod = m_col - (result << block_col_size);
		m_blockColCount = result + ((mod == 0) ? 0 : 1);

		m_blockTotalSize = (block_row_size + block_col_size);
	}

/* implement the interface */
public:
	virtual IndexType get_index(RowMajorIndexType row_index, RowMajorIndexType col_index) const
	{
		IndexType bx, by, dx, dy;
		bx = (row_index >> m_blockRowSize); dx = row_index - (bx << m_blockRowSize);
		by = (col_index >> m_blockColSize); dy = col_index - (by << m_blockColSize);

		return ((bx*m_blockColCount + by) << m_blockTotalSize) + (dx << m_blockColSize) + dy;
	}

	virtual RowMajorPoint get_origin_index(IndexType index) const
	{
		IndexType result, mod;
		result = index >> m_blockTotalSize; mod = index - (result << m_blockTotalSize);

		IndexType bx, by, dx, dy;
		bx = result / m_blockColCount; by = result - (bx * m_blockColCount);
		dx = mod >> m_blockColSize; dy = mod - (dx << m_blockColSize);

		return RowMajorPoint((bx << m_blockRowSize) + dx, (by << m_blockColSize) + dy);
	}

	virtual IndexType get_row_result(RowMajorIndexType row_index) const
	{
		IndexType bx, dx;
		bx = (row_index >> m_blockRowSize); dx = row_index - (bx << m_blockRowSize);
		return ((bx*m_blockColCount) << m_blockTotalSize) + (dx << m_blockColSize);
	}

	virtual IndexType get_index_by_row_result(IndexType row_result, RowMajorIndexType col_index) const
	{
		IndexType by, dy;
		by = (col_index >> m_blockColSize); dy = col_index - (by << m_blockColSize);
		return row_result + (by << m_blockTotalSize) + dy;
	}

	virtual IndexType get_max_index() const
	{
		return m_blockRowCount * m_blockColCount * (ONE << m_blockTotalSize);
	}

	virtual std::string get_index_method_name() const
	{
		return std::string("Block2DIndex");
	}

/* specific method */
public:
	IndexType getBlockRowCount() const { return m_blockRowCount; }
	IndexType getBlockColCount() const { return m_blockColCount; }
	IndexType getBlockTotalSize() const { return (ONE << m_blockTotalSize); }
	IndexType getBlockRowSize() const { return (ONE << m_blockRowSize); }
	IndexType getBlockColSize() const { return (ONE << m_blockColSize); }
};

class ZOrderIndexIntuition : public IndexMethodInterface
{
private:
	const IndexType ONE;
	IndexType *markEven;
	IndexType *markOdd;
	IndexType *markAll;
	IndexType m_row, m_col;

public:
	/*
	 *	@para row_size, col_size : the size of the image
	 */
	ZOrderIndexIntuition(RowMajorIndexType row_size, RowMajorIndexType col_size)
		: ONE(1) , m_row(row_size), m_col(col_size)
	{
		init();
	}
	virtual ~ZOrderIndexIntuition() { release(); }

private:

	/*
	 *	@brief : init the markEven, markOdd, markAll array for boost the calculation
	 */
	void init() {
		int bitSize = 8*sizeof(IndexType);
		markEven = new IndexType[bitSize];
		markOdd = new IndexType[bitSize];
		markAll = new IndexType[bitSize];

		IndexType bitOdd = 1, bitEven = 2;
		for(int i = 0; i < (bitSize/2); ++i) {
			markOdd[i] = bitOdd;
			markEven[i] = bitEven;
			bitOdd <<= 2;
			bitEven <<= 2;
		}

		IndexType bitAll = 1;
		for(int i = 0; i < bitSize; ++i) {
			markAll[i] = bitAll;
			bitAll <<= 1;
		}
	}

	void release() {
		delete []markEven;
		delete []markOdd;
		delete []markAll;
	}

public:
	virtual IndexType get_row_result(RowMajorIndexType row_index) const {
		IndexType result = 0;
		int loc = 0;

		while(row_index) {
			if(row_index & ONE) {
				result |= markEven[loc];
			}
			row_index >>= 1;
			++loc;
		}
		return result;
	}

	virtual IndexType get_index_by_row_result(IndexType row_result, RowMajorIndexType col_index) const {
		int loc = 0;
		while(col_index) {
			if(col_index & ONE) {
				row_result |= markOdd[loc];
			}
			col_index >>= 1;
			++loc;
		}
		return row_result;
	}

	virtual IndexType get_index(RowMajorIndexType row_index, RowMajorIndexType col_index) const {
		IndexType result = 0;
		int loc = 0;

		while(row_index) {
			if(row_index & ONE) {
				result |= markEven[loc];
			}
			row_index >>= 1;
			++loc;
		}

		loc = 0;
		while(col_index) {
			if(col_index & ONE) {
				result |= markOdd[loc];
			}
			col_index >>= 1;
			++loc;
		}

		return result;
	}

	virtual RowMajorPoint get_origin_index(IndexType index) const {
		size_t row = 0, col = 0;
		IndexType loc = 0;
		while(index) {
			if(index & ONE)	col |= markAll[loc];
			if((index >>= 1) & ONE)	row |= markAll[loc];
			index >>= 1;
			++loc;
		}
		return RowMajorPoint(row, col);
	}

	virtual IndexType get_max_index() const
	{
		return get_index(m_row - 1, m_col - 1);
	}

	virtual std::string get_index_method_name() const
	{
		return std::string("ZOrderIndexIntuition");
	}
};

class ZOrderIndex : public IndexMethodInterface
{
private:
	const IndexType ONE;
	IndexType *markAll;
	IndexType m_row, m_col;

public:
	/*
	 *	@para row_size, col_size : the size of the image
	 */
	ZOrderIndex(RowMajorIndexType row_size, RowMajorIndexType col_size)
		: ONE(1) , m_row(row_size), m_col(col_size)
	{
		init();
	}
	virtual ~ZOrderIndex() { release(); }

private:

	/*
	 *	@brief : init the markEven, markOdd, markAll array for boost the calculation
	 */
	void init() {
		int bitSize = 8*sizeof(IndexType);

		markAll = new IndexType[bitSize];

		IndexType bitAll = 1;
		for(int i = 0; i < bitSize; ++i) {
			markAll[i] = bitAll;
			bitAll <<= 1;
		}
	}

	void release() {
		delete []markAll;
	}

public:
	virtual IndexType get_row_result(RowMajorIndexType row_index) const {
		static const IndexType B[] = {0x5555555555555555, 0x3333333333333333,
			0x0F0F0F0F0F0F0F0F, 0x00FF00FF00FF00FF, 0x0000FFFF0000FFFF};
		static const IndexType S[] = {1, 2, 4, 8, 16};

		IndexType y = row_index;

		y = (y | (y << S[4])) & B[4];
		y = (y | (y << S[3])) & B[3];
		y = (y | (y << S[2])) & B[2];
		y = (y | (y << S[1])) & B[1];
		y = (y | (y << S[0])) & B[0];

		return (y << 1);
	}

	virtual IndexType get_index_by_row_result(IndexType row_result, RowMajorIndexType col_index) const {
		static const IndexType B[] = {0x5555555555555555, 0x3333333333333333,
			0x0F0F0F0F0F0F0F0F, 0x00FF00FF00FF00FF, 0x0000FFFF0000FFFF};
		static const IndexType S[] = {1, 2, 4, 8, 16};

		IndexType x = col_index;

		x = (x | (x << S[4])) & B[4];
		x = (x | (x << S[3])) & B[3];
		x = (x | (x << S[2])) & B[2];
		x = (x | (x << S[1])) & B[1];
		x = (x | (x << S[0])) & B[0];

		return (x | row_result);
	}

	virtual IndexType get_index(RowMajorIndexType row_index, RowMajorIndexType col_index) const {
		static const IndexType B[] = {0x5555555555555555, 0x3333333333333333,
			0x0F0F0F0F0F0F0F0F, 0x00FF00FF00FF00FF, 0x0000FFFF0000FFFF};
		static const IndexType S[] = {1, 2, 4, 8, 16};

		//move the lower 32 bit of row_index and col_index into a interleaving bit result
		//must ensure the row_index and col_index is less than 2^32 - 1, actually thus reasonable
		IndexType x = col_index;
		IndexType y = row_index;

		x = (x | (x << S[4])) & B[4];
		x = (x | (x << S[3])) & B[3];
		x = (x | (x << S[2])) & B[2];
		x = (x | (x << S[1])) & B[1];
		x = (x | (x << S[0])) & B[0];

		y = (y | (y << S[4])) & B[4];
		y = (y | (y << S[3])) & B[3];
		y = (y | (y << S[2])) & B[2];
		y = (y | (y << S[1])) & B[1];
		y = (y | (y << S[0])) & B[0];

		return (x | (y << 1));
	}

	virtual RowMajorPoint get_origin_index(IndexType index) const {
		size_t row = 0, col = 0;
		IndexType loc = 0;
		while(index) {
			if(index & ONE)	col |= markAll[loc];
			if((index >>= 1) & ONE)	row |= markAll[loc];
			index >>= 1;
			++loc;
		}
		return RowMajorPoint(row, col);
	}

	virtual IndexType get_max_index() const
	{
		return get_index(m_row - 1, m_col - 1);
	}

	virtual std::string get_index_method_name() const
	{
		return std::string("ZOrderIndex");
	}
};

#endif