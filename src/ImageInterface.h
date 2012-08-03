#ifndef _IMAGE_INTERFACE_H
#define _IMAGE_INTERFACE_H

#include <string>
#include <vector>

#include "BasicType.h"

/**
 * @class ImageInterface ImageInterface.h
 *
 * @brief The Basic Image Interface contains of several basic image processing functions
 *
 * @tparam T The type of the image cells
 */

template<typename T>
class ImageInterface
{
public:

    /**
     * @brief Initialize the image size, and allocate the memory for saving the image
     * @return Whether initialize successfully
     */
	virtual bool init(int rows, int cols) = 0;

    /**
     * @brief Clear the image data
     */
	virtual bool reset() = 0;

    /**
     * @brief Write the image to the disk
     * @param file_name The name of the image file
     * @return Whether write successfully
     */
	virtual bool write_image(const char* file_name) = 0;
	virtual bool write_image(const std::string &file_name) = 0;

    /**
     * @brief Get the pixel of the point (row, col), or just write code like image(row, col) to 
     * get the image pixel
     * @return The reference of the image pixel
     * @warning In release mode, the row and col param must be valid, otherwise there will be an exception
     */
	virtual T& get_pixel(int row, int col) = 0;
	virtual const T& get_pixel(int row, int col) const = 0;
	virtual T& operator() (int row, int col) = 0;
	virtual const T& operator() (int row, int col) const = 0;

    /**
     * @brief Get the range image data
	* @param start_row The left-corner point row
	* @param start_col The left-corner point col
     * @param rows The row scope of the range, thus the rows get is [start_row, start_row + rows)
     * @param cols The col scope of the range
     * @param data [Out] Returns the image data vector
     * @return Whether get the data successfully
     */
	virtual bool get_pixel(int start_row, int start_col, int rows, int cols, std::vector<T> &data) const = 0;
	virtual bool set_pixel(int start_row, int start_col, int rows, int cols, const std::vector<T> &data) = 0;

public:
	size_t get_image_cols() const { return img_size.cols; }
	size_t get_image_rows() const { return img_size.rows; }

protected:
	Size img_size;
};

#endif