#ifndef _BLOCKWISE_IMAGE_HPP
#define _BLOCKWISE_IMAGE_HPP

#include "BlockwiseImage.h"
#include <boost/assert.hpp>
#include <string>
#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <strstream>
namespace bf=boost::filesystem3;
using namespace std;

template<typename T, unsigned memory_usage>
BlockwiseImage<T, memory_usage>::BlockwiseImage(size_t rows, size_t cols, boost::shared_ptr<IndexMethodInterface> method)
	: GiantImageInterface(method ? method : (boost::shared_ptr<IndexMethodInterface>(new ZOrderIndex(rows, cols))))
{
	init(rows, cols);
}

template<typename T, unsigned memory_usage>
BlockwiseImage<T, memory_usage>::~BlockwiseImage()
{
	img_container.clear();
}

template<typename T, unsigned memory_usage>
bool BlockwiseImage<T, memory_usage>::set_pixel(int min_row, int max_row, int min_col, int max_col, const T* ptr)
{
	BOOST_ASSERT(ptr != NULL);
	BOOST_ASSERT(0 <= min_col && min_col <= max_col && max_col < img_size.cols && 0 <= min_row
		&& min_row <= max_row && max_row < img_size.rows);

	size_t count = 0;
	for(IndexMethodInterface::RowMajorIndexType row = min_row; row <= max_row; ++row) {
		IndexMethodInterface::IndexType row_result = index_method->get_row_result(row);
		for(IndexMethodInterface::RowMajorIndexType col = min_col; col <= max_col; ++col) {
			img_container[index_method->get_index_by_row_result(row_result, col)]	 = ptr[count++];
		}
	}
	return true;
}

template<typename T, unsigned memory_usage>
bool BlockwiseImage<T, memory_usage>::get_pixel(int min_row, int max_row, int min_col, int max_col, T* ptr) const
{
	BOOST_ASSERT(ptr != NULL);
	BOOST_ASSERT(0 <= min_col && min_col <= max_col && max_col < img_size.cols && 0 <= min_row
		&& min_row <= max_row && max_row < img_size.rows);

	static const ContainerType &c_img_container = img_container;
	size_t count = 0;

	for(IndexMethodInterface::RowMajorIndexType row = min_row; row <= max_row; ++row) {
		IndexMethodInterface::IndexType row_result = index_method->get_row_result(row);
		for(IndexMethodInterface::RowMajorIndexType col = min_col; col <= max_col; ++col) {
			ptr[count++] = c_img_container[index_method->get_index_by_row_result(row_result, col)];
		}
	}
	return true;
}

template<typename T, unsigned memory_usage>
bool BlockwiseImage<T, memory_usage>::set_pixel(int min_row, int max_row, int min_col, int max_col, const std::vector<T> &data)
{
	BOOST_ASSERT(data.size() >= (max_col - min_col + 1) * (max_row - min_row + 1));
	BOOST_ASSERT(0 <= min_col && min_col <= max_col && max_col < img_size.cols && 0 <= min_row
		&& min_row <= max_row && max_row < img_size.rows);

	size_t count = 0;
	for(IndexMethodInterface::RowMajorIndexType row = min_row; row <= max_row; ++row) {
		IndexMethodInterface::IndexType row_result = index_method->get_row_result(row);
		for(IndexMethodInterface::RowMajorIndexType col = min_col; col <= max_col; ++col) {
			img_container[index_method->get_index_by_row_result(row_result, col)] = data[count++];
		}
	}
	return true;
}

template<typename T, unsigned memory_usage>
bool BlockwiseImage<T, memory_usage>::get_pixel(int min_row, int max_row, int min_col, int max_col, std::vector<T> &data) const
{
	BOOST_ASSERT(0 <= min_col && min_col <= max_col && max_col < img_size.cols && 0 <= min_row
		&& min_row <= max_row && max_row < img_size.rows);

	data.resize((max_col - min_col + 1) * (max_row - min_row + 1));

	static const ContainerType &c_img_container = img_container;
	size_t count = 0;

	for(IndexMethodInterface::RowMajorIndexType row = min_row; row <= max_row; ++row) {
		IndexMethodInterface::IndexType row_result = index_method->get_row_result(row);
		for(IndexMethodInterface::RowMajorIndexType col = min_col; col <= max_col; ++col) {
			data[count++] = c_img_container[index_method->get_index_by_row_result(row_result, col)];
		}
	}
	return true;
}

