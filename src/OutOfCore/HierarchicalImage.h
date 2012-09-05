#ifndef _HIERARCHICAL_IMAGE_H
#define _HIERARCHICAL_IMAGE_H

#include <string>

#include "BlockwiseImage.h"

/**
 * @class HierarchicalImage HierarchicalImage.h
 *
 * @brief Derived from BlockwiseImage, support the big image operation, can save the image data in a hierarchical way.
 *
 * HierarchicalImage can support very big image processing and storing, different with the BlockwiseImage, HierarchicalImage 
 * can write the image into different levels, but the BlockwiseImage can only save the full size of the image.
 *
 * @tparam T The type of the image cell
 * @tparam memory_usage The memory usage used as a I/O cache in the main memory, and it must be bigger than 8 (in the unit of M).
 *		 By default, memory_usage is set to 64M
 */

template<typename T, size_t memory_usage = 64>
class HierarchicalImage: public BlockwiseImage<T, memory_usage>
{
public:

	/**
	 * @param rows the image total rows
	 * @param cols the image total cols
	 * @param mini_rows the minimum size image rows
	 * @param mini_cols the minimum size image cols
	 * @param method : the index method shared_ptr object(default is zorder index method)
	 */
	HierarchicalImage(size_t rows, size_t cols, size_t mini_rows, size_t mini_cols,
		boost::shared_ptr<IndexMethodInterface> method = boost::shared_ptr<IndexMethodInterface>());

	virtual ~HierarchicalImage();

/* Derived from BlockwiseImage */

	virtual bool write_image(const char* file_name);
	virtual bool write_image(const std::string &file_name);
	virtual bool save_mini_image(const char* file_name);

/* specific method */

public:

	/**
	 * @brief set the number for writing image files concurrently
	 * 
	 * If the image has n levels(thus max level is n-1), then when write the image data in a hierarchical way into disk
	 * the maximum concurrent number is n.
	 */
	inline void set_mutliply_ways_writing_number(size_t number);

protected:

	/**
	 * @brief set the image data path from the big image file name. 
	 *
	 * For example : the file_name is /a/x.bigimage then the data_path is /a/x/
	 */
	inline void set_image_data_path(const char *file_name); 

protected:

	/**
	 *	@brief write the image head file
	 */
	bool write_image_head_file(const char *file_name);

	/** 
	 * @brief write the start_level image data in the write image inner loop
	 */
	bool write_image_inner_loop(size_t start_level, size_t merge_number, 
        const boost::filesystem::path &data_path, const int64 &file_number);

protected:
	/** the number for writing image data files in concurrently */
	size_t concurrent_number;

	/** the data of the image file data */
	std::string img_data_path;

	/** the image current level */
	Size img_current_level_size;
};

template<typename T, size_t memory_usage>
inline void HierarchicalImage<T, memory_usage>::set_mutliply_ways_writing_number(size_t number) {
	size_t max_number = get_max_image_level() + 1;
	concurrent_number = (number > max_number) ? max_number : number;
} 

template<typename T, size_t memory_usage>
inline void HierarchicalImage<T, memory_usage>::set_image_data_path(const char * file_name) 
{
	namespace bf = boost::filesystem;

	/* save the img_data_path */
	bf::path file_path = file_name;
	img_data_path = (file_path.parent_path() / file_path.stem()).generic_string();
}

/**
 * @brief return the hierarchical image by memroy_usage, maximum support 4G
 * @param memory_usage the memory usage of the main memory
 * @param method the index method shared_ptr object
 * @param rows the image total rows
 * @param cols the image total cols
 * @param mini_rows the minimum size image rows
 * @param mini_cols the minimum size image cols
 * @return the shared_ptr of the GiantImageInterface object which indeed is a HierarchicalImage object
 * @relates HierarchicalImage
 */
template<typename T>
boost::shared_ptr<GiantImageInterface<T> > get_hierarchical_image_by_meomory_usage(unsigned memory_usage,
	size_t rows, size_t cols, size_t mini_rows, size_t mini_cols,
	boost::shared_ptr<IndexMethodInterface> method = boost::shared_ptr<IndexMethodInterface>())
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
		return boost::make_shared<HierarchicalImage<T, 8> >(rows, cols, mini_rows, mini_cols, method);
	case 16:
		return boost::make_shared<HierarchicalImage<T, 16> >(rows, cols, mini_rows, mini_cols, method);
	case 32:
		return boost::make_shared<HierarchicalImage<T, 32> >(rows, cols, mini_rows, mini_cols, method);
	case 64:
		return boost::make_shared<HierarchicalImage<T, 64> >(rows, cols, mini_rows, mini_cols, method);
	case 128:
		return boost::make_shared<HierarchicalImage<T, 128> >(rows, cols, mini_rows, mini_cols, method);
	case 256:
		return boost::make_shared<HierarchicalImage<T, 256> >(rows, cols, mini_rows, mini_cols, method);
	case 512:
		return boost::make_shared<HierarchicalImage<T, 512> >(rows, cols, mini_rows, mini_cols, method);
	case 1024:
		return boost::make_shared<HierarchicalImage<T, 1024> >(rows, cols, mini_rows, mini_cols, method);
	case 2048:
		return boost::make_shared<HierarchicalImage<T, 2048> >(rows, cols, mini_rows, mini_cols, method);
	case 4096:
		return boost::make_shared<HierarchicalImage<T, 4096> >(rows, cols, mini_rows, mini_cols, method);
	default:
		return boost::make_shared<HierarchicalImage<T, 64> >(rows, cols, mini_rows, mini_cols, method);
	}
}

/**
 * @example WriteHierarchicalImage.cpp
 * This an example of how to write the hierarchical image into the file system.
 */

#endif