cmake_minimum_required(VERSION 2.6)
project(OutOfCore)

# need packages boost
set(Boost_USE_MULTITHREAD ON)
find_package(Boost)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

# need stxxl support
set(_stxxl_include_dirs "${PROJECT_SOURCE_DIR}/Library/stxxl/include")
set(_stxxl_library_dirs "${PROJECT_SOURCE_DIR}/Library/stxxl/lib")
set(_stxxl_libs 
    debug ${_stxxl_library_dirs}/Debug/libstxxl.lib
    optimized ${_stxxl_library_dirs}/Release/libstxxl.lib
    )
include_directories(${_stxxl_include_dirs})

macro(add_stxxl_support target_name)
    target_link_libraries(${target_name} ${_stxxl_libs})

    # set c++ compile debug para
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} 
    -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_SECURE_SCL=0 
    /Od /MDd /ZI -D_RTLDLL -DBOOST_LIB_DIAGNOSTIC -DSTXXL_BOOST_TIMESTAMP 
    -DSTXXL_BOOST_CONFIG -DSTXXL_BOOST_FILESYSTEM -DSTXXL_BOOST_THREADS 
    -DSTXXL_BOOST_RANDOM /EHsc /EHs /wd4820 /wd4217 /wd4668 /wd4619 
    /wd4625 /wd4626 /wd4355 /wd4996 -D_SCL_SECURE_NO_DEPRECATE 
    /F 16777216 /nologo")

    # set c++ compile release para
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} 
    -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_SECURE_SCL=0 
    /O2 /Ob2 /MD /DNDEBUG -D_RTLDLL -DBOOST_LIB_DIAGNOSTIC -DSTXXL_BOOST_TIMESTAMP 
    -DSTXXL_BOOST_CONFIG -DSTXXL_BOOST_FILESYSTEM -DSTXXL_BOOST_THREADS -DSTXXL_BOOST_RANDOM 
    /EHsc /EHs /wd4820 /wd4217 /wd4668 /wd4619 /wd4625 /wd4626 /wd4355 /wd4996 
    -D_SCL_SECURE_NO_DEPRECATE /F 16777216 /nologo")

    # set c++ linker release para
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG}
    /STACK:16777216 /NOLOGO /DEBUG")

    # set c++ linker debug para
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE}
    /STACK:16777216 /NOLOGO /OPT:REF")

endmacro(add_stxxl_support)

# need opencv
macro(add_opencv_support target_name)
    find_package(OpenCV REQUIRED)
    include_directories(${OpenCV_INCLUDE_DIR})
    target_link_libraries(${target_name} ${OpenCV_LIBS})
endmacro(add_opencv_support)

# add my OutOfCore part
include_directories(${PROJECT_SOURCE_DIR}/include)

# set the output path
set(EXECUTABLE_OUTPUT_PATH "${PROJECT_BINARY_DIR}/bin")

# add all the building project
add_subdirectory(src/ReadingBigImage)
add_subdirectory(src/Test)
add_subdirectory(src/TimeConsumeTest)
add_subdirectory(src/WriteBlockWiseImage)
add_subdirectory(src/WriteHierarchicalImage)
