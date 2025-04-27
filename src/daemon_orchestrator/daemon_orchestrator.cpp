#include "daemon_orchestrator.h"

namespace daemon_orchestrator {
    daemon_orch_obj::daemon_orch_obj(const std::string& log_file_path, bool enable_end_of_test_diagnostics, uint64_t health_diagnostic_interval, std::vector<socket_config::unix_socket_config> unix_socket_configs, std::vector<socket_config::web_socket_config> web_socket_configs,  parser_strategy parsing_strategy) :
        log_writer(
            log_file_path,
            log_diagnostic_callback
        ),
        buffer_parser(
            [this](std::string msg) { log_writer.enqueue_msg(std::move(msg)); },
            log_diagnostic_callback,
            [this]() { return log_writer.thread_active(); },
            parsing_strategy
        ),
        enable_end_of_test_diagnostics(enable_end_of_test_diagnostics),
        health_diagnostic_interval(health_diagnostic_interval)
        {
            for (auto const& unix_socket_path_info : unix_socket_configs) {
                unix_listening_sockets.emplace_back(
                    input_socket::input_socket_builder()
                        .set_enqueue_to_consumer_callback( [this, bypass = unix_socket_path_info.bypass_parser](std::string msg) { 
                            if (!bypass) { buffer_parser.enqueue_msg(std::move(msg)); }
                            else { log_writer.enqueue_msg(std::move(msg)); }
                        })
                        .set_log_diagnostic_callback(log_diagnostic_callback)
                        .set_parser_active_callback( [this]() { return buffer_parser.thread_active(); } )
                        .set_socket_path(unix_socket_path_info.unix_socket_path)
                        .set_backlog(unix_socket_path_info.backlog)
                        .set_socket_type(input_socket::util::socket_type::UNIX)
                        .build()
                );
            }

            for (auto const& web_socket_path_info : web_socket_configs) {
                web_listening_sockets.emplace_back(
                    input_socket::input_socket_builder()
                        .set_enqueue_to_consumer_callback( [this, bypass = web_socket_path_info.bypass_parser](std::string msg) { 
                            if (!bypass) { buffer_parser.enqueue_msg(std::move(msg)); }
                            else { log_writer.enqueue_msg(std::move(msg)); }
                        })
                        .set_log_diagnostic_callback(log_diagnostic_callback)
                        .set_parser_active_callback( [this]() { return buffer_parser.thread_active(); } )
                        .set_host_path(web_socket_path_info.host)
                        .set_port(web_socket_path_info.port)
                        .set_backlog(web_socket_path_info.backlog)
                        .set_socket_type(input_socket::util::socket_type::WEB)
                        .build()
                );
            }         
        }

    void daemon_orch_obj::start_threads() {
        log_writer.init_thread();
        buffer_parser.init_thread();
        
        for (auto const& unix_socket : unix_listening_sockets) {
            unix_socket->init_socket();
            if (health_diagnostic_interval != 0) register_thread_diagnostic(unix_socket);
            unix_socket->init_thread();
        }

        for (auto const& ip_socket : web_listening_sockets) {
            ip_socket->init_socket();
            if (health_diagnostic_interval != 0) register_thread_diagnostic(ip_socket);
            ip_socket->init_thread();
        }
        
        if (health_diagnostic_interval != 0) {


            is_health_thread_running.store(true);
            health_monitoring_thread = std::thread([this] {
                std::unique_lock<std::mutex> lock(health_thread_mutex);
                while(is_health_thread_running.load()) {
                    health_thread_condition_variable.wait_for(lock, std::chrono::seconds(health_diagnostic_interval), [this] {
                        return !is_health_thread_running.load();
                    });

                    if (!is_health_thread_running.load())
                        break;
                    //std::this_thread::sleep_for(std::chrono::milliseconds(1000*health_diagnostic_interval));
                    perform_diagnostic_check();
                }
            });
            // spawn the health monitoring thread...
        }
    }

