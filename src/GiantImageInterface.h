#ifndef _GIANT_IMAGE_INTERFACE_H
#define _GIANT_IMAGE_INTERFACE_H

#include "ImageInterface.h"
#include "IndexMethodInterface.h"
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

template<typename T>
class GiantImageInterface: public ImageInterface<T>
{
public:

	/*
	 *	@para method : the index method object for internal indexing
	 */
	GiantImageInterface(boost::shared_ptr<IndexMethodInterface> method)
		: index_method(method)
	{
		file_node_size = 1024*1024;		//default has 1M image cells in one file
		file_node_shift_num = 20;
	}

	/*
     * @brief : 设置保存大图像数据的文件节点的大小（分割成多个小文件处理）
     * @para size : 文件大小（以Byte为单位）
     */
	void set_file_node_size(int64 size)
	{
		/* make size the multiple of image cell's type */
		size = int64(std::ceil((double)(size) / sizeof(T)));
		while(size & (size - 1)) {
			size &= (size - 1);
		}

		file_node_size = size;

		int64 count = 0;
		while(size) {
			++count;
			size >>= 1;
		}

		/* shift number : size == 2^(file_node_shift_num) */
		file_node_shift_num = count - 1;
		file_node_shift_num = (file_node_shift_num < 0) ? 0 : file_node_shift_num;
	}

	int64 get_file_node_size() const
	{
		return file_node_size * sizeof(T);
	}

	/*
	 *	@brief : set the new index object for internal indexing
	 *	@para method : the index object
	 */
	void set_index_method(boost::shared_ptr<IndexMethodInterface> method)
	{
		index_method = method;
	}

	boost::shared_ptr<IndexMethodInterface> get_index_method() const
	{
		return index_method;
	}

	/*
	 *	@brief : get the element by the one dimension index
	 *	@para index : one dimension index
	 */
	virtual T& at(IndexMethodInterface::IndexType index) = 0;
	virtual const T& at(IndexMethodInterface::IndexType index) const = 0;

protected:

	/* the number of cells in individual file*/
	int64 file_node_size;

	/* the shift number of file_node_size */
	int64 file_node_shift_num;

	/* the shared_ptr of index method */
	boost::shared_ptr<IndexMethodInterface> index_method;
};

#endif