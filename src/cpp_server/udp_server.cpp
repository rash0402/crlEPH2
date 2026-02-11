#include "udp_server.hpp"
#include <iostream>
#include <cstring>

namespace eph::udp {

UDPServer::UDPServer(uint16_t send_port, uint16_t recv_port, const std::string& target_host)
    : send_socket_(-1),
      recv_socket_(-1),
      initialized_(false),
      last_error_("") {

    initialized_ = init_send_socket(send_port, target_host) &&
                   init_recv_socket(recv_port);

    if (!initialized_) {
        std::cerr << "UDP Server initialization failed: " << last_error_ << std::endl;
    }
}

UDPServer::~UDPServer() {
    if (send_socket_ >= 0) {
        close(send_socket_);
    }
    if (recv_socket_ >= 0) {
        close(recv_socket_);
    }
}

bool UDPServer::init_send_socket(uint16_t port, const std::string& target_host) {
    send_socket_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (send_socket_ < 0) {
        last_error_ = "Failed to create send socket";
        return false;
    }

    std::memset(&send_addr_, 0, sizeof(send_addr_));
    send_addr_.sin_family = AF_INET;
    send_addr_.sin_port = htons(port);

    if (inet_pton(AF_INET, target_host.c_str(), &send_addr_.sin_addr) <= 0) {
        last_error_ = "Invalid target host address";
        close(send_socket_);
        send_socket_ = -1;
        return false;
    }

    return true;
}

bool UDPServer::init_recv_socket(uint16_t port) {
    recv_socket_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (recv_socket_ < 0) {
        last_error_ = "Failed to create receive socket";
        return false;
    }

    // Set non-blocking
    if (!set_non_blocking(recv_socket_)) {
        close(recv_socket_);
        recv_socket_ = -1;
        return false;
    }

    // Allow address reuse
    int opt = 1;
    if (setsockopt(recv_socket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        last_error_ = "Failed to set socket options";
        close(recv_socket_);
        recv_socket_ = -1;
        return false;
    }

    std::memset(&recv_addr_, 0, sizeof(recv_addr_));
    recv_addr_.sin_family = AF_INET;
    recv_addr_.sin_addr.s_addr = INADDR_ANY;
    recv_addr_.sin_port = htons(port);

    if (bind(recv_socket_, (struct sockaddr*)&recv_addr_, sizeof(recv_addr_)) < 0) {
        last_error_ = "Failed to bind receive socket to port " + std::to_string(port);
        close(recv_socket_);
        recv_socket_ = -1;
        return false;
    }

    return true;
}

bool UDPServer::set_non_blocking(int socket_fd) {
    int flags = fcntl(socket_fd, F_GETFL, 0);
    if (flags < 0) {
        last_error_ = "fcntl(F_GETFL) failed";
        return false;
    }
    if (fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK) < 0) {
        last_error_ = "fcntl(F_SETFL) failed";
        return false;
    }
    return true;
}

bool UDPServer::send_state(const StatePacket& packet) {
    if (!initialized_) {
        return false;
    }

    auto buffer = serialize_state_packet(packet);

    ssize_t sent = sendto(send_socket_, buffer.data(), buffer.size(), 0,
                          (struct sockaddr*)&send_addr_, sizeof(send_addr_));

    if (sent < 0) {
        int saved_errno = errno;
        last_error_ = "Failed to send packet: " + std::string(strerror(saved_errno));
        return false;
    }

    return true;
}

std::optional<nlohmann::json> UDPServer::receive_command() {
    if (!initialized_) {
        return std::nullopt;
    }

    char buffer[4096];
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    ssize_t received = recvfrom(recv_socket_, buffer, sizeof(buffer) - 1, 0,
                                (struct sockaddr*)&client_addr, &addr_len);

    if (received < 0) {
        if (errno == EWOULDBLOCK || errno == EAGAIN) {
            // No data available (non-blocking)
            return std::nullopt;
        }
        int saved_errno = errno;
        last_error_ = "Failed to receive command: " + std::string(strerror(saved_errno));
        return std::nullopt;
    }

    buffer[received] = '\0';

    try {
        nlohmann::json command = nlohmann::json::parse(buffer);
        return command;
    } catch (const nlohmann::json::parse_error& e) {
        last_error_ = "JSON parse error: " + std::string(e.what());
        return std::nullopt;
    }
}

} // namespace eph::udp
