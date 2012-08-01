#ifndef _HIERARCHICAL_IMAGE_HPP
#define _HIERARCHICAL_IMAGE_HPP

#undef min
#undef max

#include "HierarchicalImage.h"
#include "BlockwiseImage.hpp"

#include <boost/lexical_cast.hpp>
#include <algorithm>

template<typename T, size_t memory_usage>
HierarchicalImage<T, memory_usage>::HierarchicalImage(size_t rows, size_t cols, size_t mini_rows, size_t mini_cols,
	boost::shared_ptr<IndexMethodInterface> method)
	: BlockwiseImage<T, memory_usage>(rows, cols, mini_rows, mini_cols, method)
{
	/* default is maximum way concurrent writing */
	set_mutliply_ways_writing_number(get_max_image_level() + 1);
}

template<typename T, size_t memory_usage>
HierarchicalImage<T, memory_usage>::~HierarchicalImage()
{
}

template<typename T, size_t memory_usage>
bool HierarchicalImage<T, memory_usage>::write_image_head_file(const char *file_name)
{
	/* write the block wise image head info */
	if(!BlockwiseImage::write_image_head_file(file_name))	return false;

	/* append the specific hierarchical image head info */
	{
		std::ofstream fout(file_name, std::ios::out | std::ios::app);
		if(!fout.is_open()) {
			std::cerr << "open " << file_name << " failure" << std::endl;
			return false;
		}

		/* just write the max level para */
		fout << "maxlevel=" << m_max_level << std::endl;
		fout.close();
	}

	return true;
}

template<typename T, size_t memory_usage>
bool HierarchicalImage<T, memory_usage>::write_image(const char *file_name)
{
	using namespace std;
	namespace bf = boost::filesystem3;

	try {
		if(!write_image_head_file(file_name))	return false;

		/* set the img_data_path */
		set_image_data_path(file_name);

		/* check the validation of img_data_path */
		bf::path data_path(img_data_path);
		if(bf::exists(data_path)) {
			bf::remove_all(data_path);
			cout << "[Warning] : " << data_path.generic_string() 
				<< " is existing, and the original directory will be removed" << endl;
		}
		if(!bf::create_directory(data_path)) {
			cerr << "create directory " << data_path.generic_string() << " failure" << endl;
			return false;
		}

		const int64 file_number = std::ceil((double)(img_container.size()) / file_node_size);

		/* now write the code for multiply ways concurrently writing image data */
		size_t max_concurrent_loop = (get_max_image_level() + 1) / concurrent_number;
		for(size_t concurrent_loop = 0; concurrent_loop < max_concurrent_loop; ++concurrent_loop) {
			size_t start_level = concurrent_loop*concurrent_number;

			/* write concurrent_number level in concurrent begging from the start_level */
			if(!write_image_inner_loop(start_level, concurrent_number, data_path, file_number))
				return false;
		} // end for concurrent loop

		/* write the residual concurrent_loop */
		size_t start_level = max_concurrent_loop * concurrent_number;
		if(!write_image_inner_loop(start_level, get_max_image_level() - start_level + 1, data_path, file_number))
			return false;

		/* save the mini image as a jpg format */
		if(!save_mini_image(file_name)) return false;

	} catch(bf::filesystem_error &err) {
		cerr << err.what() << endl;
		return false;
	}

	return true;
}


