#ifndef _GIANTDISK_IMAGE_H
#define _GIANTDISK_IMAGE_H 

#include "DataType.h"
#include "HierarchicalInterface.h"
#include "Lru.hpp"
#include "IndexMethod.hpp"

#include <vector>

template<typename T>
class GiantDiskImage : public HierarchicalInterface<T>
{

public:
	/* Derived from HierarchicalInterface */
	virtual bool get_pixels_by_level(int level, int &start_row, int &start_col, int &rows, int &cols, std::vector<T> &vec);
	virtual bool set_pixel_by_level(int level, int start_row, int start_col, int rows, int cols, const std::vector<T> &vec);
	virtual void set_current_level(int level);
	virtual size_t get_current_level() const; 

	virtual size_t get_current_level_image_rows() const 
	{
		return img_current_level_size.rows;
	}
	virtual size_t get_current_level_image_cols() const 
	{
		return img_current_level_size.cols;
	}

public:
    /*
	 *	@brief : set the file cache number of lru when loading image data 
	 */
	inline void set_file_cache_number(int _file_cache_number) 
	{
		BOOST_ASSERT(_file_cache_number > 0);
		file_cache_number = _file_cache_number;
		lru_image_files.init(file_node_size, file_cache_number);
	}

protected:
	bool read_from_index_range(size_t front, size_t tail, ZOrderIndex::IndexType start_index, 
		const std::vector<DataIndexInfo> &index_info_vector, std::vector<T> &data_vector);
	bool check_para_validation(int level, int start_row, int start_col, int rows, int cols);
	bool load_image_head_file(const char *file_name);
	void set_image_data_path(const char * file_name);
	void init() 
	{
		/* by default, the cache number is 16 */
		set_file_cache_number(16);
	}

protected:
	GiantDiskImage() 
	{
		init();
	}

	template<typename T>
	friend boost::shared_ptr<GiantDiskImage<T> > load_image(const char *file_name);

protected:
	struct DataIndexInfo
	{
		int64 index;			//keeps the original index (in row-major order)
		int64 zorder_index;	//keeps the zorder index according to the row-major index

		friend inline bool operator< (const DataIndexInfo& lhs, const DataIndexInfo& rhs) 
		{
			return (lhs.zorder_index < rhs.zorder_index);
		}
	};

protected:
	Size img_size;

	/* the number of cells in individual file*/
	int64 file_node_size;

	/* the shift number of file_node_size */
	int64 file_node_shift_num;

	/* the shared_ptr of index method */
	boost::shared_ptr<IndexMethodInterface> index_method;

	/* the data of the image file data */
	std::string img_data_path;

	/* the image current level */
	Size img_current_level_size;

	/* the specific image level data */
	std::string img_level_data_path;

	/* the number of cache file number for lru manager */
	size_t file_cache_number;

	/* the lru image files manager */
	ImageFileLRU<Vec3b> lru_image_files;
};

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

template<typename T>
void GiantDiskImage<T>::set_current_level(int level)
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

template<typename T>
size_t GiantDiskImage<T>::get_current_level() const 
{
	return current_level;
}

template<typename T>
bool GiantDiskImage<T>::read_from_index_range(size_t front, size_t tail, ZOrderIndex::IndexType start_index, 
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

template<typename T>
bool GiantDiskImage<T>::check_para_validation(int level, int start_row, int start_col, int rows, int cols) 
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

	return true;
}

template<typename T>
bool GiantDiskImage<T>::get_pixels_by_level(int level, int &start_row, int &start_col,
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

template<typename T>
bool GiantDiskImage<T>::set_pixel_by_level(int level, int start_row, int start_col, 
	int rows, int cols, const std::vector<T> &vec)
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

template<typename T>
bool GiantDiskImage<T>::load_image_head_file(const char* file_name)
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

		/* TODO : this is hierarchical part */
		if(fin.eof()) {
			m_max_level = 0;
			return true;
		}

        /* get the hierarchal image max scale level */
        getline(fin, str);
        index = str.find('=');
        if(index == string::npos || str.substr(0, index) != "maxlevel") {
            cerr << "image format is not correct" << endl;
            return false;
        }
        m_max_level = boost::lexical_cast<size_t>(str.substr(index+1));

        if(fin.eof()) return true;
        if(fin.fail()) return false;

        fin.close();
		return true;

	} catch(boost::bad_lexical_cast &err) {
		cerr << err.what() << endl;
		fin.close();
		return false;
	}
}

