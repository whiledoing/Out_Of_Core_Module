/*
 * 1) read a picture
 * 2) enlarge it to get a big image (test the container ability)
 * 3) input the range information to get the image range data 
 */
bool test_big_image_containter(int argc, char **argv);

/*
 * test the correctness of reading a big image from disk that was wrote by BlockwiseImage or HierarchicalImage
 * input the (*.bigimage) file name, then get the range image data
 */
bool test_read_level_range_image(int argc, char **argv);