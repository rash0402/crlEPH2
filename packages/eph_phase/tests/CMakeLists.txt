find_package(GTest REQUIRED)
include(GoogleTest)

# test_phase_analyzer
add_executable(test_phase_analyzer test_phase_analyzer.cpp)
target_link_libraries(test_phase_analyzer PRIVATE eph_phase GTest::gtest_main)
gtest_discover_tests(test_phase_analyzer)

# test_beta_sweep（最重要 - V2検証実験 Phase 3版）
add_executable(test_beta_sweep test_beta_sweep.cpp)
target_link_libraries(test_beta_sweep PRIVATE eph_phase GTest::gtest_main)
gtest_discover_tests(test_beta_sweep)

# test_v2_complete（Phase 4完全版 - V2検証目標達成）
add_executable(test_v2_complete test_v2_complete.cpp)
target_link_libraries(test_v2_complete PRIVATE eph_phase GTest::gtest_main)
gtest_discover_tests(test_v2_complete)
