#ifndef _HIERARCHICAL_IMAGE_HPP
#define _HIERARCHICAL_IMAGE_HPP

#undef min
#undef max

#include "HierarchicalImage.h"
#include "BlockwiseImage.hpp"

#include <boost/lexical_cast.hpp>
#include <algorithm>

/* opencv part */
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#ifdef NDEBUG
#pragma comment(lib, "opencv_highgui240.lib")
#pragma comment(lib, "opencv_core240.lib")
#pragma comment(lib, "opencv_imgproc240.lib")
#else
#pragma comment(lib, "opencv_highgui240d.lib")
#pragma comment(lib, "opencv_core240d.lib")
#pragma comment(lib, "opencv_imgproc240d.lib")
#endif
/*---------------------------------------------*/

using boost::lexical_cast;
using namespace std;

#define PRINT(x) cout << #x << " : " << x << endl

template<typename T, size_t memory_usage>
HierarchicalImage<T, memory_usage>::HierarchicalImage(size_t rows, size_t cols, size_t mini_rows, size_t mini_cols,
	size_t file_cache_number, boost::shared_ptr<IndexMethodInterface> method)
	: HierarchicalInterface(rows, cols, mini_rows, mini_cols)
	, BlockwiseImage<T, memory_usage>(rows, cols, method)
{
	/* default is maximum way concurrent writing */
	set_mutliply_ways_writing_number(get_max_image_level() + 1);

	set_file_cache_number(file_cache_number);
}

template<typename T, size_t memory_usage>
HierarchicalImage<T, memory_usage>::~HierarchicalImage()
{
}

template<typename T, size_t memory_usage>
void HierarchicalImage<T, memory_usage>::set_file_node_size(int64 size)
{
	BlockwiseImage<T, memory_usage>::set_file_node_size(size);

	lru_image_files.init(file_node_size, file_cache_number);
}

template<typename T, size_t memory_usage>
bool HierarchicalImage<T, memory_usage>::load_image_head_file(const char *file_name)
{
	if(!BlockwiseImage::load_image_head_file(file_name))	return false;

	/* load the specific information of hierarchical image head */
	{
		ifstream fin(file_name, ios::in);
		if(!fin.is_open()) {
			cerr << "open " << file_name << "failure" << endl;
			return false;
		}

		string str;
		/* ignore the block wise image part */
		for(size_t i = 0; i < 6; ++i)
			getline(fin, str);

		if(fin.fail()) return false;

		string::size_type index = 0;

		try{
			/* get the image mini rows */
			getline(fin, str);
			index = str.find('=');
			if(index == string::npos || str.substr(0, index) != "minirows") {
				cerr << "image format is not correct" << endl;
				return false;
			}
			m_mini_rows = boost::lexical_cast<size_t>(str.substr(index+1));

			/* get the image mini cols */
			getline(fin, str);
			index = str.find('=');
			if(index == string::npos || str.substr(0, index) != "minicols") {
				cerr << "image format is not correct" << endl;
				return false;
			}
			m_mini_cols = boost::lexical_cast<size_t>(str.substr(index+1));

			/* get the hierarchal image max scale level */
			getline(fin, str);
			index = str.find('=');
			if(index == string::npos || str.substr(0, index) != "maxlevel") {
				cerr << "image format is not correct" << endl;
				return false;
			}
			m_max_level = boost::lexical_cast<size_t>(str.substr(index+1));
		} catch (boost::bad_lexical_cast &err) {
			cerr << "iamge format is not correct" << endl;
			cerr << err.what() << endl;
			fin.close();
			return false;
		}

		if(fin.eof()) return true;
		if(fin.fail()) return false;

		fin.close();
	}
	return true;
}

template<typename T, size_t memory_usage>
boost::shared_ptr<HierarchicalImage<T, memory_usage> > HierarchicalImage<T, memory_usage>::load_image(const std::string &file_name)
{
	return load_image(file_name.c_str());
}

template<typename T, size_t memory_usage>
boost::shared_ptr<HierarchicalImage<T, memory_usage> > HierarchicalImage<T, memory_usage>::load_image(const char *file_name)
{
	typedef boost::shared_ptr<HierarchicalImage<T, memory_usage> > PtrType;
	PtrType dst_image(new HierarchicalImage<T, memory_usage> (0, 0, 0, 0));
	PtrType null_image;

	if(!dst_image->load_image_head_file(file_name))	 return null_image;

	/* initialization */
	dst_image->set_image_data_path(file_name);
	dst_image->set_mutliply_ways_writing_number(dst_image->get_max_image_level() + 1);
	dst_image->set_file_cache_number(dst_image->file_cache_number);

	/*
	 * hierarchical image don't save specific level image data
	 * when using set_current_level function to set current level
	 * then using any kind of image access function, the data will be
	 * dynamically loaded, and of course there will be some kind of performance penalty
	 */
	return dst_image;
}

