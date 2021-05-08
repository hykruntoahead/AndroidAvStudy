
cmake_minimum_required(VERSION 3.10.2)

project("ffmpeg_study")


find_library(log-lib
        log )


message("PROJECT_SOURCE_DIR = ${PROJECT_SOURCE_DIR}")
set(distribution_DIR  ${PROJECT_SOURCE_DIR}/../jniLibs/armeabi)

#====
# libavcodec
add_library(avcodec
        SHARED
        IMPORTED)
set_target_properties(avcodec
        PROPERTIES IMPORTED_LOCATION
        ${distribution_DIR}/libavcodec.so)

# avfilter
add_library(avfilter
        SHARED
        IMPORTED)
set_target_properties(avfilter
        PROPERTIES IMPORTED_LOCATION
        ${distribution_DIR}/libavfilter.so)

# libavformat
add_library(avformat
        SHARED
        IMPORTED)
set_target_properties(avformat
        PROPERTIES IMPORTED_LOCATION
        ${distribution_DIR}/libavformat.so)

# libavutil
add_library(avutil
        SHARED
        IMPORTED )
set_target_properties(avutil
        PROPERTIES IMPORTED_LOCATION
        ${distribution_DIR}/libavutil.so)



# libswresample
add_library(swresample
        SHARED
        IMPORTED)
set_target_properties(swresample
        PROPERTIES IMPORTED_LOCATION
        ${distribution_DIR}/libswresample.so)

# libswscale
add_library(swscale
        SHARED
        IMPORTED)
set_target_properties(swscale
        PROPERTIES IMPORTED_LOCATION
        ${distribution_DIR}/libswscale.so)


#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")

#======


add_library(
             native-lib
             SHARED
             native-lib.cpp )

include_directories(../../../src/main/jniLibs/include)
target_link_libraries(native-lib avcodec avfilter avformat avutil swresample swscale
                       ${log-lib} )