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
# Note: BetaSweep_DetectsCriticalPoint は計算量が多いため、タイムアウトを1200秒に延長
add_executable(test_v2_complete test_v2_complete.cpp)
target_link_libraries(test_v2_complete PRIVATE eph_phase GTest::gtest_main)
gtest_discover_tests(test_v2_complete
    PROPERTIES TIMEOUT 1200
)

# test_v4_validation（V4検証 - 長時間数値安定性 + Phase解析統合）
add_executable(test_v4_validation_phase test_v4_validation.cpp)
target_link_libraries(test_v4_validation_phase PRIVATE eph_phase eph_swarm eph_agent eph_spm eph_core GTest::gtest_main)
gtest_discover_tests(test_v4_validation_phase)

# test_v5_validation（V5検証 - 大規模群スケーラビリティ）
add_executable(test_v5_validation test_v5_validation.cpp)
target_link_libraries(test_v5_validation PRIVATE eph_phase eph_swarm eph_agent eph_spm eph_core GTest::gtest_main)
gtest_discover_tests(test_v5_validation
    PROPERTIES TIMEOUT 300
)