template<typename T, size_t memory_usage>
bool HierarchicalImage<T, memory_usage>::write_image(const std::string &file_name)
{
	return write_image(file_name.c_str());
}

template<typename T, size_t memory_usage>
bool HierarchicalImage<T, memory_usage>::write_image_head_file(const char *file_name)
{
	/* write the block wise image head info */
	if(!BlockwiseImage::write_image_head_file(file_name))	return false;

	/* append the specific hierarchical image head info */
	{
		ofstream fout(file_name, ios::out | ios::app);
		if(!fout.is_open()) {
			cerr << "open " << file_name << " failure" << endl;
			return false;
		}
		fout << "minirows=" << m_mini_rows << endl;
		fout << "minicols=" << m_mini_cols << endl;
		fout << "maxlevel=" << m_max_level << endl;
		fout.close();
	}

	return true;
}

template<typename T, size_t memory_usage>
bool HierarchicalImage<T, memory_usage>::write_image_inner_loop(size_t start_level, size_t merge_number,
	const bf::path &data_path, const int64 &file_number)
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
bool HierarchicalImage<T, memory_usage>::write_image(const char *file_name)
{
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

		/* now just read the highest level image to save for some kind for observation */
		set_current_level(get_max_image_level());

		std::vector<T> img_data;
		int start_rows = 0, start_cols = 0, rows = img_current_level_size.rows, cols = img_current_level_size.cols;

		if(!get_pixels_by_level(m_max_level, start_rows, start_cols, rows, cols, img_data)) {
			cerr << "get the maximum image failure" << endl;
			return false;
		}

		cv::Mat result_image(rows, cols, CV_8UC3, img_data.data());

		/* convert the RGB format to opencv BGR format */
		cv::cvtColor(result_image, result_image, CV_RGB2BGR);

		bf::path file_path(file_name);
		string result_image_name = (file_path.parent_path() / (file_path.stem().generic_string() + ".jpg")).generic_string();
		cv::imwrite(result_image_name, result_image);
		
	} catch(bf::filesystem_error &err) {
		cerr << err.what() << endl;
		return false;
	}

	return true;
}

inline size_t make_upper_four_multiply(size_t number) {
	//number % 4 != 0
	if(number & 0x00000003) return ((number >> 2) + 1) << 2;

	//number is the multiply of 4, then just get the number itself
	return number;
}

inline size_t make_less_four_multiply(size_t number) {
	//number % 4 != 0
	if(number & 0x00000003) return ((number >> 2) << 2);

	//number is the multiply of 4, then just get the number itself
	return number;
}

template<typename T, size_t memory_usage>
void HierarchicalImage<T, memory_usage>::set_current_level(int level)
{
	BOOST_ASSERT(level >= 0 && level <= m_max_level);

	/* if set the same level, do nothing */
	if(current_level == level)	return;

	current_level = level;

	/* current image size */
	img_current_level_size.rows = std::ceil((double)(img_size.rows) / (1 << level));
	img_current_level_size.cols = std::ceil((double)(img_size.cols) / (1 << level));

	/* change the new index method */
	index_method = boost::shared_ptr<IndexMethodInterface>(new ZOrderIndex(img_current_level_size.rows, img_current_level_size.cols));

	/* change the image level data path to the specific level*/
	img_level_data_path = img_data_path + "/level_" + boost::lexical_cast<string>(level);
}

template<typename T, size_t memory_usage>
size_t HierarchicalImage<T, memory_usage>::get_current_level() const 
{
	return current_level;
}

template<typename T, size_t memory_usage>
bool HierarchicalImage<T, memory_usage>::read_from_index_range(size_t front, size_t tail, ZOrderIndex::IndexType start_index, 
	const std::vector<DataIndexInfo> &index_info_vector, std::vector<T> &data_vector)
{
	BOOST_ASSERT(tail > front);

	/* total number for reading */
	size_t total = tail - front;

	/* save the actually first zorder index (take account of the start_index) */
	size_t zorder_index_front = index_info_vector[front].zorder_index + start_index;

	/* the first image file number */
	size_t start_file_number = (zorder_index_front >> file_node_shift_num);

	/* the seekg cell size in the file */
	size_t start_seekg = zorder_index_front - (start_file_number << file_node_shift_num);

	/* while the cell number has not been finished */
	while(total > 0) {
		/* image file name */
		string img_file_name = img_level_data_path + '/' + boost::lexical_cast<string>(start_file_number);

		/* the file_index means the index of the img_file_name in lru_image_files */
		int file_index = lru_image_files.put_into_lru(img_file_name);

		/* if not get the reasonable position, there must be some kind of error, so just return false */
		if(file_index == lru_image_files.npos)	return false;

		const vector<Vec3b> &file_data = lru_image_files.get_data(file_index);

		size_t read_number = min<size_t>(tail - front, file_node_size - start_seekg);

		/* write data into the front location */
		for(size_t i = 0; i < read_number; ++i) {
			data_vector[index_info_vector[front++].index] = file_data[start_seekg + i];
		}

		total -= read_number;
		if(total <= 0)	break;

		/* here means prepare for next loop */
		/* make the seekg = 0, means in later loop the seekg will just begin from the start point of each file for reading */
		start_seekg = 0;

		/* the next file is just one number larger than formal image */
		++start_file_number;
	}

	return true;
}

