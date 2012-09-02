add_executable(WriteBlockWiseImage WriteBlockWiseImage.cpp)

# link the stxxl library
target_link_libraries(ReadingBiImage 
    debug "${_stxxl_library_dirs}/stxxl_debug.lib"
    optimized "${_stxxl_library_dirs/stxxl_release.lib"
    )
