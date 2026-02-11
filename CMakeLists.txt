cmake_minimum_required(VERSION 3.20)
project(EPH VERSION 2.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Eigenの検索（バージョン不問、3.4+互換）
find_package(Eigen3 REQUIRED NO_MODULE)

# テスト有効化
enable_testing()

# パッケージ追加（Phase 1）
add_subdirectory(packages/eph_core)
add_subdirectory(packages/eph_spm)

# パッケージ追加（Phase 2）
add_subdirectory(packages/eph_agent)

# パッケージ追加（Phase 3）
add_subdirectory(packages/eph_swarm)
add_subdirectory(packages/eph_phase)

# UDP GUI Server (optional build)
option(BUILD_GUI_SERVER "Build UDP GUI Server" ON)
if(BUILD_GUI_SERVER)
    add_subdirectory(src/cpp_server)
endif()
