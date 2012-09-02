// WriteBlockWiseImage.cpp : Defines the entry point for the console application.
//

#include "../src/BlockwiseImage.hpp"

#include <boost/timer.hpp>
#include <boost/progress.hpp>

/* opencv part */
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#ifdef NDEBUG
#pragma comment(lib, "opencv_highgui240.lib")
#pragma comment(lib, "opencv_core240.lib")
#pragma comment(lib, "opencv_imgproc240.lib")
#else
#pragma comment(lib, "opencv_highgui240d.lib")
#pragma comment(lib, "opencv_core240d.lib")
#pragma comment(lib, "opencv_imgproc240d.lib")
#endif
/*---------------------------------------------*/

bool test_writing_blockwise(int argc, char **argv)
{
	using namespace std;

	if(argc < 6) {
		cout << "Usage : [file name] [res row] [res col] [write image file name] [enlarge number]"
			" [optional (set file cell size)] [optinal (show image)] " << endl;
		return false;
	}

	const char *file_name = argv[1];
	size_t mini_rows = atoi(argv[2]);
	size_t mini_cols = atoi(argv[3]);
	const char *write_image_name = argv[4];
	size_t enlarge_number = atoi(argv[5]);
	size_t file_size = (argc >= 7) ? atoi(argv[6]) : false;
	bool show_image = (argc >= 8) ? atoi(argv[7]) : false;

	cv::Mat original_img = cv::imread(file_name);
	if(original_img.empty()) {
		cerr << "Load image error" << endl;
		return false;
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

	typedef BlockwiseImage<Vec3b, 512> ImageType;
	ImageType big_image(large_rows, large_cols, mini_rows, mini_cols);
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
	big_image.set_file_node_size(file_size*1024*1024);

	if(!big_image.write_image(write_image_name)) return false;
	cout << "Write Hierarchical Image Cost : " << t.elapsed() << " s " << endl;

	if(show_image) {
		const ImageType &c_big_image = big_image;
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

	return true;
}

int main(int argc, char **argv)
{
	test_writing_blockwise(argc, argv);
	return 0;
}

