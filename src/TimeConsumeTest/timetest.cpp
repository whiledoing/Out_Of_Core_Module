#include "IndexMethod.hpp"
#include <boost/timer.hpp>
#include <iostream>
#include <vector>
#include <boost/format.hpp>
#include <fstream>
using namespace std;

#define PRINTPARA(x) std::cout << #x << " : " << x << std::endl

inline void print_counted_time(const boost::timer &t, const char *msg)
{
	std::cout << msg << " Cost Time : " << t.elapsed() << " s " << std::endl;
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

void test_make_upper_fout_multiply() {
	for(int i = 0; i < 100; ++i)
		cout << i << "    " << make_upper_four_multiply(i) << endl;
}

class LoggingFile
{
	std::streambuf *old_buffer;
	ofstream fout;
	ostream &original_os;
public:
	LoggingFile(ostream &os, const char *file_name) : original_os(os), old_buffer(NULL) {
		fout.open(file_name);
		if(!fout.is_open()) return;

		old_buffer = os.rdbuf(fout.rdbuf());
	}
	~LoggingFile() {
		if(fout.is_open()) fout.close();
		if(old_buffer != NULL)	original_os.rdbuf(old_buffer);
	}
};

int test_normal_image_time_cost(size_t start_rows, size_t start_cols, size_t rows, size_t cols,
	size_t total_rows, size_t total_cols)
{
	if(start_rows + rows >= total_rows || start_cols + cols >= total_cols)
		cerr << "para not correct" << endl;

	PRINTPARA(start_rows);
	PRINTPARA(start_cols);
	PRINTPARA(rows);
	PRINTPARA(cols);
	PRINTPARA(total_rows);
	PRINTPARA(total_cols);

	boost::timer t;

	ZOrderIndex zorder(total_rows, total_cols);
	ZOrderIndex::IndexType start_index = zorder.get_index(start_rows, start_cols);
	ZOrderIndex::IndexType end_index = zorder.get_index(start_rows + rows - 1, start_cols + cols - 1);
	std::vector<bool> vec(end_index - start_index + 1);

	PRINTPARA(start_index);
	PRINTPARA(end_index);
	cout << "vec size "; PRINTPARA((end_index - start_index + 1));

	for(size_t i = start_rows; i < start_rows + rows; ++i) {
		for(size_t j = start_cols; j < start_cols + cols; ++j) {
			vec[zorder.get_index(i, j) - start_index] = true;
		}
	}

	print_counted_time(t, "get the rows and cols zorder index");

	boost::format fmt("%s : (%d, %d) distance is : %d ");

	std::vector<bool>::size_type front = 0, tail = 0, end = vec.size();
	size_t count = 0;
	while(1)
	{
		while(front < end) {
			//find first true index
			if(vec[front])	break;
			++front;
		}
		if(front == end) break;

		tail = front + 1;
		while(tail < end) {
			//find first not true index
			if(!vec[tail]) break;
			++tail;
		}
		if(tail == end) break;

		//then the index range is [front, tail)
		//cout << fmt % "The index range is " % front % tail % (tail - front) << endl;
		++count;

		front = tail + 1;
	}

	if(front == end);	//do nothing
	if(tail == end) {
		//cout << fmt % "The index range is " % front % tail % (tail - front) << endl;
		++count;
	}

	cout << "The total range is " << count << endl;
	return count;
};

int main(int argc, char **argv)
{
	{
		if(argc < 5) {
			cout << "Usage : [start_rows] [start_cols] [rows] [cols]" << endl;
			return -1;
		}

		size_t start_rows = atoi(argv[1]);
		size_t start_cols = atoi(argv[2]);
		size_t rows = atoi(argv[3]);
		size_t cols = atoi(argv[4]);

		int count1, count2;
		{
			//LoggingFile temp(cout, "log.txt");

			count1 = test_normal_image_time_cost(start_rows, start_cols, rows, cols, 43200, 76800);

			{
				/*
				 * keep the start_rows and start_cols not changed method
				size_t temp_start_rows = make_upper_four_multiply(start_rows);
				size_t temp_start_cols = make_upper_four_multiply(start_cols);

				int delta_rows = rows - (temp_start_rows - start_rows);
				int delta_cols = cols - (temp_start_cols - start_cols);

				if(delta_rows > 0) {
					rows = make_upper_four_multiply(delta_rows) + (temp_start_rows - start_rows);
					cols = make_upper_four_multiply(delta_cols) + (temp_start_cols - start_cols);
				}
				*/

				start_rows = make_less_four_multiply(start_rows);
				start_cols = make_less_four_multiply(start_cols);
				rows = make_less_four_multiply(rows);
				cols = make_less_four_multiply(cols);
				count2 = test_normal_image_time_cost(start_rows, start_cols, rows, cols, 43200, 76800);
			}
		}

		cout << "count distance is " << count1 - count2 << endl;
	}

	//test_make_upper_fout_multiply();
	return 0;
}