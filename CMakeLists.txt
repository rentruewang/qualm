cmake_minimum_required(VERSION 3.0.0)
project(Duostra VERSION 1.0.0)

include(CMakePrintHelpers)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# set flags

set(CMAKE_CXX_FLAGS_DEBUG 
    "${CMAKE_CXX_FLAGS_DEBUG} \
    -Wall -Wextra -Weffc++ \
    -fstack-protector-all -fsanitize=undefined \
    -g -rdynamic"
)
if(CMAKE_BUILD_TYPE MATCHES "Debug")
cmake_print_variables(CMAKE_CXX_FLAGS_DEBUG)
endif()
set(CMAKE_CXX_FLAGS_RELEASE 
    "${CMAKE_CXX_FLAGS_RELEASE} \
    -Wall -Wextra -Weffc++"
)
if(CMAKE_BUILD_TYPE MATCHES "Release")
cmake_print_variables(CMAKE_CXX_FLAGS_RELEASE)
endif()

# set(CMAKE_CXX_CLANG_TIDY
#   clang-tidy;
#   -checks=bugprone-use-after-move;)

include(CTest)
enable_testing()


# source files
file (GLOB DUOSTRA_DEP
    "src/*.cxx"
    "src/schedulers/*.cxx"
    "src/placers/*.cxx"
    "include/*.hxx"
)
add_executable(Duostra ${DUOSTRA_DEP})
include_directories(${CMAKE_SOURCE_DIR}/include/)
include_directories(${CMAKE_SOURCE_DIR}/externals/json/include/)
include_directories(${CMAKE_SOURCE_DIR}/externals/cpptqdm/)


# pytorch
# file(DOWNLOAD
#     https://download.pytorch.org/libtorch/nightly/cpu/libtorch-shared-with-deps-latest.zip
#     ${CMAKE_BINARY_DIR}/libtorch.zip
#     SHOW_PROGRESS
# )
# file(ARCHIVE_EXTRACT INPUT libtorch.zip)

find_package(Torch REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")
target_link_libraries(Duostra "${TORCH_LIBRARIES}")
set_property(TARGET Duostra PROPERTY CXX_STANDARD 17)

# Open MP
find_package(OpenMP REQUIRED)
target_link_libraries(Duostra OpenMP::OpenMP_CXX)

# GSL
find_package(GSL REQUIRED)
target_link_libraries(Duostra GSL::gsl GSL::gslcblas)

set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED OFF)  
set(Boost_USE_STATIC_RUNTIME OFF) 
find_package(Boost REQUIRED) 
include_directories(${Boost_INCLUDE_DIRS}) 
target_link_libraries(Duostra ${Boost_LIBRARIES} pthread dl)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
