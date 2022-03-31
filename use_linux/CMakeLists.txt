cmake_minimum_required (VERSION 3.0)

project (main)

#GDB 调试
SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

aux_source_directory (src SRC_LIST)
aux_source_directory (tcp SRC_LIST)

include_directories (include)
include_directories (tcp)

#添加库 -引入外部依赖包
find_package(Threads)
# target_link_libraries (${PROJECT_NAME} pthread) 

add_executable (main ${SRC_LIST})

target_link_libraries (main ${CMAKE_THREAD_LIBS_INIT}) 