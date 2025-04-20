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

namespace input_socket {

    enum socket_type {
        UNIX = 1,
        WEB = 10,
    };

    class input_socket_obj {
    public:
        input_socket_obj(std::function<void(std::string)> enqueue_to_buffer_parser_callback, 
                         std::function<void(std::string)> log_self_callback, 
                         std::function<bool()> parser_thread_active_callback,
                         const std::string& host_path,
                         const std::string& socket_path,
                         uint16_t port,
                         uint16_t backlog,
                         socket_type sock_type
                        ) : 
                        enqueue_to_buffer_parser_callback(enqueue_to_buffer_parser_callback),
                        log_self_callback(log_self_callback),
                        parser_thread_active_callback(parser_thread_active_callback),
                        host_path(host_path),
                        socket_path(socket_path),
                        port(port),
                        backlog(backlog),
                        socket_type(sock_type)
                        {}
        void init_socket();
        void read_socket();
        void close_socket();

        void init_thread();
        void stop_thread();

        bool thread_active() { return is_thread_running; }

    private:
        void enqueue_buffer_parser(std::string msg);

        int sockfd = -1;
        std::string log_file_path;
        uint16_t backlog;
        socket_type socket_type;
        std::string socket_path;
        std::string host_path;
        uint16_t port;

        std::function<void(std::string)> enqueue_to_buffer_parser_callback;
        std::function<void(std::string)> log_self_callback;
        std::function<bool()> parser_thread_active_callback;


        void run_thread();

        std::condition_variable thread_active_condition_var;
        std::mutex thread_active_mutex;
        std::thread writing_thread;
        bool is_thread_running = false;
    };

    class input_socket_builder {
    public:
        input_socket_builder& set_log_path(std::string log_file_path);
        input_socket_builder& set_backlog(uint16_t num_connections);
        input_socket_builder& set_socket_type(socket_type sock_type);

        input_socket_builder& set_enqueue_buffer_parser_callback(std::function<void(std::string)> enqueue_to_buffer_parser_callback);
        input_socket_builder& set_direct_log_callback(std::function<void(std::string)> log_self_callback);
        input_socket_builder& set_parser_active_callback(std::function<bool()> parser_thread_active_callback);

        input_socket_builder& set_socket_path(std::string socket_path);

        input_socket_builder& set_host_path(std::string host_path);
        input_socket_builder& set_port(uint16_t port);

        std::unique_ptr<input_socket_obj> build();

    private:
        std::string log_file_path = "";
        uint16_t backlog = 5;
        socket_type sock_type;
        std::string socket_path = "";
        std::string host_path = "";
        uint16_t port = 65535;

        std::function<void(std::string)> enqueue_to_buffer_parser_callback;
        std::function<void(std::string)> log_self_callback;
        std::function<bool()> parser_thread_active_callback;

    };
}