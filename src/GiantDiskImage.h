#ifndef _GIANTDISK_IMAGE_H
#define _GIANTDISK_IMAGE_H 

#include <HierarchicalInterface.h>
template<typename T>
class GiantDiskImage : public HierarchicalInterface<T>
{

};

boost::shared_ptr<BlockwiseImage<T, memory_usage> > BlockwiseImage<T, memory_usage>::load_image(const std::string &file_name)
{
	return load_image(file_name.c_str());
}

boost::shared_ptr<BlockwiseImage<T, memory_usage> > BlockwiseImage<T, memory_usage>::load_image(const char *file_name)
{
	typedef boost::shared_ptr<BlockwiseImage<T, memory_usage> > PtrType;
	PtrType dst_image(new BlockwiseImage<T, memory_usage> (0, 0));
	PtrType null_image;

	if(!dst_image->load_image_head_file(file_name))	return null_image;

	bf::path file_path(file_name);
	bf::path data_path = (file_path.parent_path() / file_path.stem()).make_preferred();
	data_path /= "level_0";
	if(!bf::exists(data_path)) {
		cerr << "image data missing" << endl;
		return null_image;
	}

	/* get the new value, then init for using */
	dst_image->init(dst_image->get_image_rows(), dst_image->get_image_cols());

	ContainerType &img_container = dst_image->img_container;
	int64 file_number = std::ceil((double)(img_container.size()) / dst_image->file_node_size);

	int64 start_index = 0, file_loop = 0;
	int64 file_node_shift_num = dst_image->file_node_shift_num;
	int64 file_node_size = dst_image->file_node_size;

	/* first read the full context files */
	for(; file_loop < file_number - 1; ++file_loop) {
		std::ostrstream strstream;
		strstream << data_path.generic_string() << "/" << file_loop << '\0';
		if(!bf::exists(bf::path(strstream.str()))) {
			cerr << "image data missing" << endl;
			return null_image;
		}

		/* now read the existing data file */
		ifstream file_in(strstream.str(), ios::out | ios::binary);
		if(!file_in.is_open()) {
			cerr << "open" << strstream.str() << " failure" << endl;
			return null_image;
		}

		start_index = (int64)(file_loop) << file_node_shift_num;
		for(int64 i = 0; i < file_node_size; ++i) {
			file_in.read(reinterpret_cast<char*>(&img_container[start_index + i]), sizeof(T));
		}
		file_in.close();
	}

	/* now read the last file */
	start_index = (int64)(file_loop) << file_node_shift_num;
	std::ostrstream strstream;
	strstream << data_path.generic_string() << "/" << file_loop << '\0';
	if(!bf::exists(bf::path(strstream.str()))) {
		cerr << "image data missing" << endl;
		return null_image;
	}
	ifstream file_in(strstream.str(), ios::out | ios::binary);
	if(!file_in.is_open()) {
		cerr << "open" << strstream.str() << " failure" << endl;
		return null_image;
	}
	for(int64 last_index = start_index; last_index < img_container.size(); ++last_index) {
		file_in.read(reinterpret_cast<char*>(&img_container[last_index]), sizeof(T));
	}
	file_in.close();

	return dst_image;
}
#endif