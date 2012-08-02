#ifndef _INDEX_METHOD_INTERFACE_H
#define _INDEX_METHOD_INTERFACE_H

#include "BasicType.h"
#include <string>

/**
 * @class IndexMethodInterface	IndexMethodInterface.h
 * 
 * @brief The basic functions of the indexing method.
 */

class IndexMethodInterface
{
public:

	/** the type of all the index */
	typedef int64 IndexType;

	/** the type of row-major like index */
	typedef size_t RowMajorIndexType;

public:

	/**
	 *	@brief get the index of row-major index (row, col)
	 */
	virtual IndexType get_index(RowMajorIndexType row, RowMajorIndexType col) const = 0;

	/**
	 *	@brief get the original row-major index
	 *	@param index the index calculated from get_index() method
	 *	@return the row-major index (row, col) format
	 */
	virtual RowMajorPoint get_origin_index(IndexType index) const = 0;

	/**
	 *	@brief get the row result from just the row index (for using row-major like loop).
	 *	
	 *	<pre>
	 *	Using this method for some kind of optimization when using row-major like loop 
	 *	for example:	
	 *	for(size_t row = 0; row < rows; ++row) { 
	 *		IndexType row_result = get_row_result(row);
	 *		for(size_t col = 0; col < cols; ++col) { 
	 *			IndexType index = get_index_by_row_result(row_result, col); 
	 *		} 
	 *	} 
	 *	</pre>
	 *	
	 *	@param row_index the index of row in the row-major format
	 *	@return the row result
	 */
	virtual IndexType get_row_result(RowMajorIndexType row_index) const = 0;

	/**
	 *	@brief get the index from row result and col index
	 *	@param row_result the result get from the get_row_result() method
	 *	@param col_index the index of col in the row-major format
	 *	@return the index result combined both the row and col
	 */
	virtual IndexType get_index_by_row_result(IndexType row_result, RowMajorIndexType col_index) const = 0;

	/**
	 *	@brief get the maximum index, often used to ensure the size of the container
	 *	@return the maximum index get from the this indexing method
	 */
	virtual IndexType get_max_index() const = 0;

	/**
	 *	@brief get the index method name for identify different method
	 */
	virtual std::string get_index_method_name() const = 0;
};

#endif