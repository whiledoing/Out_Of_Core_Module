#include "testImageContainter.h"

#include <iostream>
#include <string>
#include <intsafe.h>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "../src/HierarchicalImage.hpp"

#include <boost/assert.hpp>
#include <boost/progress.hpp>
#include <boost/timer.hpp>

using namespace std;

/*
 * 1) read a picture
 * 2) enlarge it (test the container ability)
 * 3) smaller it for seen
 */
typedef GiantImageInterface<Vec3b> BigImageType;
typedef ZOrderIndex ZOrderIndexType;

static void get_level_res_image(const BigImageType &origin_big_image, int level);
static bool build_enlarged_image(const char *file_name, size_t enlarge_number, BigImageType **p_big_image);

size_t get_least_order_number(size_t number)
{
	while(number & (number - 1)) {
		number &= (number - 1);
	}
	int count = 0;
	while(number) {
		++count;
		number >>= 1;
	}

	return count;
}

bool build_enlarged_image(const char *file_name, size_t enlarge_number, BigImageType **p_big_image)
{
	cv::Mat original_img = cv::imread(file_name);
	if(original_img.empty()) {
		cerr << "Load image error" << endl;
		return false;
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
	*p_big_image = new BlockwiseImage<Vec3b, 512>(large_rows, large_cols, index_method);
	BigImageType &big_image = **p_big_image;
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
}

bool test_big_image_containter(int argc, char **argv)
{
	if(argc < 5) {
		cout << "Read a image file first, just enlarge the image in both hierarchical and vertical " << endl
			<< "Para : enlarge number (the enlarge number of image in both hierarchical and vertical)" << endl
			<< "Para : show image (whether show the enlarged image, too large is forbidden" << endl
			<< "Para : memory usage (the maximum memory can be used)" << endl;

		cout << "Usage : " << "[fileName] [enlarge number] [resRow] [resCol]"
			"[show image(optional)] [memory usage(optional)]" << endl;
		return false;
	}

	const char *file_name = argv[1];
	size_t enlarge_number = atoi(argv[2]);
	const int res_row = atoi(argv[3]);
	const int res_col = atoi(argv[4]);
	bool bShowImg = (argc == 6 && (string(argv[5]) == "1" || string(argv[5]) == "true")) ? true : false;
	size_t memory_usage = (argc == 7) ? (atoi(argv[6])) : 64;

	BigImageType *p_big_image = NULL;
	if(!build_enlarged_image(file_name, enlarge_number, &p_big_image))	return false;
	BigImageType &big_image = *p_big_image;

	size_t large_rows = big_image.get_image_rows(), large_cols = big_image.get_image_cols();
	if(bShowImg)
	{
		boost::timer t;
		std::vector<Vec3b> data;
		big_image.get_pixel(0, big_image.get_image_rows() - 1, 0, big_image.get_image_cols() - 1, data);
		cv::Mat result_image(large_rows, large_cols, CV_8UC3, data.data());
		cout << "get the all data from zorder based container" << t.elapsed() << " s " << endl;

		cv::namedWindow("Enlarged Image");
		cv::imshow("Enlarged Image", result_image);
		cv::waitKey(0);
	}

	size_t level_row = large_rows / res_row, level_col = large_cols / res_col;
	level_row = get_least_order_number(level_row);
	level_col = get_least_order_number(level_col);

	cout << "Large Image Size Is : (" << large_rows << "," << large_cols << ")" << endl;
	get_level_res_image(big_image, std::min(level_col, level_row));

	delete p_big_image;
	return true;
}

void get_level_res_image(const BigImageType &origin_big_image, int level)
{
	int row_size = origin_big_image.get_image_rows();
	int col_size = origin_big_image.get_image_cols();

	typedef ZOrderIndexType::IndexType IndexType;
	typedef ZOrderIndexType::RowMajorIndexType RowMajorIndexType;

	const int res_row_size = std::ceil((double)(row_size) / (1 << (level-1)));
	const int res_col_size = std::ceil((double)(col_size) / (1 << (level-1)));

	cout << "Hierarchical Image Size Is : (" << res_row_size << "," << res_col_size << ")" << endl;

	ZOrderIndexType res_zorder_object(res_row_size, res_col_size);

	/* level == 1 : 表示不缩放
	* level == 2 : 表示缩放两倍
	* level == 3 : 表示缩放四倍
	* 每个level导致最后的zorder数据要乘以4^(level-1)
	*/
	IndexType max_zorder_index = origin_big_image.get_index_method()->get_max_index();

	/* 右小角的最大zorder数值，在移位一下2*(level-1)，表示除以这个2^(2*(level-1)就是4^(level-1)
	* 最后加一表示总共有多少个zorder数据需要保存（从0开始统计）
	*/
	IndexType shift_num = (2*(level-1));
	IndexType zorder_count = (max_zorder_index >> shift_num) + 1;

#ifdef LOGGING
	ofstream logfile;
    logfile.open("log.txt");
    if(!logfile.is_open()) return ;
    logfile << "max index of big image " << origin_big_image.get_index_method()->get_max_index() << endl;
    logfile << "get hierarchical data index" << endl;
#endif

	/* 保存hierarchy之后的数据 */
	std::vector<Vec3b> hierar_data(zorder_count);

	/* vip : implict bug here, even i is the index of std::vector, but should notice that the calculation
	 * (i << shift_num) will be overflow in larger i
	 */
	boost::timer t;
	t.restart();
	for(IndexType i = 0; i < zorder_count; ++i) {
		hierar_data[i] = origin_big_image.at(i << shift_num);
#ifdef LOGGING
        logfile << (i << shift_num) << endl;
#endif
	}
	cout << "get the hierarchical image cost " << t.elapsed() << " s" <<endl;

	/* 将数据重新隐射到row-major的数据中 */
	std::vector<Vec3b> back_image(res_col_size * res_row_size);

	/* hierar_data的最大索引值加一（从0开始）的数目 == 总体元素的个数 */
	BOOST_ASSERT((max_zorder_index = res_zorder_object.get_max_index() + 1 == zorder_count));

	t.restart();
	for(RowMajorIndexType i = 0; i < res_row_size; ++i) {
		for(RowMajorIndexType j = 0; j < res_col_size; ++j) {
			back_image[i*res_col_size + j] = hierar_data[res_zorder_object.get_index(i, j)];
		}
	}
	cout << "using row-major loop to get the hierarchical image cost " << t.elapsed() << " s" <<endl;

	back_image.clear();
	for(IndexType i = 0; i < hierar_data.size(); ++i) {
		RowMajorPoint point = res_zorder_object.get_origin_index(i);
		if(point.row < res_row_size && point.col < res_col_size)
            back_image[point.row*res_col_size + point.col] = hierar_data[i];
	}
	cout << "using zorder index loop to get the hierarchical image cost " << t.elapsed() << " s" << endl;

	cv::Mat show_image(res_row_size, res_col_size, CV_8UC3, back_image.data());
	cv::namedWindow("Hierarchical Image");
	cv::imshow("Hierarchical Image", show_image);
	cv::waitKey(0);
}

bool test_write_image(int argc, char **argv)
{
	if(argc < 3) {
		cout << "Usage : [file_name] [write image name]" << endl;
		return false;
	}

	cv::Mat image = cv::imread(argv[1]);
	if(image.empty()) {
		cerr << "Load image error" << endl;
		return false;
	}

	typedef ZOrderIndex::IndexType IndexType;

	BigImageType &big_image = BlockwiseImage<Vec3b>(image.rows, image.cols);

	for(IndexType row = 0; row < image.rows; ++row) {
		for(IndexType col = 0; col < image.cols; ++col) {
			big_image(row, col) = *(Vec3b*)(image.data + row*image.step[0] + col*image.step[1]);
		}
	}

	return big_image.write_image(argv[2]);
}

bool test_read_image(int argc, char **argv) {
	if(argc < 2) {
		cout << "Usage : [file name]" << endl;
		return false;
	}

	boost::shared_ptr<BlockwiseImage<Vec3b> > dst_image = BlockwiseImage<Vec3b>::load_image(argv[1]);
	if(!dst_image)	return false;
	BigImageType &big_image = *dst_image.get();

	/* if image is bigger than 128M, then don't show it */
	if(big_image.get_image_rows() * big_image.get_image_cols() * sizeof(Vec3b) > 1024*1024*128)
		return true;

	/* now the image has been read from file , just let it show the result */
	std::vector<Vec3b> data;

	/*get the full data */
	big_image.get_pixel(0, big_image.get_image_rows() - 1, 0, big_image.get_image_cols() - 1, data);

	cv::Mat result_image(big_image.get_image_rows(), big_image.get_image_cols(), CV_8UC3, data.data());

	cv::namedWindow("Read big Image");
	cv::imshow("Read big Image", result_image);
	cv::waitKey(0);

	return true;
}

bool test_read_write_image(int argc, char **argv) {
	if(argc < 3) {
		cout << "Read the image into big image container and then write into specific files" << endl;
		cout << "Usage : [file_name] [write image name]" << endl;
		return false;
	}

	if(!test_write_image(argc, argv)) return false;
	argc = 2; argv[1] = argv[2];
	return test_read_image(argc, argv);
}

bool test_read_write_enlarged_image(int argc, char **argv) {
	if(argc < 4) {
		cout << "Read the image into memory, and enlarge the image into big image container before write into specific files" << endl;
		cout << "Usage : [file_name] [write image name] [enlarge number]" << endl;
		return false;
	}

	const char *file_name = argv[1];
	const char *write_image_name = argv[2];
	size_t enlarge_number = atoi(argv[3]);

	BigImageType *p_big_image = NULL;
	if(!build_enlarged_image(file_name, enlarge_number, &p_big_image))	return false;
	BigImageType &big_image = *p_big_image;

	boost::timer t;
	if(big_image.write_image(write_image_name))
		cout << "Write Image Cost Time : " << t.elapsed() << " s " << endl;
	else {
		cout << "write image failure" << endl;
		return false;
	}

	t.restart();
	argc = 2; argv[1] = argv[2];
	if(test_read_image(argc, argv))
		cout << "Read Image Cost Time : " << t.elapsed() << " s " << endl;
	else {
		cout << "read image failure" << endl;
		return false;
	}

	delete p_big_image;
	return true;
}

bool test_hierarchical_image(int argc, char **argv)
{
	if(argc < 7) {
		cout << "Usage : [file name] [res row] [res col] [write image file name] [multiply ways number] [enlarge number]"
			" [optinal (show image)] " << endl;
		return false;
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

	t.restart();

	/* 6M per file */
	big_image.set_file_node_size(6*1024*1024);

	if(!big_image.write_image(write_image_name)) return false;
	cout << "Write Hierarchical Image Cost : " << t.elapsed() << " s " << endl;

	if(show_image) {
		const BigImageType &c_big_image = big_image;
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

bool test_read_level_range_image(int argc, char **argv)
{
	if(argc < 2) {
		cout << "Input the big image file name to get level data" << endl;
		cout << "Usage : [image file name] " << endl;
		return false;
	}

	const char *file_name = argv[1];
	boost::shared_ptr<DiskImageInterface<Vec3b> > big_image = 
		load_hierarchical_image_by_meomory_usage<Vec3b>(512, file_name);
    //boost::shared_ptr<DiskImageInterface<Vec3b> > big_image =
    //    HierarchicalImage<Vec3b, 512>::load_image(file_name);

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

		big_image->set_current_level(level);
		cout << "The level image size is " << "("
			<< reinterpret_cast<HierarchicalImage<Vec3b, 512>*>(big_image.get())->get_current_level_image_rows()
			<< ","
			<<reinterpret_cast<HierarchicalImage<Vec3b, 512>*>(big_image.get())->get_current_level_image_cols() << ")" << endl;

		try{
			std::vector<Vec3b> vec;
			if(!big_image->get_pixels_by_level(level, start_row, start_col, rows, cols, vec))
				continue;

			cv::Mat result_image(rows, cols, CV_8UC3, vec.data());
            cv::namedWindow("get pixel by level image");
			cv::imshow("get pixel by level image", result_image);
			cv::waitKey(1000);
		} catch(std::exception &err) {
			cout << err.what() << endl;
			return false;
		}
	}

	return true;
}