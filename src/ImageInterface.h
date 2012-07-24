#ifndef _IMAGE_INTERFACE_H
#define _IMAGE_INTERFACE_H

#include <string>
#include <vector>

#include "DataType.h"

template<typename T>
class ImageInterface
{
public:

    /*
     * @brief : 初始化图像的长宽
     * @para width, height ： 长宽
     * @return : 是否返回成功
     */
	virtual bool init(size_t rows, size_t cols) = 0;

    /*
     * @brief : 清空图像数据
     */
	virtual bool reset() = 0;

    /*
     * @brief : 写入图像
     * @para file_name : 图像的名称
     * @return : 是否写入成功
     */
	virtual bool write_image(const char* file_name) = 0;
	virtual bool write_image(const std::string &file_name) = 0;

    /*
     * @brief : 获取坐标（x,y)位置图像数据
     * @para x,y : 图像坐标位置
     * @return : 图像数据的引用
     */
	virtual T& get_pixel(int row, int col) = 0;
	virtual const T& get_pixel(int row, int col) const = 0;
	virtual T& operator() (int row, int col) = 0;
	virtual const T& operator() (int row, int col) const = 0;

    /*
     * @brief : 获取区域图像数据
     * @para min_row, max_row : 区域行的坐标范围
     * @para min_col, max_col : 区域列的坐标范围
     * @para ptr ： 图像数据指针
     * @return : 是否获取成功
     */
	virtual bool get_pixel(int min_row, int max_row, int min_col, int max_col, T* ptr) const = 0;
	virtual bool set_pixel(int min_row, int max_row, int min_col, int max_col, const T* ptr) = 0;

    /*
     * @brief : 获取区域图像数据
     * @para min_row, max_row : 区域行的坐标范围
     * @para min_col, max_col : 区域列的坐标范围
     * @para data ： 图像数据容器
     * @return : 是否获取成功
     */
	virtual bool get_pixel(int min_row, int max_row, int min_col, int max_col, std::vector<T> &data) const = 0;
	virtual bool set_pixel(int min_row, int max_row, int min_col, int max_col, const std::vector<T> &data) = 0;

public:
	size_t get_image_cols() const { return img_size.cols; }
	size_t get_image_rows() const { return img_size.rows; }

protected:
	Size img_size;
};

#endif