    void daemon_orch_obj::kill_threads() {

        // kill the health monitoring thread first and wait for it to join...
        {
            std::lock_guard<std::mutex> health_thread_lock(health_thread_mutex);
            is_health_thread_running.store(false);
        }
        health_thread_condition_variable.notify_one();
        if (health_monitoring_thread.joinable()) {
            health_monitoring_thread.join();
        }
            
        std::vector<std::optional<input_socket::util::socket_tracer_health_snapshot>> socket_tracers;

        for (auto const& unix_socket : unix_listening_sockets) {
            unix_socket->close_socket();
            if (enable_end_of_test_diagnostics) {
                socket_tracers.push_back(unix_socket->get_socket_tracer_snapshot());
            }
            unix_socket->stop_thread();
        }

        for (auto const& ip_socket : web_listening_sockets) {
            ip_socket->close_socket();
            if (enable_end_of_test_diagnostics) {
                socket_tracers.push_back(ip_socket->get_socket_tracer_snapshot());
            }
            ip_socket->stop_thread();
        }

        // instead of having formatting in the input socket, do all of that formatting here?? Return the tracer structr at the end of the test (have a get tracer struct method)
        // this will enforce constant ordering of things...
        buffer_parser.stop_thread();

        if (enable_end_of_test_diagnostics) {
            std::string socket_eot_diagnostic_stream = input_socket::util::construct_eot_socket_diagnostic_msg(socket_tracers);
            while (!buffer_parser.queue_empty()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
            log_writer.enqueue_msg(std::move(socket_eot_diagnostic_stream));
        }

        log_writer.stop_thread();
    }

    void daemon_orch_obj::log_direct(std::string msg) { 
        if (buffer_parser.thread_active() && log_writer.thread_active()) {
            buffer_parser.enqueue_msg(msg); 
        }
    }

    void daemon_orch_obj::log_direct_bypass_parser(std::string msg) {
        if (log_writer.thread_active()) {
            log_writer.enqueue_msg(msg);
        }
    }

    void daemon_orch_obj::create_log_file(const std::string& log_file_path) {
        std::ofstream log_file(log_file_path, std::ios::out | std::ios::trunc);
        if (log_file.is_open()) {
            throw std::runtime_error("Log file already open elsewhere. Please try again.");
        }
    }

    void daemon_orch_obj::register_thread_diagnostic(const std::unique_ptr<input_socket::input_socket_obj>& socket) {
        delta_start.push_back(socket->get_socket_tracer_snapshot().value());
    }

    void daemon_orch_obj::perform_diagnostic_check() {
        std::chrono::steady_clock::time_point current_time = std::chrono::steady_clock::now();

        for (auto const& unix_socket : unix_listening_sockets) {
            delta_end.push_back(unix_socket->get_socket_tracer_snapshot().value());
        }

        for (auto const& ip_socket : web_listening_sockets) {
            delta_end.push_back(ip_socket->get_socket_tracer_snapshot().value());
        }

        log_writer.enqueue_msg("\nHEALTH DIAGNOSTIC CHECK\n");
    
        std::stringstream out_string;
        // starttime endtime .count
        double uptime = std::chrono::duration<double>(current_time - delta_end.at(0).start_time).count();
        out_string << "UPTIME: " << std::to_string(uptime) << "\n";
        out_string << "SOCKET/PORT: " << delta_end.at(0).socket_or_port << "\n";
        out_string << "TOTAL BYTES READ: " << delta_end.at(0).bytes_received << "\n";
        out_string << "TOTAL CONNECTIONS: " << delta_end.at(0).connections_received << "\n";
        out_string << "Δ BYTES: " << (delta_end.at(0).bytes_received - delta_start.at(0).bytes_received) << "\n";
        out_string << "Δ CONN: " << (delta_end.at(0).connections_received - delta_start.at(0).connections_received) << "\n";
        
        log_writer.enqueue_msg(out_string.str());

        delta_start = std::move(delta_end);

        
        // diff delta start and delta end 
        // do something with those diffs
        // set delta start to delta and and clear delta end
    }

    void daemon_orch_obj::wait_until_queues_empty() {
        buffer_parser.wait_until_queue_empty();
        log_writer.wait_until_queue_empty();
    }
    
}