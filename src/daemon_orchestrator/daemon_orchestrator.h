#pragma once

#include "buffer_parser.h"
#include "input_socket.h"
#include "log_writer.h"

#include "../../config_headers/socket_config_structs.h"

namespace daemon_orchestrator {

    class daemon_orch_obj {
    public:
        daemon_orch_obj(const std::string& log_file_path, std::vector<socket_config::unix_socket_config> unix_socket_config, std::vector<socket_config::web_socket_config> web_socket_configs, parser_strategy parsing_strategy=nullptr);
        void start_threads();
        void kill_threads();
        void wait_until_queues_empty();

        void log_direct(std::string msg);

    private:
        void create_log_file(const std::string& log_file_path);

        std::function<void(std::string)> logger_messages_callback = [this](std::string msg) {
            buffer_parser.enqueue_msg(msg);
        };

        log_writer::log_writer_obj log_writer;
        buffer_parser::buffer_parser_obj buffer_parser;
        std::vector<std::unique_ptr<input_socket::input_socket_obj>> unix_listening_sockets;
        std::vector<std::unique_ptr<input_socket::input_socket_obj>> web_listening_sockets;

    };
}