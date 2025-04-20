#pragma once

namespace socket_config {
    struct unix_socket_config {
        std::string unix_socket_path;
        uint16_t backlog;
    };

    struct web_socket_config {
        uint16_t port;
        uint16_t backlog;
        std::string host = "";
    };
}