template<typename T, size_t memory_usage>
bool HierarchicalImage<T, memory_usage>::check_para_validation(int level, int start_row, int start_col, int rows, int cols) 
{
	if(level > m_max_level || level < 0) {
		cerr << "HierarchicalImage::get_pixels_by_level function para error : invalid level" << endl;
		return false;
	}

	set_current_level(level);

	if(start_row >= img_current_level_size.rows || start_row < 0) {
		cerr << "HierarchicalImage::get_pixels_by_level function para error : invalid start_rows" << endl;
		return false;
	}

	if(start_col >= img_current_level_size.cols || start_col < 0) {
		cerr << "HierarchicalImage::get_pixels_by_level function para error : invalid start_cols" << endl;
		return false;
	}

	if(start_row + rows > img_current_level_size.rows) {
		cerr << "HierarchicalImage::get_pixels_by_level function para error "<< endl;
		cerr << "rows is too large, changed to the appropriate size" << endl; 
		return false;
	}

	if(start_col + cols > img_current_level_size.cols) {
		cerr << "HierarchicalImage::get_pixels_by_level function para err" << endl;
		cout << "cols is too large, changed to the appropriate size" << endl; 
		return false;
	}
}

template<typename T, size_t memory_usage>
bool HierarchicalImage<T, memory_usage>::get_pixels_by_level(int level, int &start_row, int &start_col,
	int &rows, int &cols, std::vector<T> &vec)
{
	if(!check_para_validation(level, start_row, start_col, rows, cols)) return false;

	/* first recalculate the para */
	start_row = make_less_four_multiply(start_row);
	start_col = make_less_four_multiply(start_col);
	rows = make_less_four_multiply(rows);
	cols = make_less_four_multiply(cols);

    /* save the zorder indexing method information*/
	std::vector<DataIndexInfo> index_info_vector(rows*cols);

	/* save the actual image data in row-major */
    vec.resize(rows*cols);

	/* the start index of the image range, thus the zorder index of the top-left point */
	ZOrderIndex::IndexType start_zorder_index = index_method->get_index(start_row, start_col);

	/* initialize the data index information */ 
	for(size_t i = 0; i < rows; ++i) {
		ZOrderIndex::IndexType row_result = index_method->get_row_result(i + start_row);
		size_t row_index = i*cols;	
		for(size_t j = 0; j < cols; ++j) {
			index_info_vector[row_index + j].index = row_index + j;
			index_info_vector[row_index + j].zorder_index = 
				index_method->get_index_by_row_result(row_result, j + start_col) - start_zorder_index; 
		}
	}

	/* sort the index info vector by the zorder index value */
	std::sort(index_info_vector.begin(), index_info_vector.end());

	/* front and tail means a range of the successive zorder index in format [front, tail) */
	size_t front = 0, tail = 0;
	const size_t end = rows*cols;  

	/* first make the front and tail the same, the expect_index means the expect zorder index when searching in successive way*/
	front = tail = 0;
	ZOrderIndex::IndexType expect_index = index_info_vector[front].zorder_index;
	while(front < end) {
		expect_index = index_info_vector[front].zorder_index;

		while(tail < end && expect_index == index_info_vector[tail].zorder_index) {
			++tail;
			++expect_index;
		}

		/*now get the successive zorder index range [front, tail) */
		{
			BOOST_ASSERT(tail > front);

			/* total number for reading */
			size_t total = tail - front;

			/* save the actually first zorder index (take account of the start_zorder_index) */
			size_t zorder_index_front = index_info_vector[front].zorder_index + start_zorder_index;

			/* the first image file number */
			size_t start_file_number = (zorder_index_front >> file_node_shift_num);

			/* the seekg cell size in the file */
			size_t start_seekg = zorder_index_front - (start_file_number << file_node_shift_num);

			/* while the cell number has not been finished */
			while(total > 0) {
				/* image file name */
				string img_file_name = img_level_data_path + '/' + boost::lexical_cast<string>(start_file_number);

				/* the file_index means the index of the img_file_name in lru_image_files */
				int file_index = lru_image_files.put_into_lru(img_file_name);

				/* if not get the reasonable position, there must be some kind of error, so just return false */
				if(file_index == lru_image_files.npos)	return false;

				const vector<Vec3b> &file_data = lru_image_files.get_const_data(file_index);

				size_t read_number = min<size_t>(tail - front, file_node_size - start_seekg);

				/* write data into the front location */
				for(size_t i = 0; i < read_number; ++i) {
					vec[index_info_vector[front++].index] = file_data[start_seekg + i];
				}

				total -= read_number;
				if(total <= 0)	break;

				/* here means prepare for next loop */
				/* make the seekg = 0, means in later loop the seekg will just begin from the */
				/* start point of each file for reading */
				start_seekg = 0;

				/* the next file is just one number larger than formal image */
				++start_file_number;
			}
		}

		front = tail;
	}

	return true;
}

