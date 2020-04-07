include_directories("${PROJECT_SOURCE_DIR}")

set(PROJECT_TEST_NAME ${PROJECT_NAME}-test)

#set(FFmpeg_ROOT "$ENV{FFMPEG_ROOT}")
#link_directories(${FFmpeg_ROOT}/lib)
#include_directories(${PROJECT_SOURCE_DIR}/include)

add_executable(${PROJECT_TEST_NAME} main.cpp pass_and_fail_test.cpp)


target_link_libraries( ${PROJECT_TEST_NAME}
      videotransform

#      avcodec
#      avdevice
#      avfilter
#      avformat
#      avutil
#      postproc
#      swresample
#      swscale

#      x264
#      x265
#      vdpau
#      va-drm 
#      va-x11
#      va
#      X11
#      pthread
#      z

      gtest 
      gmock_main 
)

add_test(NAME ${PROJECT_TEST_NAME}-runner COMMAND ${PROJECT_TEST_NAME})