template<typename T, unsigned memory_usage>
const T& BlockwiseImage<T, memory_usage>::operator()(int row, int col) const
{
	BOOST_ASSERT(0 <= row && row < img_size.rows && 0 <= col && col < img_size.cols);
	static const ContainerType &c_img_container = img_container;
	return c_img_container[index_method->get_index(row, col)];
}

template<typename T, unsigned memory_usage>
T& BlockwiseImage<T, memory_usage>::operator()(int row, int col)
{
	BOOST_ASSERT(0 <= row && row < img_size.rows && 0 <= col && col < img_size.cols);
	return img_container[index_method->get_index(row, col)];
}

template<typename T, unsigned memory_usage>
const T& BlockwiseImage<T, memory_usage>::get_pixel(int row, int col) const
{
	return this->operator() (row, col);
}

template<typename T, unsigned memory_usage>
T& BlockwiseImage<T, memory_usage>::get_pixel(int row, int col)
{
	return this->operator() (row, col);
}

template<typename T, unsigned memory_usage>
bool BlockwiseImage<T, memory_usage>::write_image(const std::string &file_name)
{
	return write_image(file_name.c_str());
}

template<typename T, unsigned memory_usage>
bool BlockwiseImage<T, memory_usage>::write_image(const char* file_name)
{
	try {
		if(!write_image_head_file(file_name))	return false;

		bf::path file_path = file_name;
		bf::path data_path = (file_path.parent_path() / file_path.stem()).make_preferred();
		if(bf::exists(data_path)) {
			bf::remove_all(data_path);
			cout << "[Warning] : " << data_path << " is existing, and the original directory will be removed" << endl;
		}

		/* block wise image only has one level : means the full size level */
		data_path /= bf::path("level_0");
		if(!bf::create_directories(data_path)) {
			cerr << "create directory " << data_path << "failure" << endl;
			return false;
		}

		/* because just read the image data, so just const reference to read for some kind of optimization */
		static const ContainerType &c_img_container = img_container;
		int64 file_number = std::ceil((double)(c_img_container.size()) / file_node_size);

		/* first write the full one file context */
		int64 start_index = 0, file_loop = 0;
		for(; file_loop < file_number - 1; ++file_loop) {
			std::ostrstream strstream;
			strstream << data_path.generic_string() << "/" << file_loop << '\0';
			ofstream file_out(strstream.str(), ios::out | ios::binary);
			if(!file_out.is_open()) {
				cerr << "create " << strstream.str() << " failure" << endl;
				return false;
			}

			start_index = (int64)(file_loop) << file_node_shift_num;
			for(int64 i = 0; i < file_node_size; ++i) {
				file_out.write(reinterpret_cast<const char*>(&c_img_container[start_index + i]), sizeof(T));
			}
			file_out.close();
		}

		/* write the last file till the end of the container(maybe not full) */
		start_index = (int64)(file_loop) << file_node_shift_num;
		std::ostrstream strstream;
		strstream << data_path.generic_string() << "/" << file_loop << '\0';
		ofstream file_out(strstream.str(), ios::out | ios::binary);
		if(!file_out.is_open()) {
			cerr << "create " << strstream.str() << " failure" << endl;
			return false;
		}

		for(int64 last_index = start_index; last_index < c_img_container.size(); ++last_index) {
			file_out.write(reinterpret_cast<const char*>(&c_img_container[last_index]), sizeof(T));
		}
		file_out.close();
	} catch(bf::filesystem_error &err) {
		cerr << err.what() << endl;
		return false;
	}

	return true;
}

template<typename T, unsigned memory_usage>
boost::shared_ptr<BlockwiseImage<T, memory_usage> > BlockwiseImage<T, memory_usage>::load_image(const std::string &file_name)
{
	return load_image(file_name.c_str());
}

