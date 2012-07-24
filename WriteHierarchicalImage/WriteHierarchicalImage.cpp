#include "../test/testImageContainter.h"

//#ifdef NDEBUG
//
//#pragma comment(lib, "opencv_highgui240.lib")
//#pragma comment(lib, "opencv_core240.lib")
//#pragma comment(lib, "opencv_contrib240.lib")
//#pragma comment(lib, "opencv_calib3d240.lib")
//#pragma comment(lib, "opencv_imgproc240.lib")
//#pragma comment(lib, "opencv_highgui240.lib")
//
//#else
//
//#pragma comment(lib, "opencv_highgui240d.lib")
//#pragma comment(lib, "opencv_core240d.lib")
//#pragma comment(lib, "opencv_contrib240d.lib")
//#pragma comment(lib, "opencv_calib3d240d.lib")
//#pragma comment(lib, "opencv_imgproc240d.lib")
//#pragma comment(lib, "opencv_highgui240d.lib")
//
//#endif

int main(int argc, char **argv)
{
	test_hierarchical_image(argc, argv);
	return 0;
}