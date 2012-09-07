#include "OutOfCore/HierarchicalImage.hpp"
#include <climits>

#include <boost/timer.hpp>
#include <boost/progress.hpp>
#include <boost/lexical_cast.hpp>

/* opencv part */
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>
#define print(x) std::cout << #x << " : " << x << std::endl

bool build_image_cake(int argc, const char ** argv)
{
    using namespace std;

    if(argc < 8) {
        cout << "Usage : [image containter rows] [image containter cols] [image dir] \
            [number of files] [mini rows] [mini cols] [write image file name] [file extension]" << endl;
        return false;
    }

    int large_rows = atoi(argv[1]);
    int large_cols = atoi(argv[2]);
    const char *file_dir = argv[3];
    int number_of_images = atoi(argv[4]);
    int mini_rows = atoi(argv[5]);
    int mini_cols = atoi(argv[6]);
    const char *write_image_name  = argv[7];
    const char *extension = (argc > 8) ? argv[8] : "jpg";

    boost::shared_ptr<IndexMethodInterface> index_method =
        boost::make_shared<ZOrderIndex>(large_rows, large_cols);

    //init the config file
    {
        size_t imageBytes = (double)(index_method->get_max_index() * sizeof(Vec3b)) / (1024*1024);
        fstream fout("config.stxxl", ios::out | ios::trunc);
        fout << "disk=d:\\stxxl," << (imageBytes * 1.2) <<",wincall" << endl;
        fout.close();
    }


    HierarchicalImage<Vec3b, 512> big_image(large_rows, large_cols, mini_rows, mini_cols);
    big_image.set_mutliply_ways_writing_number(big_image.get_max_image_level()+1);
    cout << "mini_rows " << big_image.get_minimal_image_rows() << endl;
    cout << "mini_cols " << big_image.get_minimal_image_cols() << endl;
    cout << "max_level " << big_image.get_max_image_level() << endl;

    boost::timer t;

    int current_file_number = 0;
    std::string current_file_name = std::string(file_dir) 
        + "/" + boost::lexical_cast<std::string>(current_file_number) + "." + extension;
    current_file_number = (current_file_number+1)%(number_of_images);

    cv::Mat img_data = cv::imread(current_file_name);
    if(img_data.empty()) {
        cerr << "Load image error" << endl;
        return false;
    }
    print(current_file_name);

    cv::cvtColor(img_data, img_data, CV_BGR2RGB);

    int img_rows = img_data.rows;
    int img_cols = img_data.cols;

    Vec3b white;
    white.r = white.g = white.b = 255;

    /* fill the image with white color */
    for(int row = 0; row < large_rows; ++row) {
        for(int col = 0; col < large_cols; ++col) {
            big_image(row, col) = white;
        }
    }

    int current_max_row = -1;
    for(int start_row = 0; start_row + img_rows <= large_rows;) {

        current_max_row = -1;
        for(int start_col = 0; start_col + img_cols <= large_cols;) {

            /* copy the image file into the (start_rows, start_cols) pos in the big containter */
            for(int row = 0, big_row = start_row; row < img_rows; ++row, ++big_row) {
                for(int col = 0, big_col = start_col; col < img_cols; ++col, ++big_col) {
                    big_image(big_row, big_col) = *(Vec3b*)(img_data.data + row*img_data.step[0] + col*img_data.step[1]);
                }
            }

            if(img_rows > current_max_row)  current_max_row = img_rows;

            start_col += img_cols;

            /* get a new image */
            current_file_name = std::string(file_dir) 
                + "/" + boost::lexical_cast<std::string>(current_file_number) + "." + extension;
            current_file_number = (current_file_number+1)%(number_of_images);
            print(current_file_name);

            img_data = cv::imread(current_file_name);
            if(img_data.empty()) {
                cerr << "Load image error" << endl;
                return false;
            }
            cv::cvtColor(img_data, img_data, CV_BGR2RGB);

            /* change the new image size info */
            img_rows = img_data.rows;
            img_cols = img_data.cols;

            //if(start_col + img_cols > large_cols) {
            //    for(int row = 0, big_row = start_row; row < current_max_row; ++row, ++big_row) {
            //        for(int col = 0, big_col = start_col; col < (large_cols - start_col); ++col, ++big_col) {
            //            big_image(big_row, big_col) = white;
            //        }
            //    }
            //    break;
            //}
        }

        print(current_max_row);
        print(start_row);
        print(img_rows);
        print(img_cols);
        std::cout << "--------------------------------------------" << std::endl;

        start_row += current_max_row;
        //if(start_row + img_rows > large_rows) {
        //    for(int big_row = start_row; big_row < large_rows; ++big_row) {
        //        for(int big_col = 0; big_col < large_cols; ++big_col) {
        //            big_image(big_row, big_col) = white;
        //        }
        //    }
        //    break;
        //}
    }

    cout << "Build Enlarged Image Cost Time : " << t.elapsed() << " s " << endl;

    /* 6M per file */
    big_image.set_file_node_size(6*1024*1024);

    t.restart();
    if(!big_image.write_image(write_image_name)) return false;
    cout << "Write Hierarchical Image Cost : " << t.elapsed() << " s " << endl;

    return true;
}

int main(int argc, char const** argv)
{
    build_image_cake(argc, argv);
    return 0;
}
