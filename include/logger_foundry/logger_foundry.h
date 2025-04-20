#pragma once

#include <functional>
#include <optional>
#include <memory>
#include <thread>
#include <vector>

#include "socket_config_structs.h"
//#include <config_headers/socket_config_structs.h>

// forward declarations
namespace daemon_orchestrator {
    class daemon_orch_obj;
}
using parser_strategy = std::function<std::optional<std::string>(const std::string&)>;
class logger_daemon_builder;

namespace logger_foundry {
    using kill_logger_strategy = std::function<void()>;

    class logger_daemon {
        friend class logger_daemon_builder;
    public:
        
        ~logger_daemon();

        void log_direct(std::string msg);
    
    private:
        logger_daemon(const std::string& log_file_path, std::vector<socket_config::unix_socket_config> unix_socket_config, parser_strategy parsing_strategy, kill_logger_strategy kill_strategy);
        std::unique_ptr<daemon_orchestrator::daemon_orch_obj> daemon_orchestrator_obj;
        kill_logger_strategy kill_strategy;
        std::thread kill_strategy_monitor;

    };

    class logger_daemon_builder {
    public:
        logger_daemon_builder& set_log_path(std::string log_file_path);
        logger_daemon_builder& add_unix_socket(std::string socket_path, uint32_t backlog);
        logger_daemon_builder& set_parser_strategy(parser_strategy parser_strategy_inst);
        logger_daemon_builder& set_kill_strategy(kill_logger_strategy kill_logger_strategy_inst);

        logger_daemon build();
    private:
        std::string log_file_path="";
        std::vector<socket_config::unix_socket_config> unix_socket_configs;
        parser_strategy parser_strategy_inst = nullptr;
        kill_logger_strategy kill_logger_strategy_inst = nullptr;
    };
}