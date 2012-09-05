# find the cpp files
file(GLOB _src_list *.cpp)
add_executable(Test ${_src_list})

# add stxxl support
add_stxxl_support(Test)

# add opencv support
add_opencv_support(Test)
