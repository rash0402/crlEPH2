#include <iostream>
#include <thread>
#include <chrono>
#include <numeric>
#include "udp_server.hpp"
#include "eph_swarm/swarm_manager.hpp"
#include "eph_phase/phase_analyzer.hpp"
#include "eph_spm/saliency_polar_map.hpp"

using namespace eph;

int main(int argc, char** argv) {
    std::cout << "EPH v2.1 GUI Server" << std::endl;
    std::cout << "Initializing UDP server..." << std::endl;

    udp::UDPServer server(5555, 5556);

    if (!server.is_initialized()) {
        std::cerr << "Failed to initialize UDP server: "
                  << server.get_last_error() << std::endl;
        return 1;
    }

    std::cout << "UDP server initialized successfully" << std::endl;
    std::cout << "  Send port: 5555 (state data)" << std::endl;
    std::cout << "  Recv port: 5556 (commands)" << std::endl;

    // Initialize simulation
    const size_t N_AGENTS = 10;  // Small for Phase 1 test
    const Scalar BETA = 0.098;
    const int AVG_NEIGHBORS = 6;

    swarm::SwarmManager swarm(N_AGENTS, BETA, AVG_NEIGHBORS);
    phase::PhaseAnalyzer analyzer;

    // Create simple SPM (all zeros for Phase 1)
    spm::SaliencyPolarMap test_spm;

    std::cout << "Simulation initialized (N=" << N_AGENTS << ")" << std::endl;
    std::cout << "Starting simulation loop..." << std::endl;

    uint32_t sequence_num = 0;
    uint32_t timestep = 0;
    const Scalar dt = 0.1;
    const int SEND_INTERVAL = 10;  // Send every 10 steps

    // Playback control state (start paused, user must press Play)
    bool is_playing = false;
    double speed_multiplier = 1.0;
    int sleep_ms = static_cast<int>(dt * 1000);

    // Agent selection state
    std::optional<size_t> selected_agent_id;

    while (true) {
        // Check for commands
        auto command = server.receive_command();
        if (command.has_value()) {
            std::cout << "Received command: " << command.value().dump() << std::endl;

            std::string cmd_type = command.value().value("type", "");

            if (cmd_type == "play") {
                is_playing = true;
                std::cout << "  Simulation resumed" << std::endl;
            }
            else if (cmd_type == "pause") {
                is_playing = false;
                std::cout << "  Simulation paused" << std::endl;
            }
            else if (cmd_type == "stop") {
                is_playing = false;
                timestep = 0;
                sequence_num = 0;
                std::cout << "  Simulation stopped (reset to t=0)" << std::endl;
                // Note: swarm state not reset (would need SwarmManager.reset())
            }
            else if (cmd_type == "set_speed") {
                speed_multiplier = command.value().value("speed", 1.0);
                sleep_ms = static_cast<int>(dt * 1000 / speed_multiplier);
                std::cout << "  Speed set to " << speed_multiplier << "x (sleep=" << sleep_ms << "ms)" << std::endl;
            }
            else if (cmd_type == "set_parameters") {
                auto params = command.value()["parameters"];
                std::cout << "  Parameters: " << params.dump() << std::endl;
                // TODO: Apply parameters (Phase 2 deferred)
            }
            else if (cmd_type == "select_agent") {
                int agent_id = command.value().value("agent_id", -1);
                if (agent_id >= 0 && agent_id < static_cast<int>(N_AGENTS)) {
                    selected_agent_id = static_cast<size_t>(agent_id);
                    std::cout << "  Selected agent: " << agent_id << std::endl;
                } else {
                    selected_agent_id = std::nullopt;
                    std::cout << "  Deselected agent (invalid ID)" << std::endl;
                }
            }
        }

        // Update simulation (only if playing)
        if (is_playing) {
            swarm.update_all_agents(test_spm, dt);
            timestep++;
        }

        // Send state at intervals
        if (timestep % SEND_INTERVAL == 0) {
            udp::StatePacket packet;
            packet.header.sequence_num = sequence_num++;
            packet.header.timestep = timestep;
            packet.header.num_agents = N_AGENTS;

            // Collect agent data
            for (size_t i = 0; i < N_AGENTS; ++i) {
                const auto& agent = swarm.get_agent(i);
                const auto& agent_state = agent.state();

                udp::AgentData agent_data;
                agent_data.agent_id = static_cast<uint16_t>(i);
                agent_data.x = static_cast<float>(agent_state.position.x());
                agent_data.y = static_cast<float>(agent_state.position.y());
                agent_data.vx = static_cast<float>(agent_state.velocity.x());
                agent_data.vy = static_cast<float>(agent_state.velocity.y());
                agent_data.haze_mean = static_cast<float>(agent.haze().mean());
                agent_data.fatigue = static_cast<float>(agent_state.fatigue);
                agent_data.efe = 0.0f;  // TODO: Add EFE tracking

                packet.agents.push_back(agent_data);
            }

            // Collect metrics
            auto haze_fields = swarm.get_all_haze_fields();
            Scalar phi = analyzer.compute_phi(haze_fields);

            // Calculate average haze
            Scalar avg_haze = 0.0;
            for (const auto& haze : haze_fields) {
                avg_haze += haze.mean();
            }
            avg_haze /= static_cast<Scalar>(haze_fields.size());

            // Calculate average speed
            Scalar avg_speed = 0.0;
            Scalar avg_fatigue = 0.0;
            for (size_t i = 0; i < N_AGENTS; ++i) {
                const auto& agent = swarm.get_agent(i);
                const auto& state = agent.state();
                avg_speed += state.velocity.norm();
                avg_fatigue += state.fatigue;
            }
            avg_speed /= static_cast<Scalar>(N_AGENTS);
            avg_fatigue /= static_cast<Scalar>(N_AGENTS);

            packet.metrics.phi = phi;
            packet.metrics.chi = 0.0;  // Chi needs time series, set to 0 for Phase 1
            packet.metrics.beta_current = BETA;
            packet.metrics.avg_haze = avg_haze;
            packet.metrics.avg_speed = avg_speed;
            packet.metrics.avg_fatigue = avg_fatigue;

            // Send state packet (checksum and data_length calculated in serialize)
            if (!server.send_state(packet)) {
                std::cerr << "Failed to send packet: " << server.get_last_error() << std::endl;
            }
        }

        // Send selected agent detail (if any)
        if (selected_agent_id.has_value() && timestep % SEND_INTERVAL == 0) {
            const auto& agent = swarm.get_agent(selected_agent_id.value());
            const auto& agent_state = agent.state();
            const auto& spm_data = agent.haze();  // 12x12 Matrix12x12

            udp::AgentDetailData detail;
            detail.agent_id = static_cast<uint16_t>(selected_agent_id.value());

            // Flatten SPM data (row-major: r=0..11, theta=0..11)
            for (int r = 0; r < 12; ++r) {
                for (int theta = 0; theta < 12; ++theta) {
                    detail.spm_data[r * 12 + theta] = static_cast<float>(spm_data(r, theta));
                }
            }

            // Velocity angle (atan2 returns [-π, π])
            detail.velocity_angle = static_cast<float>(
                std::atan2(agent_state.velocity.y(), agent_state.velocity.x())
            );

            // Get neighbors
            auto neighbors = swarm.find_neighbors(selected_agent_id.value());
            detail.num_neighbors = static_cast<uint16_t>(std::min(neighbors.size(), size_t(6)));
            for (size_t i = 0; i < detail.num_neighbors; ++i) {
                detail.neighbor_ids[i] = static_cast<uint16_t>(neighbors[i]);
            }
            for (size_t i = detail.num_neighbors; i < 6; ++i) {
                detail.neighbor_ids[i] = 0xFFFF;  // Invalid marker
            }

            // Send via UDP (reuse send socket, different format)
            // For simplicity, append to state packet (alternative: separate port)
            // TODO: Implement proper serialization
        }

        // Sleep with speed adjustment
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
    }

    return 0;
}
