#pragma once

#include <iostream>
#include <string>
#include <optional>
#include <fstream>
#include <queue>
#include <functional>
#include <condition_variable>
#include <thread>
#include <mutex>

namespace log_writer {
    class log_writer_obj {
    public:
        log_writer_obj(const std::string& file_path,
                       std::function<void(std::string)> log_self_callback
                      ) : 
                      file_path(file_path),
                      log_self_callback(log_self_callback)
                      {
                        if (!file_path.empty()) valid_log_stream = true;
                      }
        bool log_to_file(std::string file_path, std::string msg);
        void enqueue_msg(std::string msg);
        void init_thread();
        void stop_thread();

        void wait_until_queue_empty();

        bool thread_active() { return is_thread_running; }

    private:
        std::function<void(std::string)> log_self_callback;

        void run_thread();

        std::queue<std::string> msgs_to_log;
        std::ofstream log_stream;
        bool valid_log_stream = false;

        std::condition_variable thread_active_condition_var;
        std::mutex thread_active_mutex;
        std::thread writing_thread;
        bool is_thread_running = false;
        std::string file_path;


    };
}