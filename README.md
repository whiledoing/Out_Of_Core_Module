### Information
**Out Of Core** Module is designed for easy operation on big image(even larger than 10G). 

You can store a very big image in a low memory computer, manipulate the pixels, save it 
into the disk, and even read it back later for observation and operation. 

The Core Design of the module is to support big image operation even in **low memory computer**.

### Additional Libraries
* [boost](http://www.boost.org/)
* [stxxl](http://stxxl.sourceforge.net/)

### Setup
-- build the following sub-library in boost:
   **data_time iostreams filesystem system thread**

-- unzip the stxxl src file, build the static library in both Release and Debug Configuration.
   After the building, copy the library file into Library/stxxl/lib/Debug[Release] directory respectively.

-- using cmake to build the whole project
```
$ mkdir build
$ cd build
$ cmake ..
$ make
```
### Sample Codes
``` C++
/*
 * image_cell_type : the image cell type
 * memory_usage : memory usage for caching
 * large_rows, large_cols : big image size
 * mini_rows, mini_cols : the scaled image size (the minimum resolution image size)
 */
HierarchicalImage<image_cell_type, memory_usage> big_image(large_rows, large_cols, mini_rows, mini_cols);

/* image operation */
big_image(0, 0) = image_cell_type();

/* write back to the disk */
if(!big_image.write_image(write_image_name)) return false;

/* read from the disk */
boost::shared_ptr<DiskBigImageInterface<image_cell_type> > disk_big_image = 
        load_disk_image<image_cell_type>(file_name);

/*
 * get the image data
 * img_level : the image scaled level (you can get different resolution image data by this function)
 * start_row, start_col : start point of the image data area
 * img_rows, img_cols : the size of the image data area
 * img_data : return the image data
 */
disk_big_image->get_pixels_by_level_fast(img_level, start_row, start_col, 
        img_rows, img_cols, img_data);

```
