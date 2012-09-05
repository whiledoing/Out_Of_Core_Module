#include "DiskBigImage.hpp"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>
#include <string>

/*
 * test the correctness of reading a big image from disk that was wrote by BlockwiseImage or HierarchicalImage
 * input the (*.bigimage) file name, then get the range image data
 */

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

			/* image data was wrote in the format of RGB, but opencv is BRG*/
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