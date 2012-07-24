#ifndef _GIANT_HIERARCHICAL_INTERFACE_H
#define _GIANT_HIERARCHICAL_INTERFACE_H
#include "GiantImageInterface.h"

template<typename T>
class HierarchicalInterface
{
public:
	HierarchicalInterface(size_t rows, size_t cols, size_t mini_rows, size_t mini_cols)
	{
		set_minimal_resolution(rows, cols, mini_rows, mini_cols);

		current_level = UINT_MAX;
	}

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

	virtual bool set_pixel_by_level(int level, int min_row, int max_row, int min_col, int max_col, T* ptr) = 0;

	/*
     * @brief ： 设置最小分辨率
     * @para rows, cols : 图像的大小
     * @para mini_rows, mini_cols : 图像分辨率
     */
	void set_minimal_resolution(size_t rows, size_t cols, size_t mini_rows, size_t mini_cols)
	{
		BOOST_ASSERT(rows >= mini_rows && cols >= mini_cols);
		if(mini_rows == 0)	mini_rows = 1;
		if(mini_cols == 0)	mini_cols = 1;

		size_t level_row = rows / mini_rows, level_col = cols / mini_cols;
		level_row = get_least_order_number(level_row);
		level_col = get_least_order_number(level_col);

		/* ensure the smallest image (the max scale level) is not less than mini_rows or mini_cols which user specified */
		m_max_level = std::min(level_row, level_col);

		/* recalculate the mini_rows and mini_cols */
		m_mini_rows = std::ceil((double)(rows) / (1 << m_max_level));
		m_mini_cols = std::ceil((double)(cols) / (1 << m_max_level));
	}

	/*
	 *	@brief : set the image current level before any access to the hierarchical image data
	 */
	virtual void set_current_level(size_t level) {
		current_level = level;
	}

	virtual size_t get_current_level() const {
		return current_level;
	}

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
		
	/*
	 *	@brief : get the current level image rows after calling the set_current_level function
	 */
	virtual size_t get_current_level_image_rows() const = 0;

	/*
	 *	@brief : get the current level image cols after calling the set_current_level function
	 */
	virtual size_t get_current_level_image_cols() const = 0;

protected:
	size_t get_least_order_number(size_t number) {
		while(number & (number - 1)) {
			number &= (number - 1);
		}
		int count = 0;
		while(number) {
			++count;
			number >>= 1;
		}

		return (count < 1) ? 0 : (count - 1);
	}

protected:
	size_t m_mini_rows, m_mini_cols;

	/*
	 * the max image hierarchical level
	 * level = 0 : means no scale
	 * level = n : means scale 1/2^n of original image
	 */
	size_t m_max_level;

	/* the current level for reading and writing */
	size_t current_level;
};

#endif