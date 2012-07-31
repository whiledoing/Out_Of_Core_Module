#include <stdio.h>

#include "DataType.h"
#include "HierarchicalImage.hpp"
#include "BlockwiseImage.hpp"
#include "IndexMethod.hpp"
#include "Lru.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/timer.hpp>
#include <boost/progress.hpp>

int main(int argc, char **argv)
{
	//if(argc < 2) {
	//	cerr << "Usage : [file path]" << endl;
	//	return -1;
	//}
	//ImageFileLRU<Vec3b> lru(1024*1024, 4);

	//boost::timer t;
	//string path = argv[1];
	//for(size_t i = 0; i < 100; ++i) {
	//	string file_name = path + '/' + boost::lexical_cast<string>(rand() % 20);
	//	cout << file_name << endl;
	//	lru.put_into_lru(file_name);
	//}
	//cout << "Cose Timer " << t.elapsed() << " s" << endl;

	//HierarchicalImage<Vec3b> image(1000, 1000, 10, 10);

	//image.load_image("../Release/bigimage/bear.bigimage");
	//std::vector<Vec3b> vec;
	//size_t start_row, start_col, rows, cols;
	//image.get_pixels_by_level(0, start_row, start_col, rows, cols, vec);

	//lru.put_into_lru("../Release/bigimage/bear/level_0/0");
	//lru.put_into_lru("../Release/bigimage/bear/level_0/1");
	//lru.put_into_lru("../Release/bigimage/bear/level_0/2");
	//lru.put_into_lru("../Release/bigimage/bear/level_0/2");
	//lru.put_into_lru("../Release/bigimage/bear/level_0/3");
	//lru.put_into_lru("../Release/bigimage/bear/level_0/4");
	//lru.put_into_lru("../Release/bigimage/bear/level_0/5");
	//lru.put_into_lru("../Release/bigimage/bear/level_0/1");
	//lru.put_into_lru("../Release/bigimage/bear/level_0/5");

	if(argc < 7) {
		cout << "Usage : [file name] [res row] [res col] [write image file name] [multiply ways number] [enlarge number]"
			" [optinal (show image)] " << endl;
		return -1;
	}

	const char *file_name = argv[1];
	size_t mini_rows = atoi(argv[2]);
	size_t mini_cols = atoi(argv[3]);
	const char *write_image_name = argv[4];
	size_t merge_number = atoi(argv[5]);
	size_t enlarge_number = atoi(argv[6]);
	bool show_image = (argc >= 8) ? atoi(argv[7]) : false;

	cv::Mat original_img = cv::imread(file_name);
	if(original_img.empty()) {
		cerr << "Load image error" << endl;
		return -1;
	}

	BOOST_ASSERT_MSG(original_img.depth() == CV_8U, "image depth not correct");
	BOOST_ASSERT_MSG(original_img.channels() == 3, "image channels not correct");

	cv::cvtColor(original_img, original_img, CV_BGR2RGB);

	size_t rows = original_img.rows, cols = original_img.cols;
	size_t large_rows = rows * enlarge_number, large_cols = cols * enlarge_number;

	boost::shared_ptr<IndexMethodInterface> index_method = boost::make_shared<ZOrderIndex>(large_rows, large_cols);
	//init the config file
	{
		size_t imageBytes = (double)(index_method->get_max_index() * sizeof(Vec3b)) / (1024*1024);
		fstream fout("config.stxxl", ios::out | ios::trunc);
		fout << "disk=d:\\stxxl," << (imageBytes * 2) <<",wincall" << endl;
		fout.close();
	}

	HierarchicalImage<Vec3b, 512> big_image(large_rows, large_cols, mini_rows, mini_cols);
	big_image.set_mutliply_ways_writing_number(merge_number);
	cout << "mini_rows " << big_image.get_minimal_image_rows() << endl;
	cout << "mini_cols " << big_image.get_minimal_image_cols() << endl;
	cout << "max_level " << big_image.get_max_image_level() << endl;

	//time related
	boost::progress_display pd(enlarge_number*enlarge_number);
	boost::timer t;
	t.restart();

	//the ZOrder part
	typedef ZOrderIndex::IndexType IndexType;
	for(IndexType outI = 0; outI < enlarge_number; ++outI) {
		for(IndexType outJ = 0; outJ < enlarge_number; ++outJ) {
			IndexType startI = outI * rows;
			IndexType startJ = outJ * cols;
			for(IndexType i = 0; i < rows; ++i) {
				IndexType I = startI + i;
				for(IndexType j = 0; j < cols; ++j) {
					IndexType J = startJ + j;
					big_image(I, J) = *(Vec3b*)(original_img.data + i*original_img.step[0] + j*original_img.step[1]);
				}
			}
			++pd;
		}
	}
	cout << "Build Enlarged Image Cost Time : " << t.elapsed() << " s " << endl;

	t.restart();

	/* 6M per file */
	big_image.set_file_node_size(6*1024*1024);

	if(!big_image.write_image(write_image_name)) return -1;
	cout << "Write Hierarchical Image Cost : " << t.elapsed() << " s " << endl;

	if(show_image) {
		const HierarchicalImage<Vec3b, 512> &c_big_image = big_image;
		std::vector<Vec3b> vec(large_rows*large_cols);
		for(IndexType i = 0; i < big_image.get_image_rows(); ++i) {
			for(IndexType j = 0; j < big_image.get_image_cols(); ++j) {
				vec[i*large_cols  + j] = c_big_image(i,j);
			}
		}

		cv::Mat result_image(large_rows, large_cols, CV_8UC3, vec.data());
		cv::cvtColor(result_image, result_image, CV_RGB2BGR);
		cv::namedWindow("hierarchical image");
		cv::imshow("hierarchical image", result_image);
		cv::waitKey(0);
	}

	return 0;
}