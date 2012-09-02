using namespace std;

extern bool test_read_level_range_image(int argc, char **argv);
extern bool test_big_image_containter(int argc, char **argv);
extern bool test_zorder_index(int argc, char **argv);
extern bool test_block_index(int argc, char **argv);

int main(int argc, char **argv)
{
	//test_zorder_index(argc, argv);
	//test_block_index(argc, argv);
	//test_big_image_containter(argc, argv);
	test_read_level_range_image(argc, argv);

	return 0;
}