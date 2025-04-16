#pragma once

#include "buffer_parser.h"
#include "input_socket.h"
#include "log_writer.h"

namespace daemon_orchestrator {
    class daemon_orch_obj {
    public:
        daemon_orch_obj(const char* log_file_path, const char* socket_path, parser_strategy parsing_strategy);
        void start_threads();
        void kill_threads();
        void wait_until_queues_empty();

        void log_orchestrator_info(std::string msg);

    private:
        std::function<void(std::string)> logger_messages_callback = [this](std::string msg) {
            buffer_parser.enqueue_msg(msg);
        };

        log_writer::log_writer_obj log_writer;
        buffer_parser::buffer_parser_obj buffer_parser;
        input_socket::input_socket_obj input_socket;

    };
}