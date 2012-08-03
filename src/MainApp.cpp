#include <stdio.h>

#include "BasicType.h"
#include "HierarchicalImage.hpp"
#include "BlockwiseImage.hpp"
#include "IndexMethod.hpp"
#include "DiskBigImage.hpp"

#include <boost/lexical_cast.hpp>
#include <boost/timer.hpp>
#include <boost/progress.hpp>


bool test_read_level_range_image(int argc, char **argv)
{
	using namespace std;
	if(argc < 2) {
		cout << "Input the big image file name to get level data" << endl;
		cout << "Usage : [image file name] " << endl;
		return false;
	}

	const char *file_name = argv[1];
	boost::shared_ptr<DiskBigImageInterface<Vec3b> > big_image = load_disk_image<Vec3b>(file_name); 
	if(!big_image) {
		cerr << "can't load image " << endl;
		return false;
	}

	/* save the input para */
	std::string str_array[5];
	while(1) {
		cout << "Input (quit to exit) : [image level] [start_row] [start_col] [rows] [cols]" << endl;

		cin >> str_array[0];
		if(str_array[0] == "quit")	return true;

		cin >> str_array[1] >> str_array[2] >> str_array[3] >> str_array[4];

		int level = atoi(str_array[0].c_str());
		int start_row = atoi(str_array[1].c_str());
		int start_col = atoi(str_array[2].c_str());
		int rows = atoi(str_array[3].c_str());
		int cols = atoi(str_array[4].c_str());

		if(!big_image->set_current_level(level)) continue;

		cout << "The level image size is " << "("
			<< big_image->get_current_level_image_rows() << "," 
			<< big_image->get_current_level_image_cols() << ")" << endl;

		try{
			std::vector<Vec3b> vec;
			if(!big_image->get_pixels_by_level(level, start_row, start_col, rows, cols, vec))
				continue;

			cv::Mat result_image(rows, cols, CV_8UC3, vec.data());
			cv::cvtColor(result_image, result_image, CV_RGB2BGR);
			cv::namedWindow("get pixel by level image");
			cv::imshow("get pixel by level image", result_image);
			cv::waitKey(2000);
		} catch(std::exception &err) {
			cout << err.what() << endl;
			return false;
		}
	}

	return true;
}

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

	//writing_blockwise(argc, argv);
	return 0;
}
