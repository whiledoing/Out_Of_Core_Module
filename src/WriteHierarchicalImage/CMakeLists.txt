add_executable(WriteHierarchicalImage WriteHierarchicalImage.cpp)

# link the stxxl library
add_stxxl_support(WriteHierarchicalImage)

# opencv part
add_opencv_support(WriteHierarchicalImage)
