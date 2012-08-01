#ifndef _LRU_HPP
#define _LRU_HPP 

#include <list>
#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>

#include <boost/assert.hpp>

template<typename T>
class ImageFileLRU
{
private:
	struct ValueType
	{
		ValueType(std::string _image_file_name, int _file_cell_numbers)
			: image_file_name(_image_file_name), count(0)
		{
			image_data.resize(_file_cell_numbers);
		}

		int count;
		std::string image_file_name;
		std::vector<T> image_data;
	};

	typedef std::vector<ValueType> DataType;

public:

	/*
	 *	@brief : initialize the lru manager
	 *	@para _file_cell_numbers : the cell number in one file
	 *	@para _file_cache_numbers : the file cache number
	 */
	void init(int _file_cell_numbers, int _file_cache_numbers)
	{
		BOOST_ASSERT(_file_cache_numbers > 0);
		file_cell_numbers = _file_cell_numbers;
		file_cache_numbers = _file_cache_numbers;
		current_used = 0;
		b_data_dirty.resize(file_cache_numbers, false);
	}

	ImageFileLRU(int _file_cell_numbers = 0, int _file_cache_numbers = 16) {
		init(_file_cell_numbers, _file_cache_numbers);
	}

	~ImageFileLRU() {
		/* write back the dirty image */
		for(size_t i = 0; i < lru_data.size(); ++i) {
			if(b_data_dirty[i] == true)
				write_back_data(i);
		}
	}

	/*
	 *	@brief : checks whether the file_name is in the file cache
	 */
	bool exists(const std::string &file_name) const {
		for(DataType::const_iterator ite = lru_data.cbegin(); ite != lru_data.cend(); ++ite) {
			if(ite->image_file_name == file_name)
				return true;
		}
		return false;
	}

	/*
	 *	@brief : find the index of the file_name in the lru manager, if not exist, return npos
	 */
	int find(const std::string &file_name) const {
		for(DataType::const_iterator ite = lru_data.cbegin(); ite != lru_data.cend(); ++ite) {
			if(ite->image_file_name == file_name)
				return (ite - lru_data.cbegin());
		}
		return npos;
	}

	/*
	 *	@brief : put the file_name into the lru manager, and return the index of the specific file_name in the 
	 *	lru manager.
	 */
	int put_into_lru(const std::string &file_name) {
		using namespace std;

		int index = find(file_name);

		/* value is in the lru caches, just return the index */
		if(index != npos)	{
			update_count(index);
			return index;
		}

		ifstream fin(file_name, ios::in | ios::binary);
		if(!fin.is_open()) {
			cerr << "can't open file " << file_name << endl;
			return npos;
		}

		/* the data cache is not full */
		if(current_used < file_cache_numbers) {
			lru_data.push_back(ValueType(file_name, file_cell_numbers));
			index = current_used++;
		} else {		/* remove one of the last not used data in cache */
			int max_number = -1;

			/* the last number count is the most not used data */
			for(size_t i = 0; i < file_cache_numbers; ++i) {
				if(lru_data[i].count > max_number)	{
					max_number = lru_data[i].count;
					index = i;
				}
			}

			if(!write_back_data(index)) return npos;

			/* put the new data in the remove index, the data will be covered by the new data read from file */
			lru_data[index].image_file_name = file_name;
		}

		/* read the data into cache */
		std::vector<T> &data = lru_data[index].image_data;
		fin.read(reinterpret_cast<char*>(&data[0]), file_cell_numbers*sizeof(T));
		if(!fin.eof() && fin.fail()) {
			cerr << "read image file " << lru_data[index].image_file_name << " fails" << endl;
			return npos;
		}

		fin.close();
		update_count(index);
		return index;
	}

	/*
	 *	@brief : write the index data into the file system
	 */
	bool write_back_data(int index) 
	{
		using namespace std;

		/* if the data is dirty, then write it back to the file to update the data in the disk */
		if(b_data_dirty[index] == true) {
			ofstream fout(lru_data[index].image_file_name, ios::out | ios::binary);
			fout.write(reinterpret_cast<char*>(&lru_data[index].image_data[0]), file_cell_numbers*sizeof(T));

			if(fout.fail()) {
				cerr << "write image file " << lru_data[index].image_file_name << " fails" << endl;
				return false;
			}
			fout.close();
		}

		return true;
	}

	/*
	 *	@brief : update the count of all the file cache count
	 */
	void update_count(int index) {
		BOOST_ASSERT(index < lru_data.size() && index >= 0);
		for(size_t i = 0; i < lru_data.size(); ++i) {
			++lru_data[i].count;
		}

		/* the most current used is index by 0 */
		lru_data[index].count = 0;
	}

	/*
	 *	@brief : get the index file cache's const data
	 */
	const std::vector<T>& get_const_data(int index) const {
		BOOST_ASSERT(index < lru_data.size() && index >= 0);
		return lru_data[index].image_data;
	}

	/*
	 *	@brief : get the index file cache's data
	 */
	std::vector<T>& get_data(int index) {
		BOOST_ASSERT(index < lru_data.size() && index >= 0);
		
		/* if get the image data by this function, then the data will be marked as dirty */
		b_data_dirty[index] = true;
		return lru_data[index].image_data;
	}

public:
	/* the npos means invalid index */
	static const int npos = -1;

private:
	std::vector<ValueType> lru_data;
	std::vector<bool> b_data_dirty;
	size_t current_used;
	size_t file_cache_numbers;
	size_t file_cell_numbers;
};

#endif