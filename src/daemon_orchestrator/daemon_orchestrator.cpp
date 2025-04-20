#include "daemon_orchestrator.h"

namespace daemon_orchestrator {
    daemon_orch_obj::daemon_orch_obj(const std::string& log_file_path, std::vector<socket_config::unix_socket_config> unix_socket_configs, std::vector<socket_config::web_socket_config> web_socket_configs,  parser_strategy parsing_strategy) :
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
            
            for (auto const& unix_socket_path_info : unix_socket_configs) {
                unix_listening_sockets.emplace_back(
                    input_socket::input_socket_builder()
                        .set_enqueue_buffer_parser_callback( [this](std::string msg) { buffer_parser.enqueue_msg(std::move(msg)); } )
                        .set_direct_log_callback(logger_messages_callback)
                        .set_parser_active_callback( [this]() { return buffer_parser.thread_active(); } )
                        .set_socket_path(unix_socket_path_info.unix_socket_path)
                        .set_backlog(unix_socket_path_info.backlog)
                        .set_socket_type(input_socket::socket_type::UNIX)
                        .build()
                );
            }

            for (auto const& web_socket_path_info : web_socket_configs) {
                web_listening_sockets.emplace_back(
                    input_socket::input_socket_builder()
                        .set_enqueue_buffer_parser_callback( [this](std::string msg) { buffer_parser.enqueue_msg(std::move(msg)); } )
                        .set_direct_log_callback(logger_messages_callback)
                        .set_parser_active_callback( [this]() { return buffer_parser.thread_active(); } )
                        .set_host_path(web_socket_path_info.host)
                        .set_port(web_socket_path_info.port)
                        .set_backlog(web_socket_path_info.backlog)
                        .set_socket_type(input_socket::socket_type::WEB)
                        .build()
                );
            }
            
            
        }

    void daemon_orch_obj::start_threads() {
        log_writer.init_thread();
        buffer_parser.init_thread();
        
        for (auto const& unix_socket : unix_listening_sockets) {
            unix_socket->init_socket();
            unix_socket->init_thread();
        }

        for (auto const& ip_socket : web_listening_sockets) {
            ip_socket->init_socket();
            ip_socket->init_thread();
        }
        /*
        if (input_socket) {
            input_socket->init_socket();
            input_socket->init_thread();
        }
        */
        //buffer_parser.enqueue_msg("DEBUG|LogInfo|Logger Orchestrator|C++|Logger Threads Initialized|");
        //buffer_parser.enqueue_msg("DEBUG|LogInfo|Logger Orchestrator|C++|Logger Input Socket Initialized|");
    }

    void daemon_orch_obj::kill_threads() {
        //buffer_parser.enqueue_msg("DEBUG|LogInfo|Logger Orchestrator|C++|Killing Logger Threads|");
        //buffer_parser.enqueue_msg("DEBUG|LogInfo|Logger Orchestrator|C++|Closing Logger Socket|");
        for (auto const& unix_socket : unix_listening_sockets) {
            unix_socket->close_socket();
            unix_socket->stop_thread();
        }

        for (auto const& ip_socket : web_listening_sockets) {
            ip_socket->close_socket();
            ip_socket->stop_thread();
        }
        /*
        if (input_socket) {
            input_socket->close_socket();
            input_socket->stop_thread();
        }
        */

        buffer_parser.stop_thread();
        log_writer.stop_thread();
    }

    void daemon_orch_obj::log_direct(std::string msg) { 
        if (buffer_parser.thread_active() && log_writer.thread_active()) {
            buffer_parser.enqueue_msg(msg); 
        }
    }

    void create_log_file(const std::string& log_file_path) {
        std::ofstream log_file(log_file_path, std::ios::out | std::ios::trunc);
        if (log_file.is_open()) {
            throw std::runtime_error("Log file already open elsewhere. Please try again.");
        }
    }

    void daemon_orch_obj::wait_until_queues_empty() {
        buffer_parser.wait_until_queue_empty();
        log_writer.wait_until_queue_empty();
    }
    
}