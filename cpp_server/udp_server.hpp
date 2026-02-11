#ifndef EPH_UDP_SERVER_HPP
#define EPH_UDP_SERVER_HPP

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <optional>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "protocol.hpp"

namespace eph::udp {

/**
 * @brief UDP Server for EPH GUI communication
 *
 * Port 5555: Send state data (C++ → Python)
 * Port 5556: Receive control commands (Python → C++)
 */
class UDPServer {
public:
    /**
     * @param send_port Port for sending state data (default: 5555)
     * @param recv_port Port for receiving commands (default: 5556)
     * @param target_host Target host for sending (default: "127.0.0.1")
     */
    UDPServer(uint16_t send_port = 5555,
              uint16_t recv_port = 5556,
              const std::string& target_host = "127.0.0.1");

    ~UDPServer();

    // Disable copy
    UDPServer(const UDPServer&) = delete;
    UDPServer& operator=(const UDPServer&) = delete;

    /**
     * @brief Send state packet to GUI
     * @return true if sent successfully
     */
    bool send_state(const StatePacket& packet);

    /**
     * @brief Receive control command (non-blocking)
     * @return JSON command if available, std::nullopt otherwise
     */
    std::optional<nlohmann::json> receive_command();

    /**
     * @brief Check if sockets are initialized properly
     */
    bool is_initialized() const { return initialized_; }

    /**
     * @brief Get last error message
     */
    const std::string& get_last_error() const { return last_error_; }

private:
    int send_socket_;
    int recv_socket_;
    struct sockaddr_in send_addr_;
    struct sockaddr_in recv_addr_;
    bool initialized_;
    std::string last_error_;

    bool init_send_socket(uint16_t port, const std::string& target_host);
    bool init_recv_socket(uint16_t port);
    void set_non_blocking(int socket_fd);
};

} // namespace eph::udp

#endif // EPH_UDP_SERVER_HPP
