#ifndef _MAINPAGE_H_
#define _MAINPAGE_H_

/**
 * @mainpage Out Of Core Image Processing Liabary
 * 
 * @section intro_sec 介绍
 * 通常的图像处理是基于内存进行的，但在图像数据非常大导致无法将数据全部装入内存处理的时候，基于内存的处理方法就会失效。\n
 * Out Of Core技术就是用来解决这个问题。当数据很大的时候，实际的数据存放在硬盘上，在需要获取或者处理数据的时候，
 *再动态的从磁盘中加载数据到内存进行处理。\n
 *
 * 主要实现了下面几个类：
 * - BlockwiseImage 可以容纳大型的数据数据储存，提供对图像处理的通用性接口，同时可以将图像数据写入到磁盘中以备后续处理。
 * - HierarchicalImage 继承自BlockwiseImage，提供一致的图像处理接口，但在保存到磁盘时，将提取不同的层级的图像数据并写入磁盘，这样方便以后提取
 *不同分辨率下的图像数据。
 * - DiskBigImage 用来对由BlockwiseImage和HierarchicalImage写入到磁盘中的图像数据进行动态的读写操作。
 *
 * 几个工程文件说明：
 * - ReadingBigImage 图像浏览器的初步实现。
 * - Test 对BlockwiseImage的图像处理接口和DiskBigImage的读写接口进行了测试，还有一部分是关于不同索引方法的测试。
 * - WriteBlockWiseImage 使用BlockwiseImage写入图像数据
 * - WriteHierarchicalImage 使用HierarchicalImage写入图像数据
 * \n
 * 
 * @section setup_sec 安装
 * @subsection libary_sec 相关库
 * <pre>
 * <b>1. boost</b>
 * 使用到了boost的下面相关子库：
 * data_time iostreams filesystem system thread
 * 需要编译好上面的静态库

 * <b>2. stxxl</b>
 * 是一个提供了基于stl的容器接口，同时将容器数据存放在硬盘中的库。
 * Website : http://stxxl.sourceforge.net/ 
 * 安装说明：
 * （1）将/SharedLibrary/stxxl/include 设置为头文件包含的地址
 * （2）将/SharedLibrary/stxxl/lib 设置为lib包含的地址 
 * （3）在工程文件中设置下面的command line （前提配置好了boost, 在链接 stxxl的库的时候， 会自动去链接boost的相关库）

 * <b>Debug :</b>
 * C/C++ -- Command Line :
 * -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_SECURE_SCL=0 /Od /MDd /ZI -D_RTLDLL -DBOOST_LIB_DIAGNOSTIC -DSTXXL_BOOST_TIMESTAMP -DSTXXL_BOOST_CONFIG -DSTXXL_BOOST_FILESYSTEM -DSTXXL_BOOST_THREADS -DSTXXL_BOOST_RANDOM /EHsc /EHs /wd4820 /wd4217 /wd4668 /wd4619 /wd4625 /wd4626 /wd4355 /wd4996 -D_SCL_SECURE_NO_DEPRECATE /F 16777216 /nologo /DSTXXL_LIBNAME=\"stxxl_debug\" 

 * Linker -- Command Line :
 * /STACK:16777216 /NOLOGO /DEBUG 

 * <b>Release :</b>
 * C/C++ -- Command Line :
 * -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_SECURE_SCL=0 /O2 /Ob2 /MD /DNDEBUG -D_RTLDLL -DBOOST_LIB_DIAGNOSTIC -DSTXXL_BOOST_TIMESTAMP -DSTXXL_BOOST_CONFIG -DSTXXL_BOOST_FILESYSTEM -DSTXXL_BOOST_THREADS -DSTXXL_BOOST_RANDOM /EHsc /EHs /wd4820 /wd4217 /wd4668 /wd4619 /wd4625 /wd4626 /wd4355 /wd4996 -D_SCL_SECURE_NO_DEPRECATE /F 16777216 /nologo /DSTXXL_LIBNAME=\"stxxl_release\" 

 * Linker -- Command Line :
 * /STACK:16777216 /NOLOGO /OPT:REF 

 * （4）stxxl的配置文件，在代码运行的目录下面新建文件config.stxxl
 * For example : #config.stxxl
 * disk=c:\stxxl,70000,wincall     
 * disk=d:\stxxl,70000,wincall
 * disk=e:\stxxl,70000,wincall
 * #表示在在c，d，e中分别建立70000M大小的文件stxxl用来保存数据用。
 * #对于Out_Of_Core模块而言，大小取决于图像的数据大小，比如一个102400*102400的图像而言，
 * 如果图像是RGB个格式，那么总大小为30G的图像。由于使用了zorder进行存储，存在数据冗余。
 * 使用zorder作为索引格式的话，最多是为原来大小的4倍，一般为2倍左右，所以分配60G到120G比较合适。

 * <b>3. OpenCV</b>
 * 如果定义了宏 SAVE_MINI_IMAGE
 * 在将图像写入到磁盘中的时候，同时也会产生一张由用户指定分辨率大小的jpg格式图像文件，该文件是图像的缩略图用来方便得到显示效果
 * 将使用到了OpenCV的下面模块
 * core highgui imgproc
 * </pre>
 * @note 默认是没有定义SAVE_MINI_IMAGE
 * \n
 * 
 * @section src_sec Out Of Core模块代码
 * 所有代码都是模板类或者是放在.h文件中的inline函数，直接include到代码中编译即可。\n
 * 代码放在了/Platform/OutOfCore/include/ 文件夹下
 *
 */

#endif