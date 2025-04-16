#include "logger_foundry/logger_foundry.h"
#include "daemon_orchestrator.h"

namespace logger_foundry {
    logger_daemon::logger_daemon(const char* log_file_path, const char* socket_path, parser_strategy parsing_strategy) :
                  daemon_orchestrator(std::make_unique<daemon_orchestrator::daemon_orch_obj>(log_file_path, socket_path, parsing_strategy))
                  { daemon_orchestrator->start_threads(); }

    logger_daemon::~logger_daemon() { daemon_orchestrator->kill_threads(); }
}


