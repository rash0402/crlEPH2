find_package(GTest REQUIRED)
include(GoogleTest)

# test_haze_estimator（最重要）
add_executable(test_haze_estimator test_haze_estimator.cpp)
target_link_libraries(test_haze_estimator PRIVATE eph_agent GTest::gtest_main)
gtest_discover_tests(test_haze_estimator)

# test_eph_agent
add_executable(test_eph_agent test_eph_agent.cpp)
target_link_libraries(test_eph_agent PRIVATE eph_agent GTest::gtest_main)
gtest_discover_tests(test_eph_agent)

# test_action_selector (Phase 4)
add_executable(test_action_selector test_action_selector.cpp)
target_link_libraries(test_action_selector PRIVATE eph_agent GTest::gtest_main)
gtest_discover_tests(test_action_selector)

# test_eph_agent_phase4 (Phase 4)
add_executable(test_eph_agent_phase4 test_eph_agent_phase4.cpp)
target_link_libraries(test_eph_agent_phase4 PRIVATE eph_agent GTest::gtest_main)
gtest_discover_tests(test_eph_agent_phase4)

# test_v1_validation (Phase 5 - V1検証)
add_executable(test_v1_validation test_v1_validation.cpp)
target_link_libraries(test_v1_validation PRIVATE eph_agent GTest::gtest_main)
gtest_discover_tests(test_v1_validation)

# test_v3_validation (Phase 5 - V3検証)
add_executable(test_v3_validation test_v3_validation.cpp)
target_link_libraries(test_v3_validation PRIVATE eph_agent GTest::gtest_main)
gtest_discover_tests(test_v3_validation)
