#include <iostream>
#include <iomanip>
#include <algorithm>
#include <vector>
#include <boost/assert.hpp>
#include <string>

#include "IndexMethod.hpp"

using namespace std;

/* test the block index method, input the rows, cols, blockrows, blockcols for testing */
bool test_block_index(int argc, char **argv)
{
	if(argc < 5) {
		cout << "Usage : [row count] [col count] [block row size n(by 2^n)] [block col size n(by 2^n)]" << endl;
		return false;
	}

	typedef Block2DIndex::IndexType IndexType;
	typedef Block2DIndex::RowMajorIndexType RowMajorType;

	IndexType rowCount = atoi(argv[1]);
	IndexType colCount = atoi(argv[2]);
	IndexType blockRowOrderSize = atoi(argv[3]);
	IndexType blockColOrderSize = atoi(argv[4]);

	Block2DIndex blockObject(rowCount, colCount, blockRowOrderSize, blockColOrderSize);

	{
		cout << "method one : using get_index method : " << endl;
		cout << "*******************************************************" << endl;
		IndexType totalSize = blockObject.get_max_index();
		int *vec = new int[totalSize];
		std::generate(vec, vec + totalSize, []()->int {
			return -1;
		});

		for(RowMajorType i = 0; i < rowCount; ++i) {
			for(RowMajorType j = 0; j < colCount; ++j) {
				vec[blockObject.get_index(i, j)] = i*colCount + j;
			}
		}

		IndexType blockSize = blockObject.getBlockTotalSize();

		IndexType blockColSize = blockObject.getBlockColSize();
		IndexType blockIndex = 0;

		cout << "the after block : " << endl;
		cout << "------------------------------------------------------" << endl;
		for(IndexType index = 0; index < totalSize;) {
			cout << setw(8) << vec[index++];

			//meet the end of the small block line
			if(index % blockColSize == 0) cout << endl;

			if(index % blockSize == 0) cout << "------------------------------------------------------" << endl;
		}
		cout << endl;

		cout << "get row major index : " << endl;
		cout << "------------------------------------------------------" << endl;
		for(IndexType index = 0; index < totalSize;) {
			RowMajorPoint point = blockObject.get_origin_index(index++);
			cout << setw(8) << "(" << point.row << "," << point.col << ")";
			if(index % blockColSize == 0) cout << endl;
			if(index % blockSize == 0) cout << "------------------------------------------------------" << endl;
		}
		cout << endl;
	}

	{
		cout << "method two : using get_index_by_row_index method : " << endl;
		cout << "*******************************************************" << endl;
		IndexType totalSize = blockObject.get_max_index();
		int *vec = new int[totalSize];
		std::generate(vec, vec + totalSize, []()->int {
			return -1;
		});

		for(RowMajorType i = 0; i < rowCount; ++i) {
			IndexType row_result = blockObject.get_row_result(i);
			for(RowMajorType j = 0; j < colCount; ++j) {
				vec[blockObject.get_index_by_row_result(row_result, j)] = i*colCount + j;
			}
		}

		IndexType blockSize = blockObject.getBlockTotalSize();

		IndexType blockColSize = blockObject.getBlockColSize();
		IndexType blockIndex = 0;

		cout << "the after block : " << endl;
		cout << "------------------------------------------------------" << endl;
		for(IndexType index = 0; index < totalSize;) {
			cout << setw(8) << vec[index++];

			//meet the end of the small block line
			if(index % blockColSize == 0) cout << endl;

			if(index % blockSize == 0) cout << "------------------------------------------------------" << endl;
		}
		cout << endl;

		cout << "get row major index : " << endl;
		cout << "------------------------------------------------------" << endl;
		for(IndexType index = 0; index < totalSize;) {
			RowMajorPoint point = blockObject.get_origin_index(index++);
			cout << setw(8) << "(" << point.row << "," << point.col << ")";
			if(index % blockColSize == 0) cout << endl;
			if(index % blockSize == 0) cout << "------------------------------------------------------" << endl;
		}
		cout << endl;
	}
	return true;
}
template<typename T, typename RowMajorType>
void printTwoArrayVector(T array, RowMajorType row, RowMajorType col, string arrayInfo = "", ostream& os = cout) {
	os << arrayInfo << endl;
	for(RowMajorType i = 0; i < row; ++i) {
		for(RowMajorType j = 0; j < col; ++j) {
			os << setw(8) << array[i*col + j];
		}
		os << endl;
	}
	os << endl;
}

