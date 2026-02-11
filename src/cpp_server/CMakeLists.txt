cmake_minimum_required(VERSION 3.20)
project(eph_udp_server VERSION 2.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Eigen (required for EPH types)
find_package(Eigen3 REQUIRED NO_MODULE)

# Find nlohmann/json (required for command parsing)
find_package(nlohmann_json QUIET)
if(NOT nlohmann_json_FOUND)
    # Fallback to include path if package not found
    find_path(NLOHMANN_JSON_INCLUDE_DIR
        NAMES nlohmann/json.hpp
        PATHS /opt/homebrew/include /usr/local/include
    )
    if(NLOHMANN_JSON_INCLUDE_DIR)
        add_library(nlohmann_json INTERFACE)
        target_include_directories(nlohmann_json INTERFACE ${NLOHMANN_JSON_INCLUDE_DIR})
    else()
        message(FATAL_ERROR "nlohmann/json not found. Please install: brew install nlohmann-json")
    endif()
endif()

# UDP Server library
add_library(eph_udp_server
    udp_server.cpp
)

target_include_directories(eph_udp_server PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/packages
)

target_link_libraries(eph_udp_server PUBLIC
    eph_core
    Eigen3::Eigen
    nlohmann_json
)

# Main executable (simulation + UDP server)
add_executable(eph_gui_server
    main.cpp
)

target_link_libraries(eph_gui_server PRIVATE
    eph_udp_server
    eph_swarm
    eph_phase
    eph_agent
    eph_spm
    eph_core
)

# Installation
install(TARGETS eph_udp_server eph_gui_server
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
