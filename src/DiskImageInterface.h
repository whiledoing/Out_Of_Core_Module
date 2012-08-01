#ifndef _GIANT_HIERARCHICAL_INTERFACE_H
#define _GIANT_HIERARCHICAL_INTERFACE_H
#include "GiantImageInterface.h"
#include "UtlityFunc.h"

template<typename T>
class DiskImageInterface
{
public:
	/* pure virtual function */

	/*
	* @brief : 获取制定等级下区域图像数据
	* @para start_row, start_col : 区域点的左上角坐标
	* @para rows, cols : 区域的行和列的数目
	* @para level ： 图像的分辨率等级
	* @para vec : 图像数据
	* @return : 是否获取成功
	*/
	virtual bool get_pixels_by_level(int level, int &start_row, int &start_col,
		int &rows, int &cols, std::vector<T> &vec) = 0;
	virtual bool set_pixel_by_level(int level, int start_row, int start_col, int rows, int cols, const std::vector<T> &vec) = 0;

    /*
	 *	@brief : get the current level image rows after calling the set_current_level function
	 */
	virtual size_t get_current_level_image_rows() const = 0;

	/*
	 *	@brief : get the current level image cols after calling the set_current_level function
	 */
	virtual size_t get_current_level_image_cols() const = 0;

    /*
	 *	@brief : set the image current level before any access to the hierarchical image data
	 */
	virtual bool set_current_level(int level) = 0;
	virtual size_t get_current_level() const = 0;

    /*
	 *	@brief : set the file cache number when loading image data since the image from disk
	 *	is too large so when load data from disk, using several file caches to save the image data for
	 *	improving I/O.
	 */
	virtual bool set_file_cache_number(int _file_cache_number) = 0;

	/*
	 *	@brief : get the maximum image level thus the minimal size image's scale level
	 */
	virtual size_t get_max_image_level() const = 0;
};

#endif