template<typename T, size_t memory_usage>
bool HierarchicalImage<T, memory_usage>::write_image_inner_loop(size_t start_level, size_t merge_number,
	const boost::filesystem3::path &data_path, const int64 &file_number)
{
	/*
	*	fout_array : different ofstream for multiply ways writing into files
	*	fout_complete : tells whether the ofstream of a file has complete the writing
	*	the file name of ofstream is decided by fout_file_level_number and fout_level_path
	*	for example :
	*	write file into ./some image
	*	then fout_level_path saves like ./some image/level_0 ./some image/level_1	 etc
	*	fout_file_level_number saves the current number of each level
	*	the entire file name is like:
	*	./some image/level_0/fout_file_level_number;
	*/

	using boost::lexical_cast;
	using namespace std;
	namespace bf = boost::filesystem3;

	static const ContainerType &c_img_container = img_container;

	std::vector<ofstream> fout_array(merge_number);
	std::vector<bool> fout_complete(merge_number, true);
	std::vector<size_t> fout_file_level_number(merge_number, 0);
	std::vector<bf::path> fout_level_path(merge_number);
	std::vector<size_t> mask(merge_number, 0);

	/* initialization */
	for(size_t k = 0; k < merge_number; ++k) {
		fout_level_path[k] = data_path / (string("level_") + lexical_cast<std::string>(k+start_level));

		/* mask is 4^n - 1, start_level is the level delta from last loop */
		mask[k] = (1 << (2*(k + start_level))) - 1;
	}

	/* Before writing, first create the needed directories */
	for(size_t k = 0; k < merge_number; ++k) {
		if(!bf::create_directory(fout_level_path[k])) {
			cerr << "create directory " << fout_level_path[k].generic_string() << " failure" << endl;
			return false;
		}
	}

	/* writing the first (file_number - 1) data files */
	int64 file_loop = 0, start_index = 0;
	for(; file_loop < (file_number - 1); ++file_loop) {
		/* check ofstream status to decide to whether to open a new file for writing */
		for(size_t k = 0; k < merge_number; ++k) {
			if(fout_complete[k]) {
				string level_file_name =
					(fout_level_path[k] / lexical_cast<string>(fout_file_level_number[k])).generic_string();

				/* open file for writing */
				fout_array[k].open(level_file_name, ios::out | ios::binary);
				if(!fout_array[k].is_open()) {
					cerr << "open file " << level_file_name << " failure" << endl;
					return false;
				}

				/* tells the ofstream is in using, thus not complete for creating a new file */
				fout_complete[k] = false;
			}
		}

		/* write one file data */
		start_index = file_loop << file_node_shift_num;
		for(int64 i = 0; i < file_node_size; ++i) {
			for(size_t k = 0; k < merge_number; ++k) {
				/* if index i is the multiply of 2^k, then write the data into the fout_array[k] */
				if((i & mask[k]) == 0)
					fout_array[k].write(reinterpret_cast<const char*>(&c_img_container[start_index + i]), sizeof(T));
			}
		}

		/* check ofstream status again to decide whether to close a file for writing */
		for(size_t k = 0; k < merge_number; ++k) {
			/* attention : using file_loop+1, because the file is counted from 1 */
			/* if just using file_loop, when file_loop is 0, all the ofstream will be closed that's certainly not correct */
			if(((file_loop+1) & mask[k]) == 0) {
				fout_array[k].close();

				/* prepare for next file writing */
				fout_file_level_number[k]++;
				fout_complete[k] = true;
			}
		}
	} // end loop for writing of first (file_number - 1) files

	/* now just has left one file for writing */

	/* still first checks whether to open a new file for each ofstream */
	for(size_t k = 0; k < merge_number; ++k) {
		if(fout_complete[k]) {
			string level_file_name = (fout_level_path[k] / lexical_cast<string>(fout_file_level_number[k])).generic_string();
			fout_array[k].open(level_file_name, ios::out | ios::binary);

			if(!fout_array[k].is_open()) {
				cerr << "open file " << level_file_name << " failure" << endl;
				return false;
			}
		}
	}

	/* write the file data */
	start_index = file_loop << file_node_shift_num;
	for(int64 last_index = start_index; last_index < c_img_container.size(); ++last_index) {
		for(size_t k = 0; k < merge_number; ++k) {
			/* if index last_index is the multiply of 2^k, then write the data into the fout_array[k] */
			if((last_index & mask[k]) == 0)
				fout_array[k].write(reinterpret_cast<const char*>(&c_img_container[last_index]), sizeof(T));
		}
	}

	/* now just close all the files */
	for(size_t k = 0; k < merge_number; ++k)
		fout_array[k].close();

	return true;
}

template<typename T, size_t memory_usage>
bool HierarchicalImage<T, memory_usage>::write_image(const std::string &file_name)
{
	return write_image(file_name.c_str());
}

#include "DiskBigImage.hpp"
template<typename T, size_t memory_usage>
bool HierarchicalImage<T, memory_usage>::save_mini_image(const char* file_name)
{
	/* since the image data has been write successfully, we can using the DiskBigImageInterface to access
	 * the image data in the disk
	 */
	boost::shared_ptr<DiskBigImageInterface<T> > big_image = load_image<T>(file_name);

	/* now just read the highest level image to save as a jpg file */
	big_image->set_current_level(big_image->get_max_image_level());

	int start_rows = 0, start_cols = 0;
	int rows = big_image->get_current_level_image_rows();
	int cols = big_image->get_current_level_image_cols();

	std::vector<T> img_data;
	if(!big_image->get_pixels_by_level(m_max_level, start_rows, start_cols, rows, cols, img_data)) {
		std::cerr << "get the maximum image failure" << std::endl;
		return false;
	}

	cv::Mat result_image(rows, cols, CV_8UC3, img_data.data());

	/* convert the RGB format to opencv BGR format */
	cv::cvtColor(result_image, result_image, CV_RGB2BGR);
	boost::filesystem3::path file_path(file_name);
	std::string result_image_name = (file_path.parent_path() / (file_path.stem().generic_string() + ".jpg")).generic_string();
	cv::imwrite(result_image_name, result_image);

	return true;
}

#endif



