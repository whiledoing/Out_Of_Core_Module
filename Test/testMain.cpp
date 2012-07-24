#include "testImageContainter.h"
#include "testIndexMethod.h"
#include <vector>
#include <fstream>
#include "../src/IndexMethod.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <boost/timer.hpp>
#include <boost/lexical_cast.hpp>
#include <strstream>
using namespace std;

int main(int argc, char **argv)
{
	//test_zorder_index(argc, argv);
	//test_block_index(argc, argv);
	//test_big_image_containter(argc, argv);
	//test_write_image(argc, argv);
	//test_read_image(argc, argv);
	//test_read_write_image(argc, argv);
	//test_read_write_enlarged_image(argc, argv);
	//test_hierarchical_image(argc, argv);
	test_read_level_range_image(argc, argv);

	/* just read one file for test */
	//ifstream fin("D:/DunHuang Project/Out_Of_Core/Release/hierarchical/bear/level_3/0", ios::in | ios::binary);
	//if(!fin.is_open()) {
	//	cout << "Can't open file" << endl;
	//	return -1;
	//}

	//size_t rows = 269, cols = 250;
	//ZOrderIndex index_method(rows, cols);
	//std::vector<Vec3b> img_data(index_method.get_max_index() + 1);
	//cout << "Max index is " << index_method.get_max_index() << endl;

	//size_t count = 0;
	//fin.seekg(0, ios::end);
	//cout << "file size is " << fin.tellg() << endl;
	//fin.seekg(0, ios::beg);

	//cout << "Size of Vec3b " << sizeof(Vec3b) << endl;
 //   fin.read((char*)(&img_data[0]), sizeof(Vec3b)*(index_method.get_max_index()+1));
	//cout << fin.gcount() << endl;

	//cout << "Count is " << count << endl;

	//std::vector<Vec3b> vec;
	///* now get back the zorder indexing hierar_data into row-major format vec */
	//vec.resize(rows*cols);
	//for(size_t i = 0; i < rows; ++i) {
	//	IndexMethodInterface::IndexType row_result = index_method.get_row_result(i);
	//	for(size_t j = 0; j < cols; ++j) {
	//		vec[i*cols + j] = img_data[index_method.get_index_by_row_result(row_result, j)];
	//	}
	//}

	//cv::Mat result_image(rows, cols, CV_8UC3, vec.data());
	//cv::namedWindow("get pixel by level image");
	//cv::imshow("get pixel by level image", result_image);
	//cv::waitKey(0);

	return 0;
}