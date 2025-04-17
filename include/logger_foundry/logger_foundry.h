#pragma once

#include <functional>
#include <optional>
#include <memory>
#include <thread>

namespace daemon_orchestrator {
    class daemon_orch_obj;
}

using parser_strategy = std::function<std::optional<std::string>(const std::string&)>;

namespace logger_foundry {
    using kill_logger_strategy = std::function<void()>;

    class logger_daemon {
    public:
        logger_daemon(const char* log_file_path, const char* socket_path, parser_strategy parsing_strategy, kill_logger_strategy kill_strategy=nullptr);
        logger_daemon(const char* log_file_path, const char* socket_path, kill_logger_strategy kill_strategy);
        
        ~logger_daemon();

        void log_orchestrator_info(std::string msg);
    
    private:

        std::unique_ptr<daemon_orchestrator::daemon_orch_obj> daemon_orchestrator_obj;
        kill_logger_strategy kill_strategy;
        std::thread kill_strategy_monitor;

    };
}