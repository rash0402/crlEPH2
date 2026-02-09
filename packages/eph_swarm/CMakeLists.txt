cmake_minimum_required(VERSION 3.20)
project(eph_swarm VERSION 2.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Eigenの検索
find_package(Eigen3 REQUIRED NO_MODULE)

# nanoflannの取得（k-d tree近傍探索ライブラリ）
include(FetchContent)
FetchContent_Declare(
    nanoflann
    GIT_REPOSITORY https://github.com/jlblancoc/nanoflann.git
    GIT_TAG        v1.5.5
)
FetchContent_MakeAvailable(nanoflann)

# インターフェースライブラリ（ヘッダーオンリー）
add_library(eph_swarm INTERFACE)

target_include_directories(eph_swarm INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(eph_swarm INTERFACE
    eph_core
    eph_spm
    eph_agent
    Eigen3::Eigen
    nanoflann::nanoflann
)

# テスト
if(BUILD_TESTING)
    find_package(GTest REQUIRED)
    add_subdirectory(tests)
endif()
