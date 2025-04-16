#pragma once

#include <functional>
#include <optional>
#include <memory>

namespace daemon_orchestrator {
    class daemon_orch_obj;
}

using parser_strategy = std::function<std::optional<std::string>(const std::string&)>;

namespace logger_foundry {
    class logger_daemon {
    public:
        logger_daemon(const char* log_file_path, const char* socket_path, parser_strategy parsing_strategy);
        ~logger_daemon();

        std::unique_ptr<daemon_orchestrator::daemon_orch_obj> daemon_orchestrator;

        void start();
        void stop();

    };
}