template<typename T, size_t memory_usage>
bool HierarchicalImage<T, memory_usage>::set_pixel_by_level(int level, int start_row, int start_col, int rows, int cols, const std::vector<T> &vec)
{	
	if(!check_para_validation(level, start_row, start_col, rows, cols)) return false;

	/* save the zorder indexing method information*/
	std::vector<DataIndexInfo> index_info_vector(rows*cols);

	/* the start index of the image range, thus the zorder index of the top-left point */
	ZOrderIndex::IndexType start_zorder_index = index_method->get_index(start_row, start_col);

	/* initialize the data index information */ 
	for(size_t i = 0; i < rows; ++i) {
		ZOrderIndex::IndexType row_result = index_method->get_row_result(i + start_row);
		size_t row_index = i*cols;	
		for(size_t j = 0; j < cols; ++j) {
			index_info_vector[row_index + j].index = row_index + j;
			index_info_vector[row_index + j].zorder_index = 
				index_method->get_index_by_row_result(row_result, j + start_col) - start_zorder_index; 
		}
	}

	/* sort the index info vector by the zorder index value */
	std::sort(index_info_vector.begin(), index_info_vector.end());

	/* front and tail means a range of the successive zorder index in format [front, tail) */
	size_t front = 0, tail = 0;
	const size_t end = rows*cols;  

	/* first make the front and tail the same, the expect_index means the expect zorder index when searching in successive way*/
	front = tail = 0;
	ZOrderIndex::IndexType expect_index = index_info_vector[front].zorder_index;
	while(front < end) {
		expect_index = index_info_vector[front].zorder_index;

		while(tail < end && expect_index == index_info_vector[tail].zorder_index) {
			++tail;
			++expect_index;
		}

		/*now get the successive zorder index range [front, tail) */
		{
			BOOST_ASSERT(tail > front);

			/* total number for reading */
			size_t total = tail - front;

			/* save the actually first zorder index (take account of the start_zorder_index) */
			size_t zorder_index_front = index_info_vector[front].zorder_index + start_zorder_index;

			/* the first image file number */
			size_t start_file_number = (zorder_index_front >> file_node_shift_num);

			/* the seekg cell size in the file */
			size_t start_seekg = zorder_index_front - (start_file_number << file_node_shift_num);

			/* while the cell number has not been finished */
			while(total > 0) {
				/* image file name */
				string img_file_name = img_level_data_path + '/' + boost::lexical_cast<string>(start_file_number);

				/* the file_index means the index of the img_file_name in lru_image_files */
				int file_index = lru_image_files.put_into_lru(img_file_name);

				/* if not get the reasonable position, there must be some kind of error, so just return false */
				if(file_index == lru_image_files.npos)	return false;

				/* using get_data function will make the file_index cache be dirty, thus will be write back when the cache is swap out 
				 * of the memory */
				vector<Vec3b> &file_data = lru_image_files.get_data(file_index);

				size_t read_number = min<size_t>(tail - front, file_node_size - start_seekg);

				/* just read the data into the right place */
				for(size_t i = 0; i < read_number; ++i) {
					file_data[start_seekg + i] = vec[index_info_vector[front++].index];
				}

				total -= read_number;
				if(total <= 0)	break;

				/* here means prepare for next loop */
				/* make the seekg = 0, means in later loop the seekg will just begin from the */
				/* start point of each file for reading */
				start_seekg = 0;

				/* the next file is just one number larger than formal image */
				++start_file_number;
			}
		}

		front = tail;
	}

	return true;
}

#endif



