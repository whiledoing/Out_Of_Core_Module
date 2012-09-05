#include <iostream>
#include <string>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "HierarchicalImage.hpp"

#include <boost/assert.hpp>
#include <boost/progress.hpp>
#include <boost/timer.hpp>

using namespace std;

typedef GiantImageInterface<Vec3b> BigImageType;
typedef boost::shared_ptr<BigImageType> BigImagePtr;
typedef DiskBigImage<Vec3b> DiskImageType;
typedef boost::shared_ptr<DiskImageType> DiskImagePtr;
typedef ZOrderIndex ZOrderIndexType;

static BigImagePtr build_enlarged_image(const char *file_name, size_t enlarge_number, size_t memory_usage);

BigImagePtr build_enlarged_image(const char *file_name, size_t enlarge_number, size_t memory_usage)
{
	cv::Mat original_img = cv::imread(file_name);
	if(original_img.empty()) {
		cerr << "Load image error" << endl;
		return BigImagePtr();
	}

	BOOST_ASSERT_MSG(original_img.depth() == CV_8U, "image depth not correct");
	BOOST_ASSERT_MSG(original_img.channels() == 3, "image channels not correct");

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

	//the container to store the enlarged image
	BigImagePtr p_big_image = get_block_wise_image_by_meomory_usage<Vec3b>(memory_usage, large_rows, large_cols, 1, 1, index_method);
	BigImageType &big_image = *p_big_image;
	big_image.set_file_node_size(1024*1024);

	//time related
	boost::progress_display pd(enlarge_number*enlarge_number);
	boost::timer t;
	t.restart();

	//the ZOrder part
	typedef ZOrderIndexType::IndexType IndexType;
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

	return p_big_image;
}

/*
 * 1) read a picture
 * 2) enlarge it to get a big image (test the container ability)
 * 3) input the range information to get the image range data 
 */
bool test_big_image_containter(int argc, char **argv)
{
	if(argc < 5) {
		cout << "Read a image file first, just enlarge the image in both hierarchical and vertical " << endl
			<< "Para : enlarge number (the enlarge number of image in both hierarchical and vertical)" << endl
			<< "Para : memory usage (the maximum memory can be used)" << endl
			<< "Para : show image (whether show the enlarged image, too large is forbidden)" << endl;

		cout << "Usage : " << "[fileName] [enlarge number] [resRow] [resCol] "
			"[memory usage(optional)] [show image(optional)]" << endl;
		return false;
	}

	const char *file_name = argv[1];
	size_t enlarge_number = atoi(argv[2]);
	const int res_row = atoi(argv[3]);
	const int res_col = atoi(argv[4]);
	size_t memory_usage = (argc >= 6) ? (atoi(argv[5])) : 64;
	bool bShowImg = (argc >= 7 && (string(argv[6]) == "1" || string(argv[6]) == "true")) ? true : false;

	BigImagePtr p_big_image = build_enlarged_image(file_name, enlarge_number, memory_usage);
	if(!p_big_image) return false;
	BigImageType &big_image = *p_big_image;

	size_t large_rows = big_image.get_image_rows(), large_cols = big_image.get_image_cols();
	if(bShowImg)
	{
		boost::timer t;

		std::vector<Vec3b> data;
		try {

            big_image.get_pixels(0, 0, big_image.get_image_rows(), big_image.get_image_cols(), data);

            cout << "get the all data from zorder based container " << t.elapsed() << " s " << endl;

            cv::Mat result_image(large_rows, large_cols, CV_8UC3, data.data());
            cv::namedWindow("Enlarged Image");
            cv::imshow("Enlarged Image", result_image);
            cv::waitKey(0);

		} catch (std::exception &err) {
			cerr << err.what() << endl;
			return false;
		}
	}

	cout << "Large Image Size Is : (" << large_rows << "," << large_cols << ")" << endl;

	/* save the input para */
	std::string str_array[4];
	while(1) {
		cout << "Input (quit to exit) : [start_row] [start_col] [rows] [cols]" << endl;

		cin >> str_array[0];
		if(str_array[0] == "quit")	return true;

		cin >> str_array[1] >> str_array[2] >> str_array[3];

		int start_row = atoi(str_array[0].c_str());
		int start_col = atoi(str_array[1].c_str());
		int rows = atoi(str_array[2].c_str());
		int cols = atoi(str_array[3].c_str());

		try{
			std::vector<Vec3b> vec;
			if(!big_image.get_pixels(start_row, start_col, rows, cols, vec))
				continue;

			cv::Mat result_image(rows, cols, CV_8UC3, vec.data());
			cv::namedWindow("RangeArea");
			cv::imshow("RangeArea", result_image);
			cv::waitKey(2000);

		} catch(std::exception &err) {
			cout << err.what() << endl;
			return false;
		}
	}

	return true;
}
