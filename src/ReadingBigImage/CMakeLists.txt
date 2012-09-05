find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})

# files that need to be moc
file(GLOB _moc_files "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

# all the ui files
file(GLOB _forms "${CMAKE_CURRENT_SOURCE_DIR}/ui/*.ui")

# moc the files
QT4_WRAP_CPP(_moc_files_sources "${_moc_files}")

# uic the ui files
QT4_WRAP_UI(_forms_headers "${_forms}")

# add the moced files shown in vs
source_group("ui header" ${_forms_headers})

# find all the files in the src directory
file(GLOB_RECURSE _all_files "${CMAKE_CURRENT_SOURCE_DIR}/*")

# input : all added into msvc project files
# output : put all the files into the same filter name as the directory name in
# msvc project
macro(source_group_by_dir source_files)
    if(MSVC)
        set(sgbd_cur_dir ${CMAKE_CURRENT_SOURCE_DIR})
        foreach(sgbd_file ${${source_files}})
            string(REGEX REPLACE "${sgbd_cur_dir}/(.*)" "\\1" sgbd_fpath ${sgbd_file})
            string(REGEX REPLACE "(.*)/.*" "\\1" sgbd_group_name ${sgbd_fpath})
            string(COMPARE EQUAL ${sgbd_fpath} ${sgbd_group_name} sgbd_nogroup)
            string(REPLACE "/" "\\" sgbd_group_name ${sgbd_group_name})
            if(sgbd_nogroup)
                set(sgbd_group_name "\\")
            endif(sgbd_nogroup)
            source_group(${sgbd_group_name} ${sgbd_file})
        endforeach(sgbd_file)
    endif(MSVC)
endmacro(source_group_by_dir)

# scan all the files in current directory
source_group_by_dir(_all_files)

# add all the files 
add_executable(ReadingBigImage
    ${_moc_files_sources}
    ${_forms_headers}
    ${_all_files}
    )

# add the ui files include directory
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# add qt support
target_link_libraries(ReadingBigImage ${QT_LIBRARIES})

# link the stxxl library
add_stxxl_support(ReadingBigImage)

# link the opencv part
add_opencv_support(ReadingBigImage)
