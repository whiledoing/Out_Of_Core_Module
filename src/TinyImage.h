#ifndef _TINY_IMAGE_H
#define _TINY_IMAGE_H

#include "ImageInterface.h"

template<typename T>
class TinyImage: public ImageInterface<T>
{
public:
	virtual ~TinyImage();

	virtual bool load_image(const char * file_name);
	virtual bool load_image(const std::string &file_name);
	virtual bool write_image(const char* file_name);
	virtual bool write_image(const std::string &file_name);

	virtual T& get_pixel(int x, int y);
	virtual const T& get_pixel(int x, int y) const;

	virtual bool get_pixel(int min_x, int max_x, int min_y, int max_y, T* ptr) const;
	virtual bool set_pixel(int min_x, int max_x, int min_y, int max_y, T* ptr);

private:
	T *image_data;
};

#endif