#include <stdio.h>

#include "DataType.h"
#include "HierarchicalImage.hpp"
#include "BlockwiseImage.hpp"
#include "IndexMethod.hpp"
#include "Lru.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/timer.hpp>

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

	while(1){
		string str;
		getline(cin, str);
	}
	return 0;
}