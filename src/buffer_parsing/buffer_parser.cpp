#include "buffer_parser.h"

namespace buffer_parser { 
    void buffer_parser_obj::enqueue_msg(std::string msg) {
        {
            std::lock_guard<std::mutex> lock(thread_active_mutex);
            msgs_to_parse.push(msg);
        }
        thread_active_condition_var.notify_one();
    }

    void buffer_parser_obj::enqueue_log_writer(std::optional<std::string> msg) {
        if (msg) {
            enqueue_to_log_writer_callback(*msg);
        }
    }

    void buffer_parser_obj::run_thread() {
        while (is_thread_running || !msgs_to_parse.empty()) {
            std::unique_lock<std::mutex> lock(thread_active_mutex);
            thread_active_condition_var.wait(lock, [this] { return !msgs_to_parse.empty() || !is_thread_running; });

            while (!msgs_to_parse.empty()) {
                auto current_message = msgs_to_parse.front(); 
                msgs_to_parse.pop();
                enqueue_log_writer(parsing_strategy(current_message));
            }
        }
        //thread_active_condition_var.notify_all();
    }

    void buffer_parser_obj::init_thread() {
        if (!log_writer_thread_active_callback()) {
            // ADD ERROR HANDLING
        }
        is_thread_running = true;
        writing_thread = std::thread(&buffer_parser_obj::run_thread, this);
    }

    void buffer_parser_obj::stop_thread() {
        {
            std::lock_guard<std::mutex> lock(thread_active_mutex);
            is_thread_running = false;
            thread_active_condition_var.notify_all();
        }

        if (writing_thread.joinable()) {
            writing_thread.join();
        }
    }

    void buffer_parser_obj::wait_until_queue_empty() {
        std::unique_lock<std::mutex> lock(thread_active_mutex);
        thread_active_condition_var.wait(lock, [this]() { 
            return msgs_to_parse.empty(); 
        });
    }
    
}