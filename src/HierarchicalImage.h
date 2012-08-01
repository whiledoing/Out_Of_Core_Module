#ifndef _HIERARCHICAL_IMAGE_H
#define _HIERARCHICAL_IMAGE_H

#include <string>

#include "BlockwiseImage.h"
#include "Lru.hpp"
#include "IndexMethod.hpp"

#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
namespace bf = boost::filesystem3;

template<typename T, size_t memory_usage = 64>
class HierarchicalImage: public BlockwiseImage<T, memory_usage>
{
public:
	HierarchicalImage(size_t rows, size_t cols, size_t mini_rows, size_t mini_cols,
		boost::shared_ptr<IndexMethodInterface> method = boost::shared_ptr<IndexMethodInterface>());
	virtual ~HierarchicalImage();

/* Derived from BlockwiseImage */

	virtual bool write_image(const char* file_name);
	virtual bool write_image(const std::string &file_name);
	virtual bool save_mini_image();

/* specific method */

public:

	/*
	 * @brief : set the number for writing image files concurrently 
	 */
	inline void set_mutliply_ways_writing_number(size_t number);

protected:

	/*
	 * @brief : set the image data path from the big image file name
				for example : the file_name is /a/x.bigimage then the data_path is /a/x/
	 */
	inline void set_image_data_path(const char *file_name); 

private:
	struct DataIndexInfo
	{
		int64 index;			//keeps the original index (in row-major order)
		int64 zorder_index;	//keeps the zorder index according to the row-major index

		friend inline bool operator< (const DataIndexInfo& lhs, const DataIndexInfo& rhs) 
		{
			return (lhs.zorder_index < rhs.zorder_index);
		}
	};

protected:
	bool write_image_head_file(const char *file_name);
	bool write_image_inner_loop(size_t start_level, size_t merge_number, const bf::path &data_path, const int64 &file_number);
	//bool load_image_head_file(const char *file_name);

protected:
	/* the number for writing image data files in concurrently */
	size_t concurrent_number;

	/* the data of the image file data */
	std::string img_data_path;

	/* the image current level */
	Size img_current_level_size;
};

template<typename T, size_t memory_usage>
bool HierarchicalImage<T, memory_usage>::save_mini_image()
{
	return true;
}

template<typename T, size_t memory_usage>
inline void HierarchicalImage<T, memory_usage>::set_mutliply_ways_writing_number(size_t number) {
	size_t max_number = get_max_image_level() + 1;
	concurrent_number = (number > max_number) ? max_number : number;
} 

template<typename T, size_t memory_usage>
inline void HierarchicalImage<T, memory_usage>::set_image_data_path(const char * file_name) 
{
	/* save the img_data_path */
	bf::path file_path = file_name;
	img_data_path = (file_path.parent_path() / file_path.stem()).generic_string();
}

/*
 * @brief : return the hierarchical image by memroy_usage, maximum support 4G
 * @para memory_usage : the memory usage of the main memory
 * @para method : the index method shared_ptr object
 * @para rows, cols : the image size
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

/*
 * @brief : return the hierarchical image loaded from file_name
 * @para memory_usage : the memory usage of the main memory
 * @para file_name : the file name of the big image
 */
template<typename T>
boost::shared_ptr<GiantImageInterface<T> > load_hierarchical_image_by_meomory_usage(unsigned memory_usage, 
	const char *file_name)
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
		return HierarchicalImage<T, 8>::load_image(file_name);
	case 16:
		return HierarchicalImage<T, 16>::load_image(file_name);
	case 32:
		return HierarchicalImage<T, 32>::load_image(file_name);
	case 64:
		return HierarchicalImage<T, 64>::load_image(file_name);
	case 128:
		return HierarchicalImage<T, 128>::load_image(file_name);
	case 256:
		return HierarchicalImage<T, 256>::load_image(file_name);
	case 512:
		return HierarchicalImage<T, 512>::load_image(file_name);
	case 1024:
		return HierarchicalImage<T, 1024>::load_image(file_name);
	case 2048:
		return HierarchicalImage<T, 2048>::load_image(file_name);
	case 4096:
		return HierarchicalImage<T, 4096>::load_image(file_name);
	default:
		return HierarchicalImage<T, 64>::load_image(file_name);
	}
}

#endif