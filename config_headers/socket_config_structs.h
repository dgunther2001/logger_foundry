#pragma once

namespace socket_config {
    struct unix_socket_config {
        std::string unix_socket_path;
        uint32_t backlog;
    };
}
