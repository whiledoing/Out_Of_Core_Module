#ifndef _GIANT_HIERARCHICAL_INTERFACE_H
#define _GIANT_HIERARCHICAL_INTERFACE_H
#include "GiantImageInterface.h"

template<typename T>
class HierarchicalInterface
{
public:
	HierarchicalInterface(int rows, int cols, int mini_rows, int mini_cols);

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
     * @brief ： 设置最小分辨率
     * @para rows, cols : 图像的大小
     * @para mini_rows, mini_cols : 图像分辨率
     */
	void set_minimal_resolution(int rows, int cols, int mini_rows, int mini_cols);

public:

	/*
	 *	@brief : set the image current level before any access to the hierarchical image data
	 */
	virtual void set_current_level(int level) 
	{
		BOOST_ASSERT(level >= 0);
		current_level = level;
	}

	virtual size_t get_current_level() const 
	{
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

protected:
	size_t get_least_order_number(size_t number)
	{
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

template<typename T>
HierarchicalInterface<T>::HierarchicalInterface(int rows, int cols, int mini_rows, int mini_cols)
{
	set_minimal_resolution(rows, cols, mini_rows, mini_cols);
	
	/* init to the max int size */
	current_level = UINT_MAX;
}

template<typename T>
void HierarchicalInterface<T>::set_minimal_resolution(int rows, int cols, int mini_rows, int mini_cols)
{
	BOOST_ASSERT(m_mini_rows >= 0 && m_mini_cols >= 0 && rows >= mini_rows && cols >= mini_cols);

	/* ensure the mini_rows and mini_cols not zero to insure the correctness of the division */
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

#endif