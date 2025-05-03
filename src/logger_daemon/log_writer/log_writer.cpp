#include "log_writer.h"

namespace log_writer {


    void log_writer_obj::enqueue_msg(std::string msg) {
        if (msgs_to_log.size() >= QUEUE_MAX_SIZE) {
            dropped_messages++;
            return;
        }

        {
            std::lock_guard<std::mutex> lock(thread_active_mutex);
            msgs_to_log.push(std::move(msg));
        }
        thread_active_condition_var.notify_one();
    }

    void log_writer_obj::run_thread() {
        auto start = std::chrono::steady_clock::now();

        std::vector<std::string> write_batch;
        write_batch.reserve(BATCH_SIZE_MAX);

        while (is_thread_running || !msgs_to_log.empty()) {
            {
                std::unique_lock<std::mutex> lock(thread_active_mutex);
                thread_active_condition_var.wait_for(lock, FLUSH_INTERVAL, [this] { return !msgs_to_log.empty() || !is_thread_running; });

                write_batch.clear();
                while ((!msgs_to_log.empty()) && (write_batch.size() < BATCH_SIZE_MAX)) {
                    write_batch.push_back(msgs_to_log.front());
                    msgs_to_log.pop();
                    /*
                    auto current_message = msgs_to_log.front();
                    msgs_to_log.pop();
                    if (valid_log_stream) {
                        log_stream << current_message;
                    } 
                    */
                }
            }

            if (!write_batch.empty() && valid_log_stream) {
                for (const auto& message : write_batch) {
                    log_stream << message;
                }
                //auto now = std::chrono::steady_clock::now();
                //auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
                //std::cerr << "[flush at " << ms << "ms] batch size = " << write_batch.size() << "\n";
                log_stream.flush();
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