find_package(GTest REQUIRED)
include(GoogleTest)

# test_types
add_executable(test_types test_types.cpp)
target_link_libraries(test_types PRIVATE eph_core GTest::gtest_main)
gtest_discover_tests(test_types)

# test_math_utils
add_executable(test_math_utils test_math_utils.cpp)
target_link_libraries(test_math_utils PRIVATE eph_core GTest::gtest_main)
gtest_discover_tests(test_math_utils)

# test_config
add_executable(test_config test_config.cpp)
target_link_libraries(test_config PRIVATE eph_core GTest::gtest_main)
gtest_discover_tests(test_config)
