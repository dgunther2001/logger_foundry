#pragma once

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string>
#include <thread>
#include <iostream>
#include <functional>
#include <condition_variable>
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <chrono>
#include <optional>
#include <sstream>
#include <iomanip>
#include <format>

namespace input_socket::util {

    enum socket_type {
        UNIX = 1,
        WEB = 10,
    };

    struct socket_tracer_delta {
        std::string socket_or_port;
        uint32_t connections_received;
        uint32_t connections_delta;
        uint32_t bytes_received;
        uint32_t bytes_delta;
        double uptime;
        socket_type socket_type;
    };

    struct socket_tracer_health_snapshot {
        std::string socket_or_port; // socket path or port
        uint32_t connections_received = 0;
        uint32_t bytes_received = 0;
        std::chrono::steady_clock::time_point start_time;
        std::chrono::steady_clock::time_point end_time;
        socket_type socket_type;

        std::chrono::duration<double> socket_uptime() const {
            return end_time - start_time;
        }
    };

    struct socket_tracer {
        std::string socket_or_port; // socket path or port
        std::atomic<uint32_t> connections_received = 0;
        std::atomic<uint32_t> bytes_received = 0;
        std::chrono::steady_clock::time_point start_time;
        std::chrono::steady_clock::time_point end_time;
        socket_type socket_type;

        std::chrono::duration<double> socket_uptime() const {
            return end_time - start_time;
        }

        socket_tracer(const std::string socket_path) : socket_or_port(socket_path), socket_type(UNIX) {}
        socket_tracer(uint16_t port) : socket_or_port(std::to_string(port)), socket_type(socket_type::WEB) {}
    };


    // END OF TEST
    std::string construct_eot_socket_diagnostic_msg(std::vector<std::optional<input_socket::util::socket_tracer_health_snapshot>>& socket_tracers);
    std::string format_diagnostics(const std::string& socket_or_port, uint32_t connections_received, uint32_t bytes_received, double uptime, socket_type sock_type);


    // HEALTH MONITORING
    std::string construct_health_monitor_diagnostic(std::vector<input_socket::util::socket_tracer_delta>& snapshot_deltas, uint64_t health_interval);
    std::string format_health_monitor_diagnostics(const input_socket::util::socket_tracer_delta& snapshot_delta);

    // FORMATTING UTILITY
    std::string format_socket_path(const std::string& socket_path);
    std::string center_string(const std::string& string, uint8_t width, char filler);
}


namespace input_socket {

    class input_socket_obj {
    public:
        input_socket_obj(std::function<void(std::string)> enqueue_to_consumer_callback, 
                         std::function<void(std::string)> log_diagnostic_callback, 
                         std::function<bool()> parser_thread_active_callback,
                         const std::string& host_path,
                         const std::string& socket_path,
                         uint16_t port,
                         uint16_t backlog,
                         util::socket_type sock_type
                        ) : 
                        enqueue_to_consumer_callback(enqueue_to_consumer_callback),
                        log_diagnostic_callback(log_diagnostic_callback),
                        parser_thread_active_callback(parser_thread_active_callback),
                        host_path(host_path),
                        socket_path(socket_path),
                        port(port),
                        backlog(backlog),
                        socket_type(sock_type)
                        {
                            if (sock_type == util::socket_type::UNIX) {
                                socket_tracer.emplace(socket_path);
                            } else if (sock_type == util::socket_type::WEB) {
                                socket_tracer.emplace(port);
                            }
                        }
        void init_socket();
        void read_socket();
        void close_socket();

        void init_thread();
        void stop_thread();

        bool thread_active() { return is_thread_running; }

        std::optional<input_socket::util::socket_tracer_health_snapshot> get_socket_tracer_snapshot() { return input_socket::util::socket_tracer_health_snapshot{
            socket_tracer->socket_or_port, socket_tracer->connections_received.load(), socket_tracer->bytes_received.load(), socket_tracer->start_time, socket_tracer->end_time, socket_tracer->socket_type
        }; }

        input_socket::util::socket_type get_socket_type() { return socket_type; }


    private:
        void enqueue_to_consumer(std::string msg);

        int sockfd = -1;
        std::string log_file_path;
        uint16_t backlog;
        util::socket_type socket_type;
        std::string socket_path;
        std::string host_path;
        uint16_t port;

        std::function<void(std::string)> enqueue_to_consumer_callback;
        std::function<void(std::string)> log_diagnostic_callback;
        std::function<bool()> parser_thread_active_callback;


        void run_thread();

        std::condition_variable thread_active_condition_var;
        std::mutex thread_active_mutex;
        std::thread writing_thread;
        bool is_thread_running = false;

        std::optional<input_socket::util::socket_tracer> socket_tracer;

    };

    class input_socket_builder {
    public:
        input_socket_builder& set_log_path(std::string log_file_path);
        input_socket_builder& set_backlog(uint16_t num_connections);
        input_socket_builder& set_socket_type(util::socket_type sock_type);

        input_socket_builder& set_enqueue_to_consumer_callback(std::function<void(std::string)> enqueue_to_consumer_callback);
        input_socket_builder& set_log_diagnostic_callback(std::function<void(std::string)> log_diagnostic_callback);
        input_socket_builder& set_parser_active_callback(std::function<bool()> parser_thread_active_callback);

        input_socket_builder& set_socket_path(std::string socket_path);

        input_socket_builder& set_host_path(std::string host_path);
        input_socket_builder& set_port(uint16_t port);

        std::unique_ptr<input_socket_obj> build();

    private:
        std::string log_file_path = "";
        uint16_t backlog = 5;
        util::socket_type sock_type;
        std::string socket_path = "";
        std::string host_path = "";
        uint16_t port = 65535;

        std::function<void(std::string)> enqueue_to_consumer_callback;
        std::function<void(std::string)> log_diagnostic_callback;
        std::function<bool()> parser_thread_active_callback;

    };
}