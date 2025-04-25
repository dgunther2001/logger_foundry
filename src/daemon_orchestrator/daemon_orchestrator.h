#pragma once

#include "buffer_parser.h"
#include "input_socket.h"
#include "log_writer.h"

#include "../../config_headers/socket_config_structs.h"

namespace daemon_orchestrator {

    class daemon_orch_obj {
    public:
        daemon_orch_obj(const std::string& log_file_path, bool enable_end_of_test_diagnostics, std::vector<socket_config::unix_socket_config> unix_socket_config, std::vector<socket_config::web_socket_config> web_socket_configs, parser_strategy parsing_strategy=nullptr);
        void start_threads();
        void kill_threads();
        void wait_until_queues_empty();

        void log_direct(std::string msg);

    private:
        void create_log_file(const std::string& log_file_path);


        bool enable_end_of_test_diagnostics;
        std::function<void(std::string)> log_diagnostic_callback = [this](std::string msg) {
            if (enable_end_of_test_diagnostics) {
                log_writer.enqueue_msg(msg);
            }
        };

        log_writer::log_writer_obj log_writer;
        buffer_parser::buffer_parser_obj buffer_parser;
        std::vector<std::unique_ptr<input_socket::input_socket_obj>> unix_listening_sockets;
        std::vector<std::unique_ptr<input_socket::input_socket_obj>> web_listening_sockets;

    };
}