#include "daemon_orchestrator.h"

namespace daemon_orchestrator {
    daemon_orch_obj::daemon_orch_obj(const std::string& log_file_path, const std::string& socket_path, parser_strategy parsing_strategy) :
        log_writer(
            log_file_path,
            logger_messages_callback
        ),
        buffer_parser(
            [this](std::string msg) { log_writer.enqueue_msg(std::move(msg)); },
            logger_messages_callback,
            [this]() { return log_writer.thread_active(); },
            parsing_strategy
        )
        {
            if (!socket_path.empty()) {
                input_socket = std::make_unique<input_socket::input_socket_obj>(
                    [this](std::string msg) { buffer_parser.enqueue_msg(std::move(msg)); }, 
                    logger_messages_callback, 
                    [this]() { return buffer_parser.thread_active(); },
                    socket_path
                );
            }
        }

    void daemon_orch_obj::start_threads() {
        log_writer.init_thread();
        buffer_parser.init_thread();
        
        if (input_socket) {
            input_socket->init_socket();
            input_socket->init_thread();
        }
        //buffer_parser.enqueue_msg("DEBUG|LogInfo|Logger Orchestrator|C++|Logger Threads Initialized|");
        //buffer_parser.enqueue_msg("DEBUG|LogInfo|Logger Orchestrator|C++|Logger Input Socket Initialized|");
    }

    void daemon_orch_obj::kill_threads() {
        //buffer_parser.enqueue_msg("DEBUG|LogInfo|Logger Orchestrator|C++|Killing Logger Threads|");
        //buffer_parser.enqueue_msg("DEBUG|LogInfo|Logger Orchestrator|C++|Closing Logger Socket|");
        if (input_socket) {
            input_socket->close_socket();
            input_socket->stop_thread();
        }

        buffer_parser.stop_thread();
        log_writer.stop_thread();
    }

    void daemon_orch_obj::log_direct(std::string msg) { 
        if (buffer_parser.thread_active() && log_writer.thread_active()) {
            buffer_parser.enqueue_msg(msg); 
        }
    }

    void daemon_orch_obj::wait_until_queues_empty() {
        buffer_parser.wait_until_queue_empty();
        log_writer.wait_until_queue_empty();
    }
}