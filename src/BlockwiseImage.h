#ifndef _BLOCKWISE_IMAGE_H
#define _BLOCKWISE_IMAGE_H

#pragma warning(disable:4307 4244 4250 4290 4800 4018 4204)

#include "GiantImageInterface.h"
#include "UtlityFunc.h"
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
	BlockwiseImage(int rows, int cols, int mini_rows, int mini_cols, 
		boost::shared_ptr<IndexMethodInterface> method = boost::shared_ptr<IndexMethodInterface>());

	virtual ~BlockwiseImage();

	virtual bool init(int rows, int cols);
	virtual bool reset();

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

	size_t get_minimal_image_rows() const
	{
		return m_mini_rows;
	}

	size_t get_minimal_image_cols() const
	{
		return m_mini_cols;
	}

	size_t get_max_image_level() const 
	{
		return m_max_level;
	}

protected:
	bool write_image_head_file(const char* file_name);

	void set_minimal_resolution(int rows, int cols, int mini_rows, int mini_cols)
	{
		BOOST_ASSERT(m_mini_rows >= 0 && m_mini_cols >= 0 && rows >= mini_rows && cols >= mini_cols);

		/* ensure the mini_rows and mini_cols not zero to insure the correctness of the division */
		if(mini_rows == 0)	mini_rows = 1;
		if(mini_cols == 0)	mini_cols = 1;

		size_t level_row = rows / mini_rows, level_col = cols / mini_cols;
		level_row = get_least_order_number(level_row);
		level_col = get_least_order_number(level_col);

		/* ensure the smallest image (the max scale level) is not less than mini_rows or mini_cols which user specified */
		m_max_level = min(level_row, level_col);

		/* recalculate the mini_rows and mini_cols */
		m_mini_rows = std::ceil((double)(rows) / (1 << m_max_level));
		m_mini_cols = std::ceil((double)(cols) / (1 << m_max_level));
	}

	bool save_mini_image() {
		//TODO : get the max level image to save as a jpg file
		//if(!get_pixels_by_level(m_max_level, start_rows, start_cols, rows, cols, img_data)) {
		//	cerr << "get the maximum image failure" << endl;
		//	return false;
		//}

		//cv::Mat result_image(rows, cols, CV_8UC3, img_data.data());

		///* convert the RGB format to opencv BGR format */
		//cv::cvtColor(result_image, result_image, CV_RGB2BGR);

		//bf::path file_path(file_name);
		//string result_image_name = (file_path.parent_path() / (file_path.stem().generic_string() + ".jpg")).generic_string();
		//cv::imwrite(result_image_name, result_image);
		
  //      const ContainerType &c_img_container = img_container;

		//std::vector<T> img_data;
		//std::vector<T> img_zorder_data;
		//IndexMethodInterface::IndexType total_size, file_cell_size, delta_count;
		//total_size = c_img_container.size();
		//file_cell_size = total_size >> (2*m_max_level);
		//delta_count = 1 << (2*m_max_level);
		//for(IndexMethodInterface::IndexType i = 0, count = 0; i < file_cell_size; ++i, count += delta_count) {
		//	img_zorder_data[i] = c_img_container[count];
		//}
		//for(size_t i = 0; i <)
		return true;
	}

protected:
	/*
	 *	4 means a page has 4 blocks
	 *	lru_pages<n> : n means the number of pages in memory
	 *	each block is 2M , thus total memroy usage = 8M * (number of pages) , thus why right shift 3 bit
	 */
	typedef stxxl::vector<T, 4, stxxl::lru_pager<(memory_usage >> 3)> >  ContainerType;
	ContainerType img_container;

	static const std::string str_extension;

	size_t m_mini_rows, m_mini_cols;
	size_t m_max_level;
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