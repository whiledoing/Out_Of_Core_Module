add_executable(WriteBlockWiseImage WriteBlockWiseImage.cpp)

# link the stxxl library
add_stxxl_support(WriteBlockWiseImage)

# opencv part
add_opencv_support(WriteBlockWiseImage)
