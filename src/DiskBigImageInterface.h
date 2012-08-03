#ifndef _DISK_BIG_IMAGE_INTERFACE_H
#define _DISK_BIG_IMAGE_INTERFACE_H

#include "GiantImageInterface.h"
#include "UtlityFunc.h"

/**
 * @class DiskBigImageInterface DiskBigImageInterface.h
 *
 * @brief The interface for accessing the big image files in the disk that were saved by
 * BlockwiseImage and HierarchicalImage classes.
 *
 * @tparam T The type of the image cells
 */

template<typename T>
class DiskBigImageInterface
{
public:

	/**
	* @brief Get the the specific range area image data from the bigimage files, but first
	* specific which level data you want to get.
	* 
	* @param level The specific level
	* @param start_row The left-corner point row
	* @param start_col The left-corner point col
	* @param rows The row scope of the range, thus the rows get is [start_row, start_row + rows)
	* @param cols The col scope of the range
	* @param vec [Out] Saves the image data get from the bigimage files 
	* @return whether get the data successfully
	*/
	virtual bool get_pixels_by_level(int level, int start_row, int start_col,
		int rows, int cols, std::vector<T> &vec) = 0;
	virtual bool set_pixel_by_level(int level, int start_row, int start_col, int rows, int cols, const std::vector<T> &vec) = 0;

	virtual bool get_pixels_by_level_fast(int level, int &start_row, int &start_col,
		int &rows, int &cols, std::vector<T> &vec) = 0;

    /**
	 *	@brief get the current level image rows after calling the set_current_level function
	 */
	virtual size_t get_current_level_image_rows() const = 0;

	/**
	 *	@brief get the current level image cols after calling the set_current_level function
	 */
	virtual size_t get_current_level_image_cols() const = 0;

    /**
	 *	@brief set the image current level before any access to the hierarchical image data
	 */
	virtual bool set_current_level(int level) = 0;
	virtual size_t get_current_level() const = 0;

    /**
	 *	@brief set the file cache number when loading image data since the image from disk
	 *	is too large so when load data from disk, using several file caches to save the image data for
	 *	improving I/O.
	 */
	virtual bool set_file_cache_number(int _file_cache_number) = 0;

	/**
	 *	@brief get the maximum image level thus the minimal size image's scale level
	 */
	virtual size_t get_max_image_level() const = 0;
};

#endif