/* test zorder index, input the rows and cols for indexing */
typedef ZOrderIndex ZOrderIndexType;
bool test_zorder_index(int argc, char **argv)
{
	if(argc < 3) {
		cout << "Usage : [row] [col] " << endl;
		return false;
	}

	size_t row = atoi(argv[1]);
	size_t col = atoi(argv[2]);

	//just insure the row and col are even
	//BOOST_ASSERT((row & 1) == 0 && (col & 1) == 0);

	typedef std::vector<int> VecType;
	typedef ZOrderIndexType::IndexType IndexType;

	cout << "method one : using get_index method : " << endl;
	cout << "*******************************************************" << endl;
	{
		//save the original data
		VecType vec(row * col);
		for(IndexType i = 0; i < vec.size(); ++i) vec[i] = i;

		ZOrderIndexType zorder(row, col);
		//the after Z-order index data
		IndexType maxIndex = zorder.get_max_index();
		VecType newVec(maxIndex + 1);

		IndexType result, rowResult;
		cout << "zorder index result" << endl;
		for(size_t i = 0; i < row; ++i) {
			rowResult = zorder.get_row_result(i);
			for(size_t j = 0; j < col; ++j) {
				result = zorder.get_index_by_row_result(rowResult, j);
				cout << setw(8) << result;

				newVec[result] = vec[i*col + j];
			} //end for of col
			cout << endl;
		} // end for of row

		printTwoArrayVector(newVec, row, col, "zorder index array");

		VecType testVec(col * row);
		for(IndexType i = 0; i < newVec.size(); ++i) {
			RowMajorPoint point = zorder.get_origin_index(i);
			size_t x = point.row;
			size_t y = point.col;
			if(x < row && y < col) testVec[x*col + y] = newVec[i];
		}

		printTwoArrayVector(testVec, row, col, "get back vector");

		if(std::equal(vec.begin(), vec.end(), testVec.begin()))
			cout << "the result is correct" << endl;
		else
			cout << "the result is not correct" << endl;
	}

	cout << "method two : using get_index_by_row_index method : " << endl;
	cout << "*******************************************************" << endl;
	{
		//save the original data
		VecType vec(row * col);
		for(IndexType i = 0; i < vec.size(); ++i) vec[i] = i;

		ZOrderIndexType zorder(row, col);
		//the after Z-order index data
		IndexType maxIndex = zorder.get_max_index();
		VecType newVec(maxIndex + 1);

		IndexType result, rowResult;
		cout << "zorder index result" << endl;
		for(size_t i = 0; i < row; ++i) {
			for(size_t j = 0; j < col; ++j) {
				result = zorder.get_index(i, j);
				cout << setw(8) << result;

				newVec[result] = vec[i*col + j];
			} //end for of col
			cout << endl;
		} // end for of row

		printTwoArrayVector(newVec, row, col, "zorder index array");

		VecType testVec(col * row);
		for(IndexType i = 0; i < newVec.size(); ++i) {
			RowMajorPoint point = zorder.get_origin_index(i);
			size_t x = point.row;
			size_t y = point.col;
			if(x < row && y < col) testVec[x*col + y] = newVec[i];
		}

		printTwoArrayVector(testVec, row, col, "get back vector");

		if(std::equal(vec.begin(), vec.end(), testVec.begin()))
			cout << "the result is correct" << endl;
		else
			cout << "the result is not correct" << endl;
	}

	return true;
}