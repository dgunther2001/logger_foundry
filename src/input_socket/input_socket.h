#pragma once

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string>
#include <thread>
#include <iostream>
#include <functional>
#include <condition_variable>

namespace input_socket {

    class input_socket_obj {
    public:
        input_socket_obj(std::function<void(std::string)> enqueue_to_buffer_parser_callback, 
                         std::function<void(std::string)> log_self_callback, 
                         std::function<bool()> parser_thread_active_callback,
                         const std::string& socket_path,
                         uint32_t backlog
                        ) : 
                        enqueue_to_buffer_parser_callback(enqueue_to_buffer_parser_callback),
                        log_self_callback(log_self_callback),
                        parser_thread_active_callback(parser_thread_active_callback),
                        SOCKET_PATH(socket_path)
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
        std::string SOCKET_PATH;
        uint32_t backlog = 20;

        std::function<void(std::string)> enqueue_to_buffer_parser_callback;
        std::function<void(std::string)> log_self_callback;
        std::function<bool()> parser_thread_active_callback;


        void run_thread();

        std::condition_variable thread_active_condition_var;
        std::mutex thread_active_mutex;
        std::thread writing_thread;
        bool is_thread_running = false;
    };
}