cmake_minimum_required(VERSION 3.4)

project(example_videotransform)

file(GLOB HEADER_FILES *.h)
file(GLOB SOURCE_FILES *.cpp)

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost COMPONENTS program_options)

add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES})


message(${Boost_LIBRARIES})
target_link_libraries(${PROJECT_NAME}
  ${Boost_LIBRARIES} 
  Boost::program_options
  videotransform
)

