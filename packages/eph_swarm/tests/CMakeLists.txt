find_package(GTest REQUIRED)
include(GoogleTest)

# test_swarm_manager
add_executable(test_swarm_manager test_swarm_manager.cpp)
target_link_libraries(test_swarm_manager PRIVATE eph_swarm GTest::gtest_main)
gtest_discover_tests(test_swarm_manager)

# test_mb_breaking（最重要）
add_executable(test_mb_breaking test_mb_breaking.cpp)
target_link_libraries(test_mb_breaking PRIVATE eph_swarm GTest::gtest_main)
gtest_discover_tests(test_mb_breaking)

# test_swarm_dynamics (Phase 4)
add_executable(test_swarm_dynamics test_swarm_dynamics.cpp)
target_link_libraries(test_swarm_dynamics PRIVATE eph_swarm GTest::gtest_main)
gtest_discover_tests(test_swarm_dynamics)

# test_v4_validation (Phase 5 - V4検証)
add_executable(test_v4_validation test_v4_validation.cpp)
target_link_libraries(test_v4_validation PRIVATE eph_swarm GTest::gtest_main)
gtest_discover_tests(test_v4_validation)
