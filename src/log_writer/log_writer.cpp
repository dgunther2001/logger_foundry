#include "log_writer.h"

namespace log_writer {


    void log_writer_obj::enqueue_msg(std::string msg) {
        {
            std::lock_guard<std::mutex> lock(thread_active_mutex);
            msgs_to_log.push(std::move(msg));
        }
        thread_active_condition_var.notify_one();
    }

    void log_writer_obj::run_thread() {
        while (is_thread_running || !msgs_to_log.empty()) {
            std::unique_lock<std::mutex> lock(thread_active_mutex);
            thread_active_condition_var.wait(lock, [this] { return !msgs_to_log.empty() || !is_thread_running; });

            while (!msgs_to_log.empty()) {
                auto current_message = msgs_to_log.front();
                msgs_to_log.pop();
                if (valid_log_stream) {
                    log_stream << current_message;
                } 
            }
        }
        //thread_active_condition_var.notify_all();
        log_stream.flush();
    }

    void log_writer_obj::init_thread() {
        is_thread_running = true;
        if (valid_log_stream) {
            log_stream.open(file_path);
        }
        writing_thread = std::thread(&log_writer_obj::run_thread, this);
    }

    void log_writer_obj::stop_thread() {
        {
            std::lock_guard<std::mutex> lock(thread_active_mutex);
            is_thread_running = false;
            thread_active_condition_var.notify_all();
        }

        if (writing_thread.joinable()) {
            writing_thread.join();
        }

        if (log_stream.is_open()) {
            log_stream.close();
        }
    }

    void log_writer_obj::wait_until_queue_empty() {
        std::unique_lock<std::mutex> lock(thread_active_mutex);
        thread_active_condition_var.wait(lock, [this]() { 
            return msgs_to_log.empty(); 
        });
    }
}