template<typename T, unsigned memory_usage>
boost::shared_ptr<BlockwiseImage<T, memory_usage> > BlockwiseImage<T, memory_usage>::load_image(const char *file_name)
{
	typedef boost::shared_ptr<BlockwiseImage<T, memory_usage> > PtrType;
	PtrType dst_image(new BlockwiseImage<T, memory_usage> (0, 0));
	PtrType null_image;

	if(!dst_image->load_image_head_file(file_name))	return null_image;

	bf::path file_path(file_name);
	bf::path data_path = (file_path.parent_path() / file_path.stem()).make_preferred();
	data_path /= "level_0";
	if(!bf::exists(data_path)) {
		cerr << "image data missing" << endl;
		return null_image;
	}

	/* get the new value, then init for using */
	dst_image->init(dst_image->get_image_rows(), dst_image->get_image_cols());

	ContainerType &img_container = dst_image->img_container;
	int64 file_number = std::ceil((double)(img_container.size()) / dst_image->file_node_size);

	int64 start_index = 0, file_loop = 0;
	int64 file_node_shift_num = dst_image->file_node_shift_num;
	int64 file_node_size = dst_image->file_node_size;

	/* first read the full context files */
	for(; file_loop < file_number - 1; ++file_loop) {
		std::ostrstream strstream;
		strstream << data_path.generic_string() << "/" << file_loop << '\0';
		if(!bf::exists(bf::path(strstream.str()))) {
			cerr << "image data missing" << endl;
			return null_image;
		}

		/* now read the existing data file */
		ifstream file_in(strstream.str(), ios::out | ios::binary);
		if(!file_in.is_open()) {
			cerr << "open" << strstream.str() << " failure" << endl;
			return null_image;
		}

		start_index = (int64)(file_loop) << file_node_shift_num;
		for(int64 i = 0; i < file_node_size; ++i) {
			file_in.read(reinterpret_cast<char*>(&img_container[start_index + i]), sizeof(T));
		}
		file_in.close();
	}

	/* now read the last file */
	start_index = (int64)(file_loop) << file_node_shift_num;
	std::ostrstream strstream;
	strstream << data_path.generic_string() << "/" << file_loop << '\0';
	if(!bf::exists(bf::path(strstream.str()))) {
		cerr << "image data missing" << endl;
		return null_image;
	}
	ifstream file_in(strstream.str(), ios::out | ios::binary);
	if(!file_in.is_open()) {
		cerr << "open" << strstream.str() << " failure" << endl;
		return null_image;
	}
	for(int64 last_index = start_index; last_index < img_container.size(); ++last_index) {
		file_in.read(reinterpret_cast<char*>(&img_container[last_index]), sizeof(T));
	}
	file_in.close();

	return dst_image;
}

template<typename T, unsigned memory_usage>
bool BlockwiseImage<T, memory_usage>::reset()
{
	img_size.cols = img_size.rows = 0;
	img_container.resize(0);
	return true;
}

template<typename T, unsigned memory_usage>
bool BlockwiseImage<T, memory_usage>::init(size_t rows, size_t cols)
{
	/* ensure index_method is valid */
	BOOST_ASSERT(index_method.use_count() != 0);

	img_size.rows = rows;
	img_size.cols = cols;

	img_container.resize(index_method->get_max_index() + 1);
	return true;
}

template<typename T, unsigned memory_usage>
const T& BlockwiseImage<T, memory_usage>::at(IndexMethodInterface::IndexType index) const
{
	BOOST_ASSERT(index < img_container.size());
	static const ContainerType &c_img_container = img_container;
	return c_img_container[index];
}

template<typename T, unsigned memory_usage>
T& BlockwiseImage<T, memory_usage>::at(IndexMethodInterface::IndexType index)
{
	BOOST_ASSERT(index < img_container.size());
	return img_container[index];
}