//boost::shared_ptr<BlockwiseImage<T, memory_usage> > BlockwiseImage<T, memory_usage>::load_image(const char *file_name)
//{
//	typedef boost::shared_ptr<BlockwiseImage<T, memory_usage> > PtrType;
//	PtrType dst_image(new BlockwiseImage<T, memory_usage> (0, 0));
//	PtrType null_image;
//
//	if(!dst_image->load_image_head_file(file_name))	return null_image;
//
//	bf::path file_path(file_name);
//	bf::path data_path = (file_path.parent_path() / file_path.stem()).make_preferred();
//	data_path /= "level_0";
//	if(!bf::exists(data_path)) {
//		cerr << "image data missing" << endl;
//		return null_image;
//	}
//
//	/* get the new value, then init for using */
//	dst_image->init(dst_image->get_image_rows(), dst_image->get_image_cols());
//
//	ContainerType &img_container = dst_image->img_container;
//	int64 file_number = std::ceil((double)(img_container.size()) / dst_image->file_node_size);
//
//	int64 start_index = 0, file_loop = 0;
//	int64 file_node_shift_num = dst_image->file_node_shift_num;
//	int64 file_node_size = dst_image->file_node_size;
//
//	/* first read the full context files */
//	for(; file_loop < file_number - 1; ++file_loop) {
//		std::ostrstream strstream;
//		strstream << data_path.generic_string() << "/" << file_loop << '\0';
//		if(!bf::exists(bf::path(strstream.str()))) {
//			cerr << "image data missing" << endl;
//			return null_image;
//		}
//
//		/* now read the existing data file */
//		ifstream file_in(strstream.str(), ios::out | ios::binary);
//		if(!file_in.is_open()) {
//			cerr << "open" << strstream.str() << " failure" << endl;
//			return null_image;
//		}
//
//		start_index = (int64)(file_loop) << file_node_shift_num;
//		for(int64 i = 0; i < file_node_size; ++i) {
//			file_in.read(reinterpret_cast<char*>(&img_container[start_index + i]), sizeof(T));
//		}
//		file_in.close();
//	}
//
//	/* now read the last file */
//	start_index = (int64)(file_loop) << file_node_shift_num;
//	std::ostrstream strstream;
//	strstream << data_path.generic_string() << "/" << file_loop << '\0';
//	if(!bf::exists(bf::path(strstream.str()))) {
//		cerr << "image data missing" << endl;
//		return null_image;
//	}
//	ifstream file_in(strstream.str(), ios::out | ios::binary);
//	if(!file_in.is_open()) {
//		cerr << "open" << strstream.str() << " failure" << endl;
//		return null_image;
//	}
//	for(int64 last_index = start_index; last_index < img_container.size(); ++last_index) {
//		file_in.read(reinterpret_cast<char*>(&img_container[last_index]), sizeof(T));
//	}
//	file_in.close();
//
//	return dst_image;
//}

template<typename T>
boost::shared_ptr<GiantDiskImage<T> > load_image(const char *file_name)
{
	typedef boost::shared_ptr<GiantDiskImage<T> > PtrType;
	PtrType dst_image(new GiantDiskImage);
	PtrType null_image;

	if(!dst_image->load_image_head_file(file_name))	 return null_image;

	/* initialization */
	dst_image->set_image_data_path(file_name);
	dst_image->set_mutliply_ways_writing_number(dst_image->get_max_image_level() + 1);
	//dst_image->set_file_cache_number(dst_image->file_cache_number);

	/*
	 * hierarchical image don't save specific level image data
	 * when using set_current_level function to set current level
	 * then using any kind of image access function, the data will be
	 * dynamically loaded, and of course there will be some kind of performance penalty
	 */
	return dst_image;
}

template<typename T>
boost::shared_ptr<GiantDiskImage<T> > load_image(const std::string &file_name)
{
	return load_image<T>(file_name.c_str());
}

template<typename T>
inline void GiantDiskImage<T>::set_image_data_path(const char * file_name) 
{
	/* save the img_data_path */
	bf::path file_path = file_name;
	img_data_path = (file_path.parent_path() / file_path.stem()).generic_string();
}

#endif