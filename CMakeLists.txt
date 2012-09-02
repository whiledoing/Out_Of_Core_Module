project(OutOfCore)

# need packages boost
set(Boost_USE_MULTITHREAD ON)
find_package(Boost)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

# need stxxl support
set(_stxxl_include_dirs "${PROJECT_SOURCE_DIR}/Library/stxxl/include")
set(_stxxl_library_dirs "${PROJECT_SOURCE_DIR}/Library/stxxl/lib")
include_directories(${_stxxl_include_dirs})

# add my OutOfCore part
include_directories(${PROJECT_SOURCE_DIR}/src/OutOfCore)

# add all the building project
add_subdirectory(src/ReadingBiImage)
add_subdirectory(src/Test)
add_subdirectory(src/TimeConsumeTest)
add_subdirectory(src/WriteBlockWiseImage)
add_subdirectory(src/WriteHierarchicalImage)
