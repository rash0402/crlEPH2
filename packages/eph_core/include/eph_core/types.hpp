#ifndef EPH_CORE_TYPES_HPP
#define EPH_CORE_TYPES_HPP

#include <Eigen/Core>
#include <unsupported/Eigen/CXX11/Tensor>

namespace eph {

// スカラー型（全プロジェクト統一）
using Scalar = double;

// ベクトル型
using Vec2 = Eigen::Vector2d;

// テンソル型
using Tensor3 = Eigen::Tensor<Scalar, 3>;
using Matrix12x12 = Eigen::Matrix<Scalar, 12, 12>;

// エージェント状態（POD設計）
struct AgentState {
    Vec2 position;      // [m]
    Vec2 velocity;      // [m/s]
    Scalar kappa;       // haze sensitivity [0.3-1.5]
    Scalar fatigue;     // fatigue level [0-1]

    // デフォルトコンストラクタ
    AgentState()
        : position(Vec2::Zero()),
          velocity(Vec2::Zero()),
          kappa(1.0),
          fatigue(0.0) {}

    // パラメータ付きコンストラクタ
    AgentState(const Vec2& pos, const Vec2& vel, Scalar k, Scalar f)
        : position(pos), velocity(vel), kappa(k), fatigue(f) {}
};

// SPMチャネルID
enum class ChannelID : int {
    T0 = 0,  // Obs Occupancy (Teacher)
    R0 = 1,  // Δoccupancy (+1)
    R1 = 2,  // Uncertainty
    F0 = 3,  // Occupancy (Current)
    F1 = 4,  // Motion Pressure
    F2 = 5,  // Saliency
    F3 = 6,  // TTC Proxy
    F4 = 7,  // Visibility
    F5 = 8,  // Observation Stability
    M0 = 9   // Haze Field
};

}  // namespace eph

#endif  // EPH_CORE_TYPES_HPP
