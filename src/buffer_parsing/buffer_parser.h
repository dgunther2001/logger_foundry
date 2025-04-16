#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include <thread>

using parser_strategy = std::function<std::optional<std::string>(const std::string&)>;


namespace buffer_parser {


    // what does this need to do?
    // have a reference to a log writer obj passed via the constructor from the orchestrator
    // have a pass parsed msg to log_writer function which takes a string (checks optional and passes if not std::nullopt)
    class buffer_parser_obj {
    public:
        buffer_parser_obj(std::function<void(std::string)> enqueue_to_log_writer_callback, 
                          std::function<void(std::string)> log_self_callback, 
                          std::function<bool()> log_writer_thread_active_callback,
                          parser_strategy parsing_strategy
                         ) : 
                         enqueue_to_log_writer_callback(enqueue_to_log_writer_callback),
                         log_self_callback(log_self_callback),
                         log_writer_thread_active_callback(log_writer_thread_active_callback),
                         parsing_strategy(parsing_strategy)
                         {}

        void enqueue_msg(std::string msg);
        void init_thread();
        void stop_thread();

        void wait_until_queue_empty();

        bool thread_active() { return is_thread_running; }

    private:
        std::function<void(std::string)> enqueue_to_log_writer_callback;
        std::function<void(std::string)> log_self_callback;
        std::function<bool()> log_writer_thread_active_callback;

        parser_strategy parsing_strategy;
        void enqueue_log_writer(std::optional<std::string> msg);

        std::queue<std::string> msgs_to_parse;
        
        
        void run_thread();

        std::condition_variable thread_active_condition_var;
        std::mutex thread_active_mutex;
        std::thread writing_thread;
        bool is_thread_running = false;
    };

}

