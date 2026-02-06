#include <gtest/gtest.h>
#include "eph_spm/saliency_polar_map.hpp"

using namespace eph;
using namespace eph::spm;

// === 基本機能テスト ===

TEST(SaliencyPolarMap, Constructor_InitializesToZero) {
    SaliencyPolarMap spm;

    for (int ch = 0; ch < 10; ++ch) {
        auto channel = spm.get_channel(static_cast<ChannelID>(ch));
        for (int a = 0; a < 12; ++a) {
            for (int b = 0; b < 12; ++b) {
                EXPECT_DOUBLE_EQ(channel(a, b), 0.0)
                    << "Non-zero initial value at (ch=" << ch
                    << ", θ=" << a << ", r=" << b << ")";
            }
        }
    }
}

TEST(SaliencyPolarMap, SetGetChannel_RoundTrip) {
    SaliencyPolarMap spm;
    Matrix12x12 test_data = Matrix12x12::Random();

    spm.set_channel(ChannelID::F0, test_data);
    auto retrieved = spm.get_channel(ChannelID::F0);

    EXPECT_TRUE(retrieved.isApprox(test_data, 1e-10));
}

TEST(SaliencyPolarMap, SetGetChannel_IndependentChannels) {
    SaliencyPolarMap spm;

    Matrix12x12 data_f0 = Matrix12x12::Constant(1.0);
    Matrix12x12 data_f1 = Matrix12x12::Constant(2.0);

    spm.set_channel(ChannelID::F0, data_f0);
    spm.set_channel(ChannelID::F1, data_f1);

    auto f0 = spm.get_channel(ChannelID::F0);
    auto f1 = spm.get_channel(ChannelID::F1);

    EXPECT_TRUE(f0.isApprox(data_f0, 1e-10));
    EXPECT_TRUE(f1.isApprox(data_f1, 1e-10));
}

TEST(SaliencyPolarMap, ZeroAll_ClearsAllChannels) {
    SaliencyPolarMap spm;

    // 全チャネルにランダムデータを設定
    for (int ch = 0; ch < 10; ++ch) {
        Matrix12x12 data = Matrix12x12::Random();
        spm.set_channel(static_cast<ChannelID>(ch), data);
    }

    // ゼロクリア
    spm.zero_all();

    // 全チャネルがゼロになったことを確認
    for (int ch = 0; ch < 10; ++ch) {
        auto channel = spm.get_channel(static_cast<ChannelID>(ch));
        for (int a = 0; a < 12; ++a) {
            for (int b = 0; b < 12; ++b) {
                EXPECT_DOUBLE_EQ(channel(a, b), 0.0)
                    << "Non-zero value after zero_all at (ch=" << ch
                    << ", θ=" << a << ", r=" << b << ")";
            }
        }
    }
}

// === 次元テスト ===

TEST(SaliencyPolarMap, Dimensions_Correct) {
    SaliencyPolarMap spm;

    EXPECT_EQ(spm.channel_count(), 10);
    EXPECT_EQ(spm.theta_count(), 12);
    EXPECT_EQ(spm.r_count(), 12);
}

// === チャネルIDテスト ===

TEST(SaliencyPolarMap, AllChannelIDs_Accessible) {
    SaliencyPolarMap spm;

    std::vector<ChannelID> channel_ids = {
        ChannelID::T0, ChannelID::R0, ChannelID::R1,
        ChannelID::F0, ChannelID::F1, ChannelID::F2,
        ChannelID::F3, ChannelID::F4, ChannelID::F5,
        ChannelID::M0
    };

    for (auto id : channel_ids) {
        Matrix12x12 test_data = Matrix12x12::Random();
        spm.set_channel(id, test_data);
        auto retrieved = spm.get_channel(id);
        EXPECT_TRUE(retrieved.isApprox(test_data, 1e-10))
            << "Channel access failed for ChannelID " << static_cast<int>(id);
    }
}
