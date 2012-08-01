#ifndef _GIANTDISK_IMAGE_H
#define _GIANTDISK_IMAGE_H 

#include "BasicType.h"
#include "DiskBigImageInterface.h"
#include "Lru.hpp"
#include "IndexMethod.hpp"

/* filesystem part */
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>

#include <boost/lexical_cast.hpp>

#include <vector>

template<typename T>
class DiskBigImage : public DiskBigImageInterface<T>
{

public:
	/* Derived from DiskBigImageInterface */
	virtual bool get_pixels_by_level(int level, int &start_row, int &start_col, int &rows, int &cols, std::vector<T> &vec);
	virtual bool set_pixel_by_level(int level, int start_row, int start_col, int rows, int cols, const std::vector<T> &vec);

	virtual bool set_current_level(int level);
	virtual size_t get_current_level() const; 

	virtual size_t get_current_level_image_rows() const; 
	virtual size_t get_current_level_image_cols() const;

	virtual bool set_file_cache_number(int _file_cache_number);

	virtual size_t get_max_image_level() const;

public:

	/*
	 *	@brief : get the minimal_image size, also can get the size info by calling such functions
	 *	1) set_current_level(get_max_image_level());
	 *	2) get_current_level_image_rows() && get_current_level_image_cols()
	 */
	inline size_t get_minimal_image_rows() const; 
	inline size_t get_minimal_image_cols() const;

protected:
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

	/*
	 *	@brief : read out the [front, tail) range cells in the index_info_vector, and write
	 *	the data into the data_vector, which keeps the row-major format image data that
	 *	will be the result of get_pixels_by_level() function
	 */
	bool read_from_index_range(size_t front, size_t tail, ZOrderIndex::IndexType start_index, 
		const std::vector<DataIndexInfo> &index_info_vector, std::vector<T> &data_vector);

	/* 
	 * @brief : checks the parameter invalidation before calling the get_pixels_by_level() function
	 */
	bool check_para_validation(int level, int start_row, int start_col, int rows, int cols);

	/*
	 * @brief : load the image head file before load image
	 */
	bool load_image_head_file(const char *file_name);

	/*
	 *	@brief : get he image data path form the file_name(*.bigimage)
	 */
	void set_image_data_path(const char * file_name);

protected:
	DiskBigImage() {}

	template<typename T>
	friend boost::shared_ptr<DiskBigImage<T> > load_image(const char *file_name);

protected:
	Size img_size;

	/* the number of cells in individual file*/
	int64 file_node_size;

	/* the shift number of file_node_size */
	int64 file_node_shift_num;

	/* the shared_ptr of index method */
	boost::shared_ptr<IndexMethodInterface> index_method;

	/* size of the minimum size image */
	size_t m_mini_rows, m_mini_cols;

    /*
	 * the max image hierarchical level
	 * level = 0 : means no scale
	 * level = n : means scale 1/2^n of original image
	 */
	size_t m_max_level;

	/* the image current level */
	Size img_current_level_size;

	/* the current level for reading and writing */
	size_t m_current_level;

	/* the path of the image data in filesystem */
	std::string img_data_path;

	/* the specific level image data path */
	std::string img_level_data_path;

	/* the number of cache file number for lru manager */
	size_t file_cache_number;

	/* the lru image files manager */
	ImageFileLRU<Vec3b> lru_image_files;
};

template<typename T>
inline size_t DiskBigImage<T>::get_minimal_image_rows() const 
{
	return m_mini_rows;
}

template<typename T>
inline size_t DiskBigImage<T>::get_minimal_image_cols() const 
{
	return m_mini_cols;
}

#endif