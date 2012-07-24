#ifndef _LRU_H
#define _LRU_H

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
		ValueType(std::string _image_file_name, size_t data_size)
			: image_file_name(_image_file_name), count(0)
		{
			image_data.resize(data_size);
		}

		int count;
		std::string image_file_name;
		std::vector<T> image_data;
	};

	typedef std::vector<ValueType> DataType;

public:
	void init(size_t _file_cell_numbers, size_t _file_cache_number)
	{
		BOOST_ASSERT(_file_cache_number > 0);
		file_cell_numbers = _file_cell_numbers;
		total_number = _file_cache_number;
		current_used = 0;
	}

	ImageFileLRU(size_t _file_cell_numbers = 0, size_t _file_cache_number = 16) {
		init(_file_cell_numbers, _file_cache_number);
	}

	bool exists(const std::string &file_name) const {
		for(DataType::const_iterator ite = lru_data.cbegin(); ite != lru_data.cend(); ++ite) {
			if(ite->image_file_name == file_name)
				return true;
		}
		return false;
	}

	int find(const std::string &file_name) const {
		for(DataType::const_iterator ite = lru_data.cbegin(); ite != lru_data.cend(); ++ite) {
			if(ite->image_file_name == file_name)
				return (ite - lru_data.cbegin());
		}
		return npos;
	}

	int put_into_lru(const std::string &file_name) {
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
		if(current_used < total_number) {
			lru_data.push_back(ValueType(file_name, file_cell_numbers));
			index = current_used++;
		} else {		/* remove one of the last not used data in cache */
			int max_number = -1;

			/* the last number count is the most not used data */
			for(size_t i = 0; i < total_number; ++i) {
				if(lru_data[i].count > max_number)	{
					max_number = lru_data[i].count;
					index = i;
				}
			}

			/* put the new data in the remove index, the data will be covered by the new data read from file */
			lru_data[index].image_file_name = file_name;
		}

		/* read the data into cache */
		std::vector<T> &data = lru_data[index].image_data;
		fin.read(reinterpret_cast<char*>(&data[0]), file_cell_numbers*sizeof(T));

		fin.close();
		update_count(index);
		return index;
	}

	void update_count(size_t index) {
		BOOST_ASSERT(index < lru_data.size());
		for(size_t i = 0; i < lru_data.size(); ++i) {
			++lru_data[i].count;
		}

		/* the most current used is index by 0 */
		lru_data[index].count = 0;
	}

	const std::vector<T>& get_data(int index) const {
		return lru_data[index].image_data;
	}

public:
	/* the npos means some kind of invalid index */
	static const size_t npos = -1;

private:
	std::vector<ValueType> lru_data;
	size_t current_used;
	size_t total_number;
	size_t file_cell_numbers;
};

#endif