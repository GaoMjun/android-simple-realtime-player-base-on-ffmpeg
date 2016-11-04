# Sets the minimum version of CMake required to build the native
# library. You should either keep the default value or only pass a
# value of 3.4.0 or lower.

cmake_minimum_required(VERSION 3.4.1)

set(include_DIR ${CMAKE_SOURCE_DIR}/src/main/cpp/include)
set(library_DIR ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI})

include_directories(${include_DIR})

add_library( rtplayer SHARED src/main/cpp/rtplayer.cpp )


add_library( avcodec-57 SHARED IMPORTED )
set_target_properties( avcodec-57 PROPERTIES IMPORTED_LOCATION ${library_DIR}/libavcodec-57.so )

add_library( avformat-57 SHARED IMPORTED )
set_target_properties( avformat-57 PROPERTIES IMPORTED_LOCATION ${library_DIR}/libavformat-57.so )

add_library( avfilter-6 SHARED IMPORTED )
set_target_properties( avfilter-6 PROPERTIES IMPORTED_LOCATION ${library_DIR}/libavfilter-6.so )

add_library( avutil-55 SHARED IMPORTED )
set_target_properties( avutil-55 PROPERTIES IMPORTED_LOCATION ${library_DIR}/libavutil-55.so )

add_library( ffmpeg_codec SHARED IMPORTED )
set_target_properties( ffmpeg_codec PROPERTIES IMPORTED_LOCATION ${library_DIR}/libffmpeg_codec.so )

add_library( swresample-2 SHARED IMPORTED )
set_target_properties( swresample-2 PROPERTIES IMPORTED_LOCATION ${library_DIR}/libswresample-2.so )

add_library( swscale-4 SHARED IMPORTED )
set_target_properties( swscale-4 PROPERTIES IMPORTED_LOCATION ${library_DIR}/libswscale-4.so )

target_link_libraries( rtplayer avcodec-57 avformat-57 avfilter-6 avutil-55 ffmpeg_codec swresample-2 swscale-4 log android )