template<typename T, unsigned memory_usage>
bool BlockwiseImage<T, memory_usage>::write_image_head_file(const char* file_name)
{
	try {
		bf::path file_path(file_name);
		if(bf::is_directory(file_path)) {
			cerr << "file name should be a normal file"  << endl;
			return false;
		}

		if(bf::extension(file_path) != str_extension) {
			cerr << "extension should be bigimage" << endl;
			return false;
		}

		if(!bf::exists(file_path.parent_path()))
			bf::create_directories(file_path.parent_path());
	} catch(bf::filesystem_error &err) {
		cerr << err.what() << endl;
		return false;
	}

	ofstream fout(file_name, ios::out);
	if(!fout.is_open()) {
		cerr << "create " << file_name << " failure" << endl;
		return false;
	}

	/* the head file info */
	fout << "type=" << "BlockwiseImage" << endl;
	fout << "rows=" << img_size.rows << endl;
	fout << "cols=" << img_size.cols << endl;
	fout << "filenodesize=" << file_node_size << endl;
	fout << "filenodeshiftnum=" << file_node_shift_num << endl;
	fout << "indexmethod=" << index_method->get_index_method_name() << endl;
	fout.close();

	return true;
}

template<typename T, unsigned memory_usage>
bool BlockwiseImage<T, memory_usage>::load_image_head_file(const char* file_name)
{
	/* first check file existence */
	try {
		bf::path file_path(file_name);
		if(!bf::exists(file_path)) {
			cerr << "not exists the bigimage file" << endl;
			return false;
		}
		if(!bf::is_regular_file(file_path)) {
			cerr << "file name is not a regular file" << endl;
			return false;
		}
		if(bf::extension(file_path) != str_extension) {
			cerr << "extension should be bigimage" << endl;
			return false;
		}
	} catch(bf::filesystem_error &err) {
		cerr << err.what() << endl;
		return false;
	}

	ifstream fin(file_name, ios::in);
	if(!fin.is_open()) {
		cerr << file_name << " can't be opened for reading" << endl;
		return false;
	}

	string str;
	string::size_type index = 0;
	getline(fin, str);

	/* check image head type */
	if(str != "type=BlockwiseImage") {
		cerr << "image format is not correct" << endl;
		return false;
	}

	try {
		/* get the image rows */
		getline(fin, str);
		index = str.find('=');
		if(index == string::npos || str.substr(0, index) != "rows") {
			cerr << "image format is not correct" << endl;
			return false;
		}
		img_size.rows = boost::lexical_cast<size_t>(str.substr(index+1));

		/* get the image cols */
		getline(fin, str);
		index = str.find('=');
		if(index == string::npos || str.substr(0, index) != "cols") {
			cerr << "image format is not correct" << endl;
			return false;
		}
		img_size.cols = boost::lexical_cast<size_t>(str.substr(index+1));

		/* get the file node size */
		getline(fin, str);
		index = str.find('=');
		if(index == string::npos || str.substr(0, index) != "filenodesize") {
			cerr << "image format is not correct" << endl;
			return false;
		}
		file_node_size = boost::lexical_cast<size_t>(str.substr(index+1));

		/* get the file node shift number */
		getline(fin, str);
		index = str.find('=');
		if(index == string::npos || str.substr(0, index) != "filenodeshiftnum") {
			cerr << "image format is not correct" << endl;
			return false;
		}
		file_node_shift_num = boost::lexical_cast<size_t>(str.substr(index+1));
	} catch(boost::bad_lexical_cast &err) {
		cerr << err.what() << endl;
		fin.close();
		return false;
	}

	/* get the index method */
	getline(fin, str);
	index = str.find('=');
	if(index == string::npos || str.substr(0, index) != "indexmethod") {
		cerr << "image format is not correct" << endl;
		return false;
	}
	string index_str = str.substr(index+1);
	if(index_str == "ZOrderIndex") {
		index_method = boost::make_shared<ZOrderIndex>(img_size.rows, img_size.cols);
	} else if (index_str == "ZOrderIndexIntuition") {
		index_method = boost::make_shared<ZOrderIndexIntuition>(img_size.rows, img_size.cols);
	} else {
		cerr << "image format is not correct" << endl;
		return false;
	}

	if(fin.eof())	return true;
	if(fin.fail()) return false;

	return true;
}

#endif