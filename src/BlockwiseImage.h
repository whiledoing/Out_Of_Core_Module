#ifndef _BLOCKWISE_IMAGE_H
#define _BLOCKWISE_IMAGE_H

#pragma warning(disable:4307 4244 4250 4290 4800 4018 4204)

#include "GiantImageInterface.h"
#include <string>

/* stxxl part */
#include <stxxl.h>
#ifdef NDEBUG
#pragma comment(lib, "libstxxl_release.lib")
#else
#pragma comment(lib, "libstxxl_debug.lib")
#endif

template<typename T, unsigned memory_usage = 64>
class BlockwiseImage: public GiantImageInterface<T>
{
public:
	/*
	 * @para rows, cols : the image size
	 * @para method : the index method shared_ptr object(default is zorder index method)
	 */
	BlockwiseImage(size_t rows, size_t cols, boost::shared_ptr<IndexMethodInterface> method = boost::shared_ptr<IndexMethodInterface>());

	virtual ~BlockwiseImage();

	virtual bool init(size_t rows, size_t cols);
	virtual bool reset();

	static boost::shared_ptr<BlockwiseImage<T, memory_usage> > load_image(const char *file_name);
	static boost::shared_ptr<BlockwiseImage<T, memory_usage> > load_image(const std::string &file_name);

	virtual bool write_image(const char *file_name);
	virtual bool write_image(const std::string &file_name);

	virtual T& get_pixel(int row, int col);
	virtual const T& get_pixel(int row, int col) const;
	virtual T& operator() (int row, int col);
	virtual const T& operator() (int row, int col) const;

	virtual bool get_pixel(int min_row, int max_row, int min_col, int max_col, T* ptr) const;
	virtual bool set_pixel(int min_row, int max_row, int min_col, int max_col, const T* ptr);

	virtual bool get_pixel(int min_row, int max_row, int min_col, int max_col, std::vector<T> &data) const;
	virtual bool set_pixel(int min_row, int max_row, int min_col, int max_col, const std::vector<T> &data);

	virtual T& at(IndexMethodInterface::IndexType index);
	virtual const T& at(IndexMethodInterface::IndexType index) const;

protected:
	bool write_image_head_file(const char* file_name);
	bool load_image_head_file(const char* file_name);

protected:
	/*
	 *	4 means a page has 4 blocks
	 *	lru_pages<n> : n means the number of pages in memory
	 *	each block is 2M , thus total memroy usage = 8M * (number of pages) , thus why right shift 3 bit
	 */
	typedef stxxl::vector<T, 4, stxxl::lru_pager<(memory_usage >> 3)> >  ContainerType;
	ContainerType img_container;

	static const std::string str_extension;
};

template<typename T, unsigned memory_usage>
const std::string BlockwiseImage<T, memory_usage>::str_extension = ".bigimage";

/*
 * @brief : return the block wise image by memroy_usage, maximum support 4G
 * @para memory_usage : the memory usage of the main memory
 * @para method : the index method shared_ptr object
 * @para rows, cols : the image size
 */
template<typename T>
boost::shared_ptr<GiantImageInterface<T> > get_block_wise_image_by_meomory_usage(unsigned memory_usage,
	size_t rows, size_t cols, boost::shared_ptr<IndexMethodInterface> method = boost::shared_ptr<IndexMethodInterface>())
{
	/* ensure memory_usage is bigger than 8 */
	memory_usage = (memory_usage < 8) ? 8 : memory_usage;

	/* make memory_usage = 2^order (order >=3 && order <= 12) */
	while(memory_usage & (memory_usage - 1)) {
		memory_usage &= (memory_usage - 1);
	}

	switch(memory_usage)
	{
	case 8:
		return boost::make_shared<BlockwiseImage<T, 8> >(rows, cols, method);
	case 16:
		return boost::make_shared<BlockwiseImage<T, 16> >(rows, cols, method);
	case 32:
		return boost::make_shared<BlockwiseImage<T, 32> >(rows, cols, method);
	case 64:
		return boost::make_shared<BlockwiseImage<T, 64> >(rows, cols, method);
	case 128:
		return boost::make_shared<BlockwiseImage<T, 128> >(rows, cols, method);
	case 256:
		return boost::make_shared<BlockwiseImage<T, 256> >(rows, cols, method);
	case 512:
		return boost::make_shared<BlockwiseImage<T, 512> >(rows, cols, method);
	case 1024:
		return boost::make_shared<BlockwiseImage<T, 1024> >(rows, cols, method);
	case 2048:
		return boost::make_shared<BlockwiseImage<T, 2048> >(rows, cols, method);
	case 4096:
		return boost::make_shared<BlockwiseImage<T, 4096> >(rows, cols, method);
	default:
		return boost::make_shared<BlockwiseImage<T, 64> >(rows, cols, method);
	}
}
#endif