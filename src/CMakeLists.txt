cmake_minimum_required(VERSION 3.0)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/scripts")

find_package(FFmpeg)

file(GLOB HEADER_FILES *.h)
file(GLOB SOURCE_FILES *.cpp)

add_library(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES})

#target_compile_options(${PROJECT_NAME} PRIVATE -fPIC)
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${PROJECT_SOURCE_DIR}/include/videotransform.h")
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} PRIVATE
  FFmpeg::libavcodec
  FFmpeg::libavcodec
  FFmpeg::libavdevice
  FFmpeg::libavfilter
  FFmpeg::libavformat
  FFmpeg::libavutil
  FFmpeg::libpostproc
  FFmpeg::libswresample
  FFmpeg::libswscale
  FFmpeg::libx264
#  x265
#  vdpau
#  va-drm 
#  va-x11
#  va
#  X11
  pthread
  dl
#  z
)

