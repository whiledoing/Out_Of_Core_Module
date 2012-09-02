add_executable(WriteHierarchicalImage WriteHierarchicalImage.cpp)

# link the stxxl library
target_link_libraries(WriteHierarchicalImage 
    debug "${_stxxl_library_dirs}/stxxl_debug.lib"
    optimized "${_stxxl_library_dirs/stxxl_release.lib